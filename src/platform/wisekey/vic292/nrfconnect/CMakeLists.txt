
set(LIBVAULTIC_API_2XX_DIR "${CMAKE_CURRENT_LIST_DIR}/third_party/connectedhomeip/src/platform/wisekey/vic292/VaultIC-Elib_292")
set(LIBVAULTIC_MATTERSRCS  "${CMAKE_CURRENT_LIST_DIR}/third_party/connectedhomeip/src/platform/wisekey/vic292")
set(CHIP_ROOT              "${CMAKE_CURRENT_LIST_DIR}/third_party/connectedhomeip")

# zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(KCONFIG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Kconfig.vic292)

target_include_directories(app PRIVATE
    ${LIBVAULTIC_API_2XX_DIR}/arch/embedded/common/
    ${LIBVAULTIC_API_2XX_DIR}/arch/embedded/common/nrfconnect
    ${LIBVAULTIC_API_2XX_DIR}/common/
    ${LIBVAULTIC_API_2XX_DIR}/common/comms/
    ${LIBVAULTIC_API_2XX_DIR}/device/vaultic_2XX_family/
    ${LIBVAULTIC_MATTERSRCS}
)

target_sources(app PRIVATE
    ${LIBVAULTIC_API_2XX_DIR}/arch/embedded/common/vaultic_mem.c
    ${LIBVAULTIC_API_2XX_DIR}/arch/embedded/nrfconnect/vaultic_control_driver.c
    ${LIBVAULTIC_API_2XX_DIR}/arch/embedded/nrfconnect/vaultic_timer_delay.c
    ${LIBVAULTIC_API_2XX_DIR}/common/vaultic_command.c
    ${LIBVAULTIC_API_2XX_DIR}/common/vaultic_crc16.c
    ${LIBVAULTIC_API_2XX_DIR}/common/vaultic_utils.c
    ${LIBVAULTIC_API_2XX_DIR}/common/comms/vaultic_block_protocol.c
    ${LIBVAULTIC_API_2XX_DIR}/common/comms/vaultic_comms.c
    ${LIBVAULTIC_API_2XX_DIR}/common/comms/vaultic_protocol.c
    ${LIBVAULTIC_API_2XX_DIR}/device/vaultic_2XX_family/vaultic_api.c
    ${LIBVAULTIC_API_2XX_DIR}/arch/embedded/common/vaultic_twi_peripheral.c
    ${LIBVAULTIC_API_2XX_DIR}/arch/embedded/nrfconnect/vaultic_twi_driver.c
    ${LIBVAULTIC_MATTERSRCS}/vaultic_matter.cpp
)

if(CONFIG_VAULTIC_LOG_LEVEL)
	set(VAULTIC_LOG_LEVEL "VAULTIC_LOG_LEVEL=${CONFIG_VAULTIC_LOG_LEVEL}")
endif()

set(NRF_292_CFLAGS "TARGETCHIP_VAULTIC_292" "VCC_CTRL_BY_GPIO" ${VAULTIC_LOG_LEVEL} )

target_link_libraries(app PUBLIC zephyr_interface)

target_compile_definitions(app PRIVATE ${NRF_292_CFLAGS})
