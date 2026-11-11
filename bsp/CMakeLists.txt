add_library(bsp INTERFACE)

target_sources(bsp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/c_board_bsp/buzzer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/c_board_bsp/can.c
    ${CMAKE_CURRENT_SOURCE_DIR}/c_board_bsp/gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/c_board_bsp/i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/c_board_bsp/key.c
    ${CMAKE_CURRENT_SOURCE_DIR}/c_board_bsp/led.c
    ${CMAKE_CURRENT_SOURCE_DIR}/c_board_bsp/spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/c_board_bsp/sys.c
    ${CMAKE_CURRENT_SOURCE_DIR}/c_board_bsp/usart.c
)

target_include_directories(bsp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/c_board_bsp
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 引入测试文件（只有启用时需要）
include(${CMAKE_CURRENT_SOURCE_DIR}/c_board_bsp/test/CMakeLists.txt)