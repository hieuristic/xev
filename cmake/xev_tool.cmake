# xev_add_tool(NAME)
#
# Sets up per-tool build layout:
#   build/<name>/bin/      - executable
#   build/<name>/assets/   - symlink to tool/<name>/assets (if it exists)
#
function(xev_add_tool NAME)
    # Per-tool output directories
    set(TOOL_BIN_DIR "${CMAKE_BINARY_DIR}/${NAME}/bin")

    set_target_properties(${NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${TOOL_BIN_DIR}"
    )

    # Symlink assets if the app has an assets folder
    set(TOOL_ASSETS_SRC "${CMAKE_SOURCE_DIR}/tool/${NAME}/assets")
    set(TOOL_ASSETS_DST "${CMAKE_BINARY_DIR}/${NAME}/assets")
    if(EXISTS "${TOOL_ASSETS_SRC}" AND NOT EXISTS "${TOOL_ASSETS_DST}")
        file(CREATE_LINK "${TOOL_ASSETS_SRC}" "${TOOL_ASSETS_DST}" SYMBOLIC)
    endif()
endfunction()
