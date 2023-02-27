cmake_minimum_required(VERSION 3.13)

# Ustaw nazwę projektu.
project(netsim)

# Ustaw standard C++ (tu: C++17).
set(CMAKE_CXX_STANDARD 17)

# Dodaj flagi kompilacji wymuszające pisanie kodu zgodnego ze standardem.
#add_compile_options(-Wall -Wextra -Werror -Wpedantic -pedantic-errors)

# Dodaj katalogi z plikami nagłówkowymi dla wszystkich konfiguracji.
include_directories(
        include
)

# Ustaw zmienną `SOURCE_FILES`, która będzie przechowywać ścieżki do
# tych plików źródłowych, które będą niezbędne w każdej konfiguracji.
set(SOURCE_FILES
        src/package.cpp
        src/storage_types.cpp
        src/helpers.cpp
        src/factory.cpp
        src/nodes.cpp
        )


# Dodaj konfigurację typu `Debug`.
add_executable(netsim_debug ${SOURCE_FILES} main.cpp)

# == Unit testing using Google Testing Framework ==

# Ustaw zmienną `SOURCES_FILES_TESTS`, która będzie przechowywać ścieżki do
# plików źródłowych z testami jednostkowymi.
set(SOURCES_FILES_TESTS
        google_tests/netsim_test.cpp
        google_tests/netsim_tests/test/test_package.cpp
        google_tests/netsim_tests/test/test_storage_types.cpp
        google_tests/netsim_tests/test/test_nodes.cpp
        google_tests/netsim_tests/test/test_Factory.cpp
        google_tests/netsim_tests/test/test_factory_io.cpp
        )
# Dodaj konfigurację typu `Test`.
add_executable(netsim_test ${SOURCE_FILES} ${SOURCES_FILES_TESTS} google_tests/netsim_tests/test/main_gtest.cpp)

target_compile_definitions(netsim_test PUBLIC EXERCISE_ID=EXERCISE_ID_FACTORY)
# Dodaj katalogi z plikami nagłówkowymi wyłącznie do konkretnej konfiguracji (tu: `Test`).
target_include_directories(netsim_test PUBLIC
        google_tests/lib/googletest/include
        google_tests/lib/googlemock/include
        google_tests/netsim_tests/include
        google_tests/netsim_tests/mocks
        )

# Dodaj podkatalog `googletest-master`.
add_subdirectory(google_tests/lib)

# Podlinkuj bibliotekę o identyfikatorze `gmock` (w pliku CMake) wyłącznie do konkretnej
# konfiguracji (tu: `Test`).
target_link_libraries(netsim_test gmock)
