cmake_minimum_required(VERSION 3.10)

project(crm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Find Qt and Libsodium
find_package(Qt6 COMPONENTS Widgets Core Gui Sql REQUIRED)
find_library(SODIUM_LIBRARY sodium REQUIRED)

# Add UI Files
set(UI_FILES
    ui/mainWindowAdmin.ui
    ui/loginWindow.ui
    ui/mainWindowOperator.ui
)

qt6_wrap_ui(UI_HEADERS ${UI_FILES})

# Source Files
set(SRC_FILES
    src/main.cpp
    src/CRMDatabaseManager.cpp
    src/EmployeeLoginForm.cpp
    src/DashboardWindow.cpp
    src/SessionDetailWidget.cpp
)

# Executable
add_executable(crm ${SRC_FILES} ${UI_HEADERS})

# Ensure the UI headers and includes are properly handled
target_include_directories(crm PRIVATE ${CMAKE_BINARY_DIR} hdr src)

# Link Libraries
target_link_libraries(crm PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql ${SODIUM_LIBRARY})
