cmake_minimum_required(VERSION 3.20)

project(nexspace_ai_lab_ui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(QT_DEPLOY_DIR "${CMAKE_SOURCE_DIR}/dist/${PROJECT_NAME}")

find_package(Qt6 COMPONENTS Widgets QUIET)
if (Qt6_FOUND)
  set(QT_WIDGETS_LIB Qt6::Widgets)
else()
  find_package(Qt5 COMPONENTS Widgets REQUIRED)
    set(QT_WIDGETS_LIB Qt5::Widgets)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${QT_DEPLOY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${QT_DEPLOY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${QT_DEPLOY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${QT_DEPLOY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${QT_DEPLOY_DIR}")

set(SRC_FILES
    src/main.cpp
    src/domain.h
    src/adapters/engine_adapter.h
    src/adapters/mock_engine_adapter.h
    src/adapters/mock_engine_adapter.cpp
    src/adapters/real_engine_adapter.h
    src/adapters/real_engine_adapter.cpp
    src/adapters/noop_engine_adapter.h
    src/adapters/noop_engine_adapter.cpp
    src/sound/soundCueTypes.h
    src/sound/soundCueAdapter.h
    src/sound/noopSoundCueAdapter.h
    src/sound/noopSoundCueAdapter.cpp
    src/sound/soundCueService.h
    src/sound/soundCueService.cpp
    src/widgets/top_status_bar.h
    src/widgets/top_status_bar.cpp
    src/widgets/command_input_bar.h
    src/widgets/command_input_bar.cpp
    src/widgets/command_draft_panel.h
    src/widgets/command_draft_panel.cpp
    src/widgets/command_input_page.h
    src/widgets/command_input_page.cpp
    src/widgets/dashboard_page.h
    src/widgets/dashboard_page.cpp
    src/widgets/ticket_board.h
    src/widgets/ticket_board.cpp
    src/widgets/ticket_detail_panel.h
    src/widgets/ticket_detail_panel.cpp
    src/widgets/agent_list_page.h
    src/widgets/agent_list_page.cpp
    src/widgets/audit_log_table.h
    src/widgets/audit_log_table.cpp
    src/widgets/history_table.h
    src/widgets/history_table.cpp
    src/widgets/settings_page.h
    src/widgets/settings_page.cpp
    src/widgets/system_status_panel.h
    src/widgets/system_status_panel.cpp
    src/widgets/app_shell.h
    src/widgets/app_shell.cpp
)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${QT_WIDGETS_LIB})

if(WIN32)
  set(WINDEPLOY_SEARCH_HINTS)
  if(DEFINED Qt5_DIR)
    get_filename_component(WINDEPLOY_HINT_DIR "${Qt5_DIR}" DIRECTORY)
    get_filename_component(WINDEPLOY_HINT_DIR "${WINDEPLOY_HINT_DIR}" DIRECTORY)
    get_filename_component(WINDEPLOY_HINT_DIR "${WINDEPLOY_HINT_DIR}" DIRECTORY)
    list(APPEND WINDEPLOY_SEARCH_HINTS "${WINDEPLOY_HINT_DIR}/bin")
  endif()
  if(DEFINED Qt6_DIR)
    get_filename_component(WINDEPLOY_HINT_DIR "${Qt6_DIR}" DIRECTORY)
    get_filename_component(WINDEPLOY_HINT_DIR "${WINDEPLOY_HINT_DIR}" DIRECTORY)
    get_filename_component(WINDEPLOY_HINT_DIR "${WINDEPLOY_HINT_DIR}" DIRECTORY)
    list(APPEND WINDEPLOY_SEARCH_HINTS "${WINDEPLOY_HINT_DIR}/bin")
  endif()
  find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt HINTS ${WINDEPLOY_SEARCH_HINTS})
  if(WINDEPLOYQT_EXECUTABLE)
    add_custom_command(TARGET ${PROJECT_NAME}
      POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${QT_DEPLOY_DIR}"
      COMMAND "${WINDEPLOYQT_EXECUTABLE}" --dir "${QT_DEPLOY_DIR}" "$<TARGET_FILE:${PROJECT_NAME}>"
      COMMENT "Deploying Qt runtime to ${QT_DEPLOY_DIR} (Qt runtime + platforms/plugin)"
    )
  else()
    message(WARNING "windeployqt not found. Build will produce exe only; dist/nexspace_ai_lab_ui will not include Qt DLLs automatically.")
  endif()
endif()
