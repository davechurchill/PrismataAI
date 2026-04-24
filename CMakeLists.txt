cmake_minimum_required(VERSION 3.21)

project(PrismataAI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "PrismataAI CMake builds are x64-only. Configure with a 64-bit compiler, or with -A x64 when using Visual Studio.")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

option(PRISMATA_BUILD_GUI "Build the SFML GUI executable" ON)

add_library(prismata_options INTERFACE)
target_compile_features(prismata_options INTERFACE cxx_std_17)
target_compile_definitions(prismata_options INTERFACE
    NOMINMAX
    $<$<PLATFORM_ID:Windows>:WIN32>
)
target_include_directories(prismata_options INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/source"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/rapidjson"
)
target_compile_options(prismata_options INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/EHsc>
)

file(GLOB PRISMATA_ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/engine/*.cpp"
)

add_library(Prismata_Engine STATIC ${PRISMATA_ENGINE_SOURCES})
target_link_libraries(Prismata_Engine PUBLIC prismata_options)
target_include_directories(Prismata_Engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/source/engine"
)

file(GLOB PRISMATA_AI_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/ai/*.cpp"
)

add_library(Prismata_AI STATIC ${PRISMATA_AI_SOURCES})
target_link_libraries(Prismata_AI PUBLIC Prismata_Engine)
target_include_directories(Prismata_AI PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/source/ai"
)

file(GLOB PRISMATA_STANDALONE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/standalone/*.cpp"
)

add_executable(Prismata_Standalone ${PRISMATA_STANDALONE_SOURCES})
target_link_libraries(Prismata_Standalone PRIVATE Prismata_AI Prismata_Engine)
target_include_directories(Prismata_Standalone PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/source/standalone"
)

file(GLOB PRISMATA_TESTING_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/testing/*.cpp"
)

add_executable(Prismata_Testing ${PRISMATA_TESTING_SOURCES})
target_link_libraries(Prismata_Testing PRIVATE Prismata_AI Prismata_Engine)
target_include_directories(Prismata_Testing PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/source/testing"
)

if(PRISMATA_BUILD_GUI)
    find_package(SFML 3 COMPONENTS Audio Graphics Main Network System Window REQUIRED CONFIG)

    file(GLOB PRISMATA_GUI_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/source/gui/*.cpp"
    )

    add_executable(Prismata_GUI ${PRISMATA_GUI_SOURCES})
    target_link_libraries(Prismata_GUI PRIVATE
        Prismata_AI
        Prismata_Engine
        SFML::Audio
        SFML::Graphics
        SFML::Main
        SFML::Network
        SFML::System
        SFML::Window
    )
    target_include_directories(Prismata_GUI PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/source/gui"
    )

    if(WIN32)
        add_custom_command(TARGET Prismata_GUI POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:Prismata_GUI>
                $<TARGET_FILE_DIR:Prismata_GUI>
            COMMAND_EXPAND_LISTS
        )
    endif()
endif()
