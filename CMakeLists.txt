cmake_minimum_required(VERSION 3.10)

project(crm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets Core Gui Sql REQUIRED)
find_library(SODIUM_LIBRARY sodium REQUIRED)

set(UI_FILES
    ui/mainWindowAdmin.ui
    ui/loginWindow.ui
    ui/mainWindowOperator.ui
)

qt6_wrap_ui(UI_HEADERS ${UI_FILES})

set(SRC_FILES
    src/main.cpp
    src/CRMDatabaseManager.cpp
    src/EmployeeLoginForm.cpp
    src/DashboardWindow.cpp
    src/SessionDetailWidget.cpp
)

add_executable(crm ${SRC_FILES} ${UI_HEADERS})

target_include_directories(crm PRIVATE ${CMAKE_BINARY_DIR} hdr src)

target_link_libraries(crm PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql ${SODIUM_LIBRARY})
