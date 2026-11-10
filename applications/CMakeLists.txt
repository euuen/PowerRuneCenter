add_library(applications INTERFACE)

target_sources(applications INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/board_app/fsm.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/board_app/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/board_app/protocol.c
)

target_include_directories(applications INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/board_app
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 引入测试文件（只有启用时需要）
# include(${CMAKE_CURRENT_SOURCE_DIR}/board_app/test/CMakeLists.txt)