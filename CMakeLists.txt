cmake_minimum_required(VERSION 3.16)

# Название проекта и язык
project(lab_device LANGUAGES CXX)

# Стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- GoogleTest через FetchContent ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# Для MSVC: единый CRT, чтобы не было конфликтов
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --- Таргет обычного приложения (со своим main() в device.cpp) ---
add_executable(device_app device.cpp)

# --- Таргет тестов (device.cpp подключается ВНУТРИ tests/device_test.cpp через #include "../device.cpp") ---
add_executable(device_tests tests/device_test.cpp)
# В тестовой сборке отключаем main() и ручные тесты из device.cpp
target_compile_definitions(device_tests PRIVATE UNIT_TESTS)
target_link_libraries(device_tests PRIVATE GTest::gtest_main)

# --- Регистрация и автодискавер тестов ---
enable_testing()
include(GoogleTest)

# Для Visual Studio/мульти-конфигурационных генераторов так надёжнее
gtest_discover_tests(device_tests
  DISCOVERY_MODE PRE_TEST
)

# На всякий случай: прямой запуск бинарника тестов как одного теста
add_test(NAME device_all COMMAND device_tests)
