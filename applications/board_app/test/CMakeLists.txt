# application test（每次只启用一个，避免 APP_Setup/APP_Loop 重复定义）
target_sources(applications INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/can.c
    # ${CMAKE_CURRENT_LIST_DIR}/key.c
    #  ${CMAKE_CURRENT_LIST_DIR}/light.c
)