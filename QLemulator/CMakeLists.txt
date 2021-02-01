cmake_minimum_required(VERSION 3.7)

project(iQLux)

set(CMAKE_C_FLAGS "-DLINUX -DQVFS -DUSE_IOSZ -DDO_GRAB \
                   -DSERIAL -DNEWSERIAL \
                   -DNEWPTY -DSH_MEM -DIPDEV \
                   -DHAS_STPCPY -DMOUSE -DUXWAIT -DNOSOUND \
                   -DNOTRACE")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-function-declaration")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSOUND")
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Og -ggdb")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
endif()

set(RBMAC_HEADERS
    mac/RB_macview.h mac/RB_toolbar.h
)

set(RBMAC_SOURCES
    mac/RB_macapp.m mac/RB_macview.m mac/RB_toolbar.m
)

set(RB_HEADERS
    RB_screen.h rb_begin_code.h rb_close_code.h rb_endian.h rb_virtual_keys.h
)

set(RB_SOURCES
    RB_screen.c
)

set(QL_HEADERS
    QDOS.h QL.h
    QL_disk.h QL_filespriv.h QL_instaddr.h
    QL_68000.h QL_config.h QL_hardware.h QL_mouse.h
    QL_screen.h QL_sound.h QL_files.h QL_ip.h QL_serial.h QL_vfs.h
)

set(QL_SOURCES
    QL_disk.c QL_basext.c QL_boot.c QL_cconv.c QL_config.c QL_driver.c QL_files.c QL_hardware.c QL_poll.c
    QL_screen.c QL_serial.c QL_sound.c QL_ip.c QL_serio.c QL_traps.c QL_vfs.c QL_mouse.c
)

set(BASE_HEADERS 
    base_boot.h base_cond.h base_cow.h base_driver.h base_emudisk.h base_general.h
    base_iexl.h base_iexl_general.h base_iptraps.h base_memaccess.h base_misdefs.h
    base_mmodes.h base_proto.h base_script.h base_bdi.h base_trace.h base_unix.h
    base_cfg.h base_util.h base_uxfile.h base_xc68.h base_xcodes.h
)

set(BASE_SOURCES
    emu_main.c emu_start.c emu_version.c
    base_init.c base_dummies.c base_general.c base_iexl_general.c base_instructions_ao.c
    base_instructions_pz.c base_memaccess.c base_pty.c base_qmtrap.c base_script.c
    base_trace.c base_cfg.c base_util.c base_uxfile.c base_vm.c
    base_xc68.c base_xcodes.c base_bdi.c
)

if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    add_executable(iQLux ${RBMAC_SOURCES} ${RB_SOURCES} ${QL_SOURCES} ${BASE_SOURCES} ${RBMAC_HEADERS} ${RB_HEADERS} ${QL_HEADERS} ${BASE_HEADERS})
else()
    add_executable(iQLux  ${RB_SOURCES} ${QL_SOURCES} ${BASE_SOURCES} ${RB_HEADERS} ${QL_HEADERS} ${BASE_HEADERS)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL Darwin)
    target_link_libraries(iQLux PUBLIC
        "-framework QuartzCore"
        "-framework Cocoa"
        "-framework MetalKit"
        "-framework Metal")
endif()

source_group("RB Sources" FILES ${RB_SOURCES} ${RB_HEADERS} ${RBMAC_SOURCES} ${RBMAC_HEADERS})
source_group("QL Sources" FILES ${QL_SOURCES} ${QL_HEADERS})
source_group("Base Sources" FILES ${BASE_SOURCES} ${BASE_HEADERS})
