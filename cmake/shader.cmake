find_program(SLANGC_EXECUTABLE slangc)

if(NOT SLANGC_EXECUTABLE)
    message(WARNING "slangc compiler not found. Shader compilation will be disabled.")
endif()

function(target_compile_slang_shaders TARGET)
    if(NOT SLANGC_EXECUTABLE)
        return()
    endif()

    set(SHADER_OUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders")
    file(MAKE_DIRECTORY "${SHADER_OUT_DIR}")

    set(COMPILED_SHADERS "")

    # Iterate over all arguments passed after the TARGET name
    foreach(SHADER ${ARGN})
        get_filename_component(SHADER_ABS "${SHADER}" ABSOLUTE)
        get_filename_component(SHADER_NAME "${SHADER_ABS}" NAME)
        # Use full filename (e.g., triangle.slang.spv) to avoid clashes or just change extension
        string(REPLACE ".slang" ".spv" SPV_NAME ${SHADER_NAME})
        set(OUTPUT_SPV "${SHADER_OUT_DIR}/${SPV_NAME}")

        add_custom_command(
            OUTPUT "${OUTPUT_SPV}"
            COMMAND ${SLANGC_EXECUTABLE} "${SHADER_ABS}" -target spirv -o "${OUTPUT_SPV}"
            DEPENDS "${SHADER_ABS}"
            COMMENT "Compiling Slang shader ${SHADER_NAME} to SPIR-V"
            VERBATIM
        )

        list(APPEND COMPILED_SHADERS "${OUTPUT_SPV}")
    endforeach()

    if(COMPILED_SHADERS)
        add_custom_target(${TARGET}_shaders DEPENDS ${COMPILED_SHADERS})
        add_dependencies(${TARGET} ${TARGET}_shaders)
    endif()
endfunction()
