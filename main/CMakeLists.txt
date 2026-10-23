include("${CMAKE_CURRENT_LIST_DIR}/bgm_assets.cmake")
idf_component_register(
    SRCS "main.c" "bgm_model.c" "bgm_gain.c" "bgm_decoder.c" "bgm_tracks.c" "../assets/fonts/bgm/bgm_font.c"
    INCLUDE_DIRS "."
    REQUIRES bsp esp_timer
    EMBED_FILES ${BGM_ASSETS}
)
