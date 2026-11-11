add_library(modules INTERFACE)

target_sources(modules INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/interface/euuen_c_board_bsp/sys.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/interface/euuen_c_board_bsp/gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/interface/euuen_c_board_bsp/spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/interface/euuen_c_board_bsp/i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/interface/euuen_c_board_bsp/can.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/interface/euuen_c_board_bsp/usart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/euuen_mod.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/state_type.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/mem_allocator.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/peripheral/spi_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/peripheral/i2c_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/peripheral/can_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/peripheral/usart_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/sensor/bmi088.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/sensor/ist8310.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/sensor/mahony.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/debug/Logger.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/looper/Looper.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/motor/DJI_M3508.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/remote_control/SBUS_RC.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/remote_control/FSi6x.c
)

target_link_options(modules INTERFACE
    -u _printf_float
)

target_include_directories(modules INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 引入测试文件（只有启用时需要）
include(${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/test/CMakeLists.txt)