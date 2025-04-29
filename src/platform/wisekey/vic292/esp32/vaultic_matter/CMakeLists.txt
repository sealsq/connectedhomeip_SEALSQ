#
# CMakeLists.txt file for vaultic_wolfssl
#

set(LIBVAULTIC_API_2XX_DIR "${CMAKE_CURRENT_LIST_DIR}/third_party/connectedhomeip/src/platform/wisekey/vic292/VaultIC-Elib_292")
set(LIBVAULTIC_MATTERSRCS  "${CMAKE_CURRENT_LIST_DIR}/third_party/connectedhomeip/src/platform/wisekey/vic292")
set(CHIP_ROOT              "${CMAKE_CURRENT_LIST_DIR}/third_party/connectedhomeip")

set(COMPONENT_SRCDIRS
    "${LIBVAULTIC_API_2XX_DIR}/arch/embedded/common"
    "${LIBVAULTIC_API_2XX_DIR}/arch/embedded/esp32"
    "${LIBVAULTIC_API_2XX_DIR}/common"
    "${LIBVAULTIC_API_2XX_DIR}/common/comms"
    "${LIBVAULTIC_API_2XX_DIR}/crypto/certificate"
    "${LIBVAULTIC_API_2XX_DIR}/device/vaultic_2XX_family"
    "${LIBVAULTIC_MATTERSRCS}"
    ".."
    "."
)

set(COMPONENT_INCLUDEDIRS
    "${LIBVAULTIC_API_2XX_DIR}/arch/embedded/common"
    "${LIBVAULTIC_API_2XX_DIR}/common"
    "${LIBVAULTIC_API_2XX_DIR}/device/vaultic_2XX_family"
    "${LIBVAULTIC_API_2XX_DIR}/device/vaultic_2XX_family/vaultic_292"
    "${LIBVAULTIC_MATTERSRCS}"
    ".."
    "."
)

set(PRIV_REQUIRES_LIST chip driver)

if(CONFIG_VAULTIC_VCC_CONTROL)
    set(VAULTIC_VCC_CONTROL "VCC_CTRL_BY_GPIO")
endif()


if(CONFIG_VAULTIC_LOG_LEVEL)
	set(VAULTIC_LOG_LEVEL "VAULTIC_LOG_LEVEL=${CONFIG_VAULTIC_LOG_LEVEL}")
endif()

set(COMPONENT_CFLAGS "TARGETCHIP_VAULTIC_292" ${VAULTIC_LOG_LEVEL} ${VAULTIC_VCC_CONTROL})


idf_component_register( SRC_DIRS ${COMPONENT_SRCDIRS}
                        INCLUDE_DIRS ${COMPONENT_INCLUDEDIRS}
                        REQUIRES ${PRIV_REQUIRES_LIST}
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC ${COMPONENT_CFLAGS})
