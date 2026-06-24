cmake_minimum_required(VERSION 3.20)

project(nexspace_ai_lab_ui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Widgets QUIET)
if (Qt6_FOUND)
  set(QT_WIDGETS_LIB Qt6::Widgets)
else()
  find_package(Qt5 COMPONENTS Widgets REQUIRED)
  set(QT_WIDGETS_LIB Qt5::Widgets)
endif()

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
