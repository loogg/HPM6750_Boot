# Copyright 2021 hpmicro
# SPDX-License-Identifier: BSD-3-Clause

function(sdk_src)
    foreach(file ${ARGN})
        if(IS_DIRECTORY ${file})
            message(FATAL_ERROR "directory ${file} can't be added to sdk_lib_src")
        endif()
        if(IS_ABSOLUTE ${file})
            set(path ${file})
        else()
            set(path ${CMAKE_CURRENT_SOURCE_DIR}/${file})
        endif()
        target_sources(hpm_sdk_lib PUBLIC ${path})
    endforeach()
endfunction()

function(sdk_inc)
    foreach(inc ${ARGN})
        if(IS_ABSOLUTE ${inc})
            set(path ${inc})
        else()
            set(path ${CMAKE_CURRENT_SOURCE_DIR}/${inc})
        endif()
        target_include_directories(hpm_sdk_lib_itf INTERFACE ${path})
    endforeach()
endfunction()

function(sdk_sys_inc)
    foreach(inc ${ARGN})
        if(IS_ABSOLUTE ${inc})
            set(path ${inc})
        else()
            set(path ${CMAKE_CURRENT_SOURCE_DIR}/${inc})
        endif()
        target_include_directories(hpm_sdk_lib_itf SYSTEM INTERFACE ${path})
    endforeach()
endfunction()

function(sdk_get_compile_options opts)
    get_target_property(all_opts hpm_sdk_lib_itf INTERFACE_COMPILE_OPTIONS)
    string(JOIN " " output_opts ${all_opts})
    set(${opts} ${output_opts} PARENT_SCOPE)
endfunction()

function(sdk_linker_global_symbols)
    foreach(sym ${ARGN})
        target_link_libraries(hpm_sdk_lib_itf INTERFACE "-Xlinker --defsym=${sym}")
        get_property(link_symbols TARGET hpm_sdk_lib_itf PROPERTY INTERFACE_LINK_SYMBOLS)
        list(APPEND link_symbols "${sym}")
        set_target_properties(hpm_sdk_lib_itf PROPERTIES INTERFACE_LINK_SYMBOLS "${link_symbols}")
    endforeach()
endfunction()

function(sdk_compile_options)
    foreach(opt ${ARGN})
        target_compile_options(hpm_sdk_lib_itf INTERFACE ${opt})
    endforeach()
endfunction()

function(sdk_compile_definitions)
    foreach(def ${ARGN})
        target_compile_definitions(hpm_sdk_lib_itf INTERFACE ${def})
    endforeach()
endfunction()

function(sdk_link_libraries)
    foreach(lib ${ARGN})
        target_link_libraries(hpm_sdk_lib_itf INTERFACE ${lib})
    endforeach()
endfunction()

function(sdk_ld_options)
    foreach(opt ${ARGN})
        target_link_libraries(hpm_sdk_lib_itf INTERFACE ${opt})
    endforeach()
endfunction()

