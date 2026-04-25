cmake_minimum_required(VERSION 3.15)
project(ya_hub)

# Устанавливаем стандарт C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Находим пакет glib-2.0 и его заголовочные файлы
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)

# Ищем библиотеки MySQL C++ Connector и Mosquitto
find_library(MYSQLCPP_LIB NAMES mysqlcppconn8 libmysqlcppconn8)
if(NOT MYSQLCPP_LIB)
    message(FATAL_ERROR "Библиотека mysqlcppconn8 не найдена. Установите mysql-connector-cpp.")
endif()

find_library(MOSQUITTO_LIB NAMES mosquitto libmosquitto)
if(NOT MOSQUITTO_LIB)
    message(FATAL_ERROR "Библиотека mosquitto не найдена. Установите mosquitto-dev.")
endif()

# Собираем список всех .cpp файлов в текущей директории
file(GLOB CURRENT_SOURCES "*.cpp")

# Собираем список всех .cpp файлов в директории ../functions/src
file(GLOB FUNCTIONS_SOURCES "../functions/src/*.cpp")

# Объединяем все исходные файлы в один список
set(ALL_SOURCES
    ${CURRENT_SOURCES}
    ${FUNCTIONS_SOURCES}
)

# Создаём исполняемый файл из всех собранных исходных файлов
add_executable(ya_hub ${ALL_SOURCES})

# Указываем директории для поиска заголовочных файлов:
# - текущая директория
# - директория ../functions/src
# - заголовочные файлы glib-2.0 (из pkg-config)
target_include_directories(ya_hub PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../functions/src
    ${GLIB2_INCLUDE_DIRS}
)

# Добавляем общие флаги компилятора
target_compile_options(ya_hub PRIVATE
    -Wall
    -Wextra
    -O2
    -pipe
    -march=native
    -MMD
    -MP
    -Wpmf-conversions
    -fexceptions
)

# Настройки для конфигурации Debug
target_compile_definitions(ya_hub
    PRIVATE
        $<$<CONFIG:Debug>:__DEBUG__>
)

target_compile_options(ya_hub PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
)

# Настройки для конфигурации Release
target_compile_definitions(ya_hub
    PRIVATE
        $<$<CONFIG:Release>:__RELEASE__>
)

target_compile_options(ya_hub PRIVATE
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# Указываем библиотеки для линковки
target_link_libraries(ya_hub PRIVATE
    pthread
    ${GLIB2_LIBRARIES}
    wt
    wthttp
    ${MYSQLCPP_LIB}
    ${MOSQUITTO_LIB}
)

