// Pocket BGM: the player task exclusively owns state and all codec operations.
#include "bsp_display.h"
#include "bsp_audio.h"
#include "bsp_button.h"
#include "bsp_battery.h"
#include "bsp_pins.h"
#include "bgm_model.h"
#include "bgm_tracks.h"
#include "bgm_decoder.h"
#include "bgm_gain.h"
#include "bgm_settings.h"
#include "bgm_input.h"
#include "key_guard.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "lvgl.h"
#include <string.h>
#include <stdatomic.h>

LV_FONT_DECLARE(bgm_font);

#define RATE 16000u
#define CHUNK 320u
static const char *TAG = "pocket_bgm";
static QueueHandle_t s_events;
static atomic_bool s_input_ready;
static key_guard_t s_key_guard;
static bgm_model_t s_state;
static lv_obj_t *s_title, *s_status, *s_now, *s_meta, *s_hint, *s_battery, *s_index, *s_volume;
static lv_obj_t *s_progress, *s_bars[9];
static bool s_audio_ready, s_codec_awake, s_settings_failed;
static int16_t s_pcm[CHUNK];

typedef struct { bsp_btn_t key; bsp_btn_ev_t event; } input_t;
static void on_key(bsp_btn_t key, bsp_btn_ev_t event, void *user) {
    (void)user;
    if (key_guard_consume(&s_key_guard, event == BSP_BTN_RELEASE, esp_timer_get_time())) return;
    if (!s_input_ready) return;
    if (event != BSP_BTN_CLICK && event != BSP_BTN_LONG && event != BSP_BTN_DOUBLE) return;
    input_t input = {bgm_logical_key(key), event};
    (void)xQueueSend(s_events, &input, 0);
}
static lv_obj_t *label(lv_obj_t *parent, int x, int y, int w, const char *text, uint32_t color) {
    lv_obj_t *obj = lv_label_create(parent);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, 24);
    lv_obj_set_style_text_font(obj, &bgm_font, 0);
    lv_obj_set_style_text_color(obj, lv_color_hex(color), 0);
    lv_label_set_long_mode(obj, LV_LABEL_LONG_DOT);
    lv_label_set_text(obj, text);
    return obj;
}
static void build_ui(void) {
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x080D18), 0);
    lv_obj_set_style_pad_all(screen, 0, 0);
    label(screen, 20, 18, 145, "随身 BGM", 0x42F5F2);
    s_battery = label(screen, 175, 18, 48, "--%", 0x9CAFCB);
    lv_obj_t *card = lv_obj_create(screen);
    lv_obj_set_pos(card, 14, 51);
    lv_obj_set_size(card, BSP_LCD_W - 28, 151);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x121C30), 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_border_color(card, lv_color_hex(0x344B70), 0);
    lv_obj_set_style_radius(card, 4, 0);
    lv_obj_set_style_pad_all(card, 0, 0);
    lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    s_index = label(card, 12, 8, 188, "", 0xFF5AD6);
    for (unsigned i=0;i<9;i++) {
        s_bars[i]=lv_obj_create(card);
        lv_obj_remove_style_all(s_bars[i]);
        lv_obj_set_pos(s_bars[i], 43+i*14, 57-(i%3)*6);
        lv_obj_set_size(s_bars[i], 7, 7+(i%3)*6);
        lv_obj_set_style_bg_color(s_bars[i],lv_color_hex(i%2 ? 0xFF5AD6 : 0x42F5F2),0);
        lv_obj_set_style_bg_opa(s_bars[i],LV_OPA_COVER,0);
    }
    s_title = label(card, 12, 73, 188, "", 0xFFFFFF);
    lv_obj_set_style_text_align(s_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(s_title, LV_LABEL_LONG_SCROLL_CIRCULAR);
    s_status = label(card, 12, 103, 188, "", 0x42F5F2);
    lv_obj_set_style_text_align(s_status, LV_TEXT_ALIGN_CENTER, 0);
    s_progress = lv_bar_create(card);
    lv_obj_set_pos(s_progress, 16, 132);
    lv_obj_set_size(s_progress, 180, 4);
    lv_obj_set_style_bg_color(s_progress, lv_color_hex(0x42F5F2), LV_PART_INDICATOR);
    s_now = label(screen, 24, 240, 192, "", 0xEEF2F6);
    lv_label_set_long_mode(s_now, LV_LABEL_LONG_SCROLL_CIRCULAR);
    s_meta = label(screen, 24, 213, 108, "", 0x9CAFCB);
    s_volume = label(screen, 140, 213, 76, "", 0x42F5F2);
    lv_obj_set_style_text_align(s_volume, LV_TEXT_ALIGN_RIGHT, 0);
    s_hint = label(screen, 30, 274, 180, "", 0x9CAFCB);
    lv_obj_set_style_text_align(s_hint, LV_TEXT_ALIGN_CENTER, 0);

    lv_screen_load(screen);
}
static void render(int battery, const char *error) {
    if (!bsp_lvgl_lock(30)) return;
    if (battery < 0) lv_label_set_text(s_battery, "--%");
    else lv_label_set_text_fmt(s_battery, "%d%%", battery);
    if (!s_state.count) {
        lv_label_set_text(s_title, "曲库为空");
        lv_label_set_text(s_status, "请在后台添加音频");
        lv_label_set_text(s_index, "00 / 00");
        lv_label_set_text(s_hint, "");
        lv_label_set_text(s_now, "");
        lv_label_set_text(s_meta, "");
        lv_label_set_text(s_volume, "");
        bsp_lvgl_unlock(); return;
    }
    const bgm_track_t *selected = &bgm_tracks[s_state.selected];
    const bgm_track_t *active = s_state.active >= 0 ? &bgm_tracks[s_state.active] : selected;
    lv_label_set_text_fmt(s_index, "%02u / %02u", s_state.selected+1, s_state.count);
    lv_label_set_text(s_meta, s_state.loop ? "单曲循环" : "播放一次");
    lv_label_set_text(s_now, "");
    if (s_state.settings) {
        lv_label_set_text(s_title, "播放设置");
        if (s_settings_failed) lv_label_set_text(s_status, "设置失败 请重试");
        else lv_label_set_text_fmt(s_status, "音量 %u%%", s_state.volume);
        lv_label_set_text(s_volume, "");
        lv_label_set_text(s_now, "上下调音量 中键切循环");
        lv_label_set_text(s_hint, "长按中键返回");
    } else {
        lv_label_set_text(s_title, selected->title);
        if (s_settings_failed) lv_label_set_text(s_status, "设置失败 请重试");
        else if (error) lv_label_set_text(s_status, error);
        else {
            unsigned elapsed = s_state.playing ? s_state.cursor / RATE : 0;
            unsigned duration = (s_state.playing ? active : selected)->samples / RATE;
            lv_label_set_text_fmt(s_status, "%02u:%02u / %02u:%02u",
                                 elapsed/60, elapsed%60, duration/60, duration%60);
        }
        lv_label_set_text_fmt(s_volume, "%u%%", s_state.volume);
        lv_label_set_text(s_hint, "长按中键设置");
        // Show the active title only when browsing a different selection.
        if (s_state.playing && s_state.active != (int)s_state.selected)
            lv_label_set_text_fmt(s_now, "播放: %s", active->title);
    }
    lv_bar_set_value(s_progress, s_state.playing ? (int)(100ull*s_state.cursor/active->samples) : 0, LV_ANIM_OFF);
    for (unsigned i=0;i<9;i++) {
        int height=s_state.playing ? 6+(int)((s_state.cursor/1600+i*7)%24) : 6;
        lv_obj_set_height(s_bars[i],height);
        lv_obj_set_y(s_bars[i],64-height);
        lv_obj_set_style_bg_opa(s_bars[i],s_state.playing?LV_OPA_COVER:LV_OPA_40,0);
    }
    bsp_lvgl_unlock();
}
static void audio_failure(const char **error, esp_err_t result) {
    ESP_LOGE(TAG, "Audio operation failed: %s", esp_err_to_name(result));
    s_state.playing = false;
    *error = "音频失败 请重试";
    (void)bsp_audio_sleep();
    s_codec_awake = false;
}
static void player_task(void *arg) {
    (void)arg;
    int battery = -1;
    int64_t last_battery = -30000000, last_ui = 0, last_input = esp_timer_get_time();
    const char *error = s_audio_ready ? NULL : "音频失败 请重试";
    bool dirty = true, dimmed = false;
    unsigned applied_volume = 501;
    bgm_gain_t gain = { .percent = 100 };
    for (;;) {
        input_t in;
        if (xQueueReceive(s_events, &in, s_state.playing ? 0 : pdMS_TO_TICKS(50)) == pdTRUE) {
            last_input = esp_timer_get_time();
            if (dimmed) { bsp_display_backlight(70); dimmed = false; }
            unsigned previous_volume = s_state.volume;
            if (in.event == BSP_BTN_LONG) {
                if (in.key == BSP_BTN_OK) bgm_model_action(&s_state, BGM_SETTINGS);
            } else {
                bgm_action_t action = in.key == BSP_BTN_UP ? BGM_PREV : in.key == BSP_BTN_DOWN ? BGM_NEXT : BGM_TOGGLE;
                bgm_model_action(&s_state, action);
                // The driver emits DOUBLE instead of two CLICKs. Treat OK as one intent.
                if (in.event == BSP_BTN_DOUBLE && in.key != BSP_BTN_OK) bgm_model_action(&s_state, action);
            }
            // Commit changed volume before applying it, so a completed adjustment
            // survives power-off. No writes for navigation or limit presses.
            if (s_state.volume != previous_volume) {
                esp_err_t saved = bgm_settings_save(s_state.volume);
                s_settings_failed = saved != ESP_OK;
                if (s_settings_failed) ESP_LOGE(TAG, "Volume save failed: %s", esp_err_to_name(saved));
            }
            dirty = true;
        }
        if (s_state.playing) {
            esp_err_t result = ESP_OK;
            if (!s_audio_ready) {
                result = bsp_audio_init();
                s_audio_ready = result == ESP_OK;
            }
            if (result == ESP_OK && !s_codec_awake) {
                result = bsp_audio_wake();
                if (result == ESP_OK) result = bsp_audio_set_format(RATE, 16, 1);
                applied_volume = 501;
                gain.percent = 100;
                s_codec_awake = result == ESP_OK;
            }
            if (result != ESP_OK) { audio_failure(&error, result); dirty = true; }
            else {
                error = NULL;
                if (applied_volume != s_state.volume) {
                    bsp_audio_set_volume(bgm_codec_volume(s_state.volume));
                    applied_volume = s_state.volume;
                }
                const bgm_track_t *track = &bgm_tracks[s_state.active];
                uint32_t count = track->samples - s_state.cursor;
                if (count > CHUNK) count = CHUNK;
                if (!bgm_decode(track->pcm, track->bytes, track->samples, s_state.cursor, s_pcm, count)) {
                    audio_failure(&error, ESP_ERR_INVALID_SIZE); dirty = true; continue;
                }
                bgm_gain_process(&gain, s_pcm, count, s_state.volume);
                // Fade track boundaries to avoid discontinuities in imported clips.
                for (uint32_t i=0; i<count; i++) {
                    uint32_t pos = s_state.cursor+i, fade = 240;
                    if (pos < fade) s_pcm[i] = (int32_t)s_pcm[i]*pos/fade;
                    if (track->samples-pos < fade) s_pcm[i] = (int32_t)s_pcm[i]*(track->samples-pos)/fade;
                }
                result = bsp_audio_write(s_pcm, count*2);
                if (result != ESP_OK) { audio_failure(&error, result); dirty = true; }
                else {
                    s_state.cursor += count;
                    if (s_state.cursor == track->samples) { bgm_model_end(&s_state); dirty = true; }
                }
            }
        }
        if (!s_state.playing && s_codec_awake) {
            esp_err_t result = bsp_audio_sleep();
            s_codec_awake = false;
            if (result != ESP_OK) error = "音频失败 请重试";
        }
        int64_t now = esp_timer_get_time();
        if (now-last_battery >= 30000000) { battery = bsp_battery_soc(); last_battery=now; dirty=true; }
        if (!dimmed && now-last_input > 30000000) { bsp_display_backlight(8); dimmed=true; }
        if (dirty || (s_state.playing && now-last_ui >= 250000)) {
            render(battery, error); last_ui=now; dirty=false;
        }
        // Always allow idle/watchdog work even if a codec backend returns immediately.
        vTaskDelay(1);
    }
}
void app_main(void) {
    ESP_LOGI(TAG, "Pocket BGM 1.4.0: offline manual playback");
    if (bsp_display_init() != ESP_OK || !bsp_lvgl_init()) return;
    bsp_display_backlight(70);
    bgm_model_init(&s_state, bgm_track_count);
    esp_err_t settings_result = bgm_settings_load(&s_state.volume);
    s_settings_failed = settings_result != ESP_OK;
    if (s_settings_failed) ESP_LOGE(TAG, "Volume restore failed: %s", esp_err_to_name(settings_result));
    s_events = xQueueCreate(16, sizeof(input_t));
    if (!s_events) { ESP_LOGE(TAG, "Input queue allocation failed"); return; }
    if (!bsp_lvgl_lock(1000)) return;
    build_ui();
    bsp_lvgl_unlock();
    s_audio_ready = bsp_audio_init() == ESP_OK;
    if (s_audio_ready) (void)bsp_audio_sleep();
    (void)bsp_battery_init();
    key_guard_arm(&s_key_guard, esp_timer_get_time(), 10000000);
    if (bsp_button_init(on_key, NULL) != ESP_OK) { render(-1, "按键失败 请重启"); return; }
    key_guard_sync_released(&s_key_guard, bsp_button_read_mv() >= BSP_BTN_MV_RELEASED_MIN);
    if (xTaskCreate(player_task, "bgm_player", 5120, NULL, 5, NULL) != pdPASS) {
        render(-1, "启动失败 请重启"); return;
    }
    s_input_ready = true;
    ESP_LOGI(TAG, "Ready; free heap=%u largest=%u", (unsigned)esp_get_free_heap_size(),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
}
