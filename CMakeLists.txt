cmake_minimum_required(VERSION 3.10)

project(userLogin)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)

set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets Core Gui Sql REQUIRED)
include_directories(hdr)
include_directories(src)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

set(UI_FILES ui/mainWindowAdmin.ui ui/loginWindow.ui ui/mainWindowOperator.ui)
qt6_wrap_ui(UIHEADERS ${UI_FILES})


add_executable(userLogin
    src/main.cpp
    src/LogicDataBase.cpp
    src/MainWindow.cpp
    src/LoginWindow.cpp
    src/StartEndWidget.cpp
    ${UI_FILES}
)
include_directories(${SODIUM_INCLUDE_DIRS})

target_link_libraries(userLogin Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql ${SODIUM_LIBRARIES})

target_include_directories(userLogin PRIVATE ${CMAKE_BINARY_DIR})