function(get_openocd_probe_name_of_board board openocd-probe)
    execute_process(
        COMMAND
        ${PYTHON_EXECUTABLE}
        ${HPM_SDK_BASE}/scripts/get_board_info.py
        ${HPM_SDK_BASE}/boards/${board}/${board}.yaml
        openocd-probe
        RESULT_VARIABLE result
        OUTPUT_VARIABLE openocd_probe
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    set(${openocd-probe} ${openocd_probe} PARENT_SCOPE)
endfunction()

function(get_openocd_soc_name_of_board board openocd-soc)
    execute_process(
        COMMAND
        ${PYTHON_EXECUTABLE}
        ${HPM_SDK_BASE}/scripts/get_board_info.py
        ${HPM_SDK_BASE}/boards/${board}/${board}.yaml
        openocd-soc
        RESULT_VARIABLE result
        OUTPUT_VARIABLE openocd_soc
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    set(${openocd-soc} ${openocd_soc} PARENT_SCOPE)
endfunction()

function(get_soc_name_of_board board soc)
    execute_process(
        COMMAND
        ${PYTHON_EXECUTABLE}
        ${HPM_SDK_BASE}/scripts/get_board_info.py
        ${HPM_SDK_BASE}/boards/${board}/${board}.yaml
        soc
        RESULT_VARIABLE result
        OUTPUT_VARIABLE soc_name
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    set(${soc} ${soc_name} PARENT_SCOPE)
endfunction()

function(get_compiler_version compiler version_text compiler_version)
    if("${compiler}" STREQUAL "gcc")
        execute_process(
            COMMAND
            ${PYTHON_EXECUTABLE}
            ${HPM_SDK_BASE}/scripts/get_gcc_version.py
            "${version_text}"
            RESULT_VARIABLE result
            OUTPUT_VARIABLE v
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )
        set(${compiler_version} ${v} PARENT_SCOPE)
    elseif("${compiler}" STREQUAL "clang")
        execute_process(
            COMMAND
            ${PYTHON_EXECUTABLE}
            ${HPM_SDK_BASE}/scripts/get_gcc_version.py
            "${version_text}"
            RESULT_VARIABLE result
            OUTPUT_VARIABLE v
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )
        set(${compiler_version} ${v} PARENT_SCOPE)

    else()
        message(FATAL_ERROR "Unsupported compiler ${compiler}")
    endif()
endfunction()

function(add_subdirectory_ifdef feature dir)
    if((${feature}) AND (NOT ${${feature}} EQUAL 0))
        add_subdirectory(${dir})
    endif()
endfunction()

function(sdk_inc_ifdef feature)
    if((${feature}) AND (NOT ${${feature}} EQUAL 0))
        sdk_inc(${ARGN})
    endif()
endfunction()

function(sdk_src_ifdef feature)
    if((${feature}) AND (NOT ${${feature}} EQUAL 0))
        sdk_src(${ARGN})
    endif()
endfunction()

function(sdk_clear_compile_options)
    set_target_properties(hpm_sdk_lib_itf PROPERTIES INTERFACE_COMPILE_OPTIONS "")
endfunction()

function(sdk_app_inc)
    foreach(inc ${ARGN})
        if(IS_ABSOLUTE ${inc})
            set(path ${inc})
        else()
            set(path ${CMAKE_CURRENT_SOURCE_DIR}/${inc})
        endif()
        target_include_directories(app PUBLIC ${path})
    endforeach()
endfunction()

function(sdk_app_src)
    foreach(file ${ARGN})
        if(IS_DIRECTORY ${file})
            message(FATAL_ERROR "directory ${file} can't be added to sdk_app_src")
        endif()
        if(IS_ABSOLUTE ${file})
            set(path ${file})
        else()
            set(path ${CMAKE_CURRENT_SOURCE_DIR}/${file})
        endif()
        target_sources(app PUBLIC ${path})
    endforeach()
endfunction()

function(check_board_capability board app_yaml result)
    execute_process(
        COMMAND
        ${PYTHON_EXECUTABLE}
        ${HPM_SDK_BASE}/scripts/check_board_cap.py
        ${HPM_SDK_BASE}/boards/${board}/${board}.yaml
        ${app_yaml}
        RESULT_VARIABLE r
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    set(${result} ${r} PARENT_SCOPE)
endfunction()

function(get_flash_size_of_board board size)
    execute_process(
        COMMAND
        ${PYTHON_EXECUTABLE}
        ${HPM_SDK_BASE}/scripts/get_board_info.py
        ${HPM_SDK_BASE}/boards/${board}/${board}.yaml
        flash_size
        RESULT_VARIABLE result
        OUTPUT_VARIABLE sz
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    set(${size} ${sz} PARENT_SCOPE)
endfunction()

function(get_extram_size_of_board board size)
    execute_process(
        COMMAND
        ${PYTHON_EXECUTABLE}
        ${HPM_SDK_BASE}/scripts/get_board_info.py
        ${HPM_SDK_BASE}/boards/${board}/${board}.yaml
        extram_size
        RESULT_VARIABLE result
        OUTPUT_VARIABLE sz
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    set(${size} ${sz} PARENT_SCOPE)
endfunction()

function(get_ses_debug_auto_start_gdb_server app_yaml start)
    execute_process(
        COMMAND
        ${PYTHON_EXECUTABLE}
        ${HPM_SDK_BASE}/scripts/get_app_debug_info.py
        ${app_yaml}
        ses
        auto_start_gdb_server
        RESULT_VARIABLE result
        OUTPUT_VARIABLE auto_start
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    set(${start} ${auto_start} PARENT_SCOPE)
endfunction()

function(get_ses_debug_gdb_server_port app_yaml port)
    execute_process(
        COMMAND
        ${PYTHON_EXECUTABLE}
        ${HPM_SDK_BASE}/scripts/get_app_debug_info.py
        ${app_yaml}
        ses
        gdb_server_port
        RESULT_VARIABLE result
        OUTPUT_VARIABLE p
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    set(${port} ${p} PARENT_SCOPE)
endfunction()

function(get_ses_debug_gdb_server_reset_command app_yaml rst_cmd)
    execute_process(
        COMMAND
        ${PYTHON_EXECUTABLE}
        ${HPM_SDK_BASE}/scripts/get_app_debug_info.py
        ${app_yaml}
        ses
        gdb_server_reset_command
        RESULT_VARIABLE result
        OUTPUT_VARIABLE p
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    set(${rst_cmd} ${p} PARENT_SCOPE)
endfunction()

function(check_excluded_targets app_yaml result)
    execute_process(
        COMMAND
        ${PYTHON_EXECUTABLE}
        ${HPM_SDK_BASE}/scripts/check_excluded_targets.py
        ${app_yaml}
        OUTPUT_VARIABLE r
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    set(${result} ${r} PARENT_SCOPE)
endfunction()
