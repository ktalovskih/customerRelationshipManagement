cmake_minimum_required(VERSION 3.10)

project(crm)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)

set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets Core Gui Sql REQUIRED)
include_directories(hdr)
include_directories(src)


set(UI_FILES ui/mainWindowAdmin.ui ui/loginWindow.ui ui/mainWindowOperator.ui)
qt6_wrap_ui(UIHEADERS ${UI_FILES})


add_executable(crm
    src/main.cpp
    src/CRMDatabaseManager.cpp
    src/EmployeeLoginForm.cpp
    src/DashboardWindow.cpp
    src/SessionDetailWidget.cpp
    ${UI_FILES}
)
include_directories(${SODIUM_INCLUDE_DIRS})

target_link_libraries(crm Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql)

target_include_directories(crm PRIVATE ${CMAKE_BINARY_DIR})