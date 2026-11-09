add_library(modules INTERFACE)

target_sources(modules INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/debug/Logger.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/looper/Looper.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/motor/DJI_M3508.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/remote_control/SBUS_RC.c
    ${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/remote_control/FSi6x.c
)

target_include_directories(modules INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 引入测试文件（只有启用时需要）
# include(${CMAKE_CURRENT_SOURCE_DIR}/euuen_mod/test/CMakeLists.txt)