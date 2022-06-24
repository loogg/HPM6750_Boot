# Copyright 2021 hpmicro
# SPDX-License-Identifier: BSD-3-Clause

sdk_inc(${TOOLCHAIN_HOME}/lib/gcc/${CROSS_COMPILE_TARGET}/${COMPILER_VERSION}/include)
sdk_sys_inc(${TOOLCHAIN_HOME}/${CROSS_COMPILE_TARGET}/include/c++/${COMPILER_VERSION})
sdk_compile_options("-Wall")
sdk_compile_options("-Wno-format")
sdk_compile_options("-fomit-frame-pointer")
sdk_compile_options("-fno-builtin")
sdk_compile_options("-ffunction-sections")
sdk_compile_options("-fdata-sections")

add_library(hpm_sdk_nds_lib_itf INTERFACE)

function(sdk_nds_compile_options)
    foreach(opt ${ARGN})
        target_compile_options(hpm_sdk_nds_lib_itf INTERFACE ${opt})
    endforeach()
endfunction()

function(sdk_nds_compile_definitions)
    foreach(def ${ARGN})
        target_compile_definitions(hpm_sdk_nds_lib_itf INTERFACE ${def})
    endforeach()
endfunction()

function(sdk_nds_link_libraries)
    foreach(lib ${ARGN})
        target_link_libraries(hpm_sdk_nds_lib_itf INTERFACE ${lib})
    endforeach()
endfunction()

function(sdk_nds_ld_options)
    foreach(opt ${ARGN})
        target_link_libraries(hpm_sdk_nds_lib_itf INTERFACE ${opt})
    endforeach()
endfunction()

if("${TOOLCHAIN_VARIANT}" STREQUAL "nds-gcc")
# By default, Andes' gcc toolchain uses lld as linker
include(${HPM_SDK_BASE}/cmake/toolchain/lld.cmake)
elseif("${TOOLCHAIN_VARIANT}" STREQUAL "gcc")
include(${HPM_SDK_BASE}/cmake/toolchain/ld.cmake)
endif()

function (generate_bin2c_array c_array_path)
    add_custom_command(
        TARGET ${APP_ELF_NAME}
        COMMAND "python" $ENV{HPM_SDK_BASE}/scripts/bin2c.py ${APP_BIN_NAME} sec_core_img > ${c_array_path}
    )
endfunction ()
