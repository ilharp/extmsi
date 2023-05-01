cmake_minimum_required(VERSION 3.0)

############################################################
# Project
############################################################

project(extmsi
    VERSION 0.1.0
    DESCRIPTION "Extract Embedded MSI Resources from Executables"
    HOMEPAGE_URL "https://github.com/ilharp/extmsi"
    LANGUAGES C CXX
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_MESSAGE_LOG_LEVEL VERBOSE)

############################################################
# App
############################################################

add_executable(extmsi
    src/main.cpp
)

target_include_directories(extmsi
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

############################################################
# Compile Flags
############################################################

target_compile_definitions(extmsi
    PRIVATE
        UNICODE
        _UNICODE
        WIN32
        _WIN32
        WINDOWS
        _WINDOWS
)

if (MSVC)
    target_compile_options(extmsi
        PRIVATE
            /utf-8
    )
else()
    target_compile_options(extmsi
        PRIVATE
            -fexec-charset=UTF-8
            -finput-charset=UTF-8
    )
endif()

############################################################
# Support Visual Studio "Edit and Continue"
############################################################

if (CMAKE_GENERATOR MATCHES "Visual Studio")
    target_compile_options(extmsi
        PRIVATE
            /ZI
    )

    target_link_options(extmsi
        PRIVATE
            /INCREMENTAL
    )
endif()
