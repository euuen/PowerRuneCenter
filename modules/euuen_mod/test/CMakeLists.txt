# module test（每次只启用一个，避免 APP_Setup/APP_Loop 重复定义）
target_sources(modules INTERFACE
    # ${CMAKE_CURRENT_LIST_DIR}/led.c
    # ${CMAKE_CURRENT_LIST_DIR}/logger.c
    # ${CMAKE_CURRENT_LIST_DIR}/key.c
    # ${CMAKE_CURRENT_LIST_DIR}/looper.c
    # ${CMAKE_CURRENT_LIST_DIR}/m3508.c
    # ${CMAKE_CURRENT_LIST_DIR}/sbus_rc.c
    # ${CMAKE_CURRENT_LIST_DIR}/fsi6x.c
    # ${CMAKE_CURRENT_LIST_DIR}/bmi088.c
    # ${CMAKE_CURRENT_LIST_DIR}/ist8310.c
    ${CMAKE_CURRENT_LIST_DIR}/mahony.c
    # ${CMAKE_CURRENT_LIST_DIR}/buzzer.c
)
