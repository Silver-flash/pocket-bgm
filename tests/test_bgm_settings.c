#include "bgm_settings.h"
#include "bgm_model.h"
#include "nvs.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned disk=30, pending, commits, closes;
static int exists, init_error, open_error, read_error, write_error, commit_error;
esp_err_t nvs_flash_init(void) { return init_error; }
esp_err_t nvs_open(const char *name, int mode, nvs_handle_t *handle) {
    assert(!strcmp(name,"pocket_bgm")); *handle=1;
    if(open_error) return open_error;
    return !exists && mode==NVS_READONLY ? ESP_ERR_NVS_NOT_FOUND : ESP_OK;
}
esp_err_t nvs_get_u16(nvs_handle_t h,const char *key,uint16_t *out) {
    assert(h==1 && !strcmp(key,"volume")); *out=disk; return read_error;
}
esp_err_t nvs_set_u16(nvs_handle_t h,const char *key,uint16_t value) {
    assert(h==1 && !strcmp(key,"volume")); pending=value; return write_error;
}
esp_err_t nvs_commit(nvs_handle_t h) {
    assert(h==1); ++commits;
    if(commit_error) return commit_error;
    disk=pending; exists=1; return ESP_OK;
}
void nvs_close(nvs_handle_t h) { assert(h==1); ++closes; }
int main(void) {
    unsigned volume=999;
    assert(bgm_settings_load(&volume)==ESP_OK && volume==30 && commits==0);
    assert(bgm_settings_load(NULL)==ESP_ERR_INVALID_ARG);
    for(unsigned v=0;v<=500;v++) {
        int valid=v<=100 ? v%10==0 : v%100==0;
        unsigned before=commits;
        assert((bgm_settings_save(v)==ESP_OK)==valid);
        if(!valid) {assert(commits==before);continue;}
        bgm_model_t reboot; bgm_model_init(&reboot,11);
        assert(bgm_settings_load(&reboot.volume)==ESP_OK && reboot.volume==v);
        assert(!reboot.playing && reboot.active==-1);
    }
    assert(bgm_settings_save(501)==ESP_ERR_INVALID_ARG);
    unsigned old=disk; commit_error=ESP_FAIL;
    assert(bgm_settings_save(20)==ESP_FAIL && disk==old);commit_error=0;
    unsigned before=commits, closed=closes;write_error=ESP_FAIL;
    assert(bgm_settings_save(20)==ESP_FAIL && commits==before && closes==closed+1);write_error=0;
    disk=65535;assert(bgm_settings_load(&volume)==ESP_ERR_INVALID_ARG && volume==30);
    read_error=ESP_ERR_NVS_NOT_FOUND;assert(bgm_settings_load(&volume)==ESP_OK && volume==30);
    read_error=ESP_FAIL;assert(bgm_settings_load(&volume)==ESP_FAIL && volume==30);read_error=0;
    open_error=ESP_FAIL;assert(bgm_settings_load(&volume)==ESP_FAIL && volume==30);
    assert(bgm_settings_save(20)==ESP_FAIL);open_error=0;
    init_error=ESP_FAIL;assert(bgm_settings_load(&volume)==ESP_FAIL && volume==30);
    puts("BGM settings: PASS (all volume levels, silent reboot, invalid data and storage errors)");
}
