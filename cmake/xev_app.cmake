# xev_add_app(NAME)
#
# Sets up per-app build layout:
#   build/<name>/bin/      - executable
#   build/<name>/shaders/  - compiled engine shaders
#   build/<name>/assets/   - symlink to app/<name>/assets (if it exists)
#
function(xev_add_app NAME)
    # Per-app output directories
    set(APP_BIN_DIR "${CMAKE_BINARY_DIR}/${NAME}/bin")
    set(APP_SHADER_DIR "${CMAKE_BINARY_DIR}/${NAME}/shaders")

    set_target_properties(${NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${APP_BIN_DIR}"
    )

    # Compile engine shaders into app's shader dir
    target_compile_slang_shaders(${NAME} "${APP_SHADER_DIR}"
        "${CMAKE_SOURCE_DIR}/xev/src/shaders/mesh.slang"
        "${CMAKE_SOURCE_DIR}/xev/src/shaders/triangle.slang"
        "${CMAKE_SOURCE_DIR}/xev/src/shaders/raster.slang"
    )

    # Symlink assets if the app has an assets folder
    set(APP_ASSETS_SRC "${CMAKE_SOURCE_DIR}/app/${NAME}/assets")
    set(APP_ASSETS_DST "${CMAKE_BINARY_DIR}/${NAME}/assets")
    if(EXISTS "${APP_ASSETS_SRC}" AND NOT EXISTS "${APP_ASSETS_DST}")
        file(CREATE_LINK "${APP_ASSETS_SRC}" "${APP_ASSETS_DST}" SYMBOLIC)
    endif()
endfunction()
