cmake_minimum_required(VERSION 3.10)
project(ENIGMA VERSION 1.0.0 LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compilation options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Find SimGrid
# First try with find_package
find_package(SimGrid 4.1 QUIET)

# If not found, try manual paths
if(NOT SimGrid_FOUND)
    message(STATUS "SimGrid not found via find_package, trying manual paths...")
    
    # Check common installation paths
    set(SIMGRID_SEARCH_PATHS
        /opt/simgrid-4.1
        /usr/local
        /opt
        $ENV{HOME}/simgrid
        $ENV{SIMGRID_ROOT}
    )
    
    foreach(SEARCH_PATH ${SIMGRID_SEARCH_PATHS})
        if(EXISTS "${SEARCH_PATH}/include/simgrid/s4u.hpp" OR 
           EXISTS "${SEARCH_PATH}/include/simgrid/simix.h")
            set(SimGrid_INCLUDE_DIR "${SEARCH_PATH}/include")
            
            # Look for library
            find_library(SimGrid_LIBRARY
                NAMES simgrid
                PATHS "${SEARCH_PATH}/lib" "${SEARCH_PATH}/lib64"
                NO_DEFAULT_PATH
            )
            
            if(SimGrid_LIBRARY)
                set(SimGrid_FOUND TRUE)
                message(STATUS "Found SimGrid in: ${SEARCH_PATH}")
                message(STATUS "  Include: ${SimGrid_INCLUDE_DIR}")
                message(STATUS "  Library: ${SimGrid_LIBRARY}")
                break()
            endif()
        endif()
    endforeach()
endif()

# Final check
if(NOT SimGrid_FOUND)
    message(FATAL_ERROR "SimGrid not found. Please install SimGrid 4.1+ or set CMAKE_PREFIX_PATH to SimGrid installation directory.")
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${SimGrid_INCLUDE_DIR}
)

# Main library - Platform Generator
add_library(enigma_platform STATIC
    src/platform/PlatformGenerator.cpp
    src/platform/PlatformBuilder.cpp
    src/platform/EdgePlatform.cpp
    src/platform/FogPlatform.cpp
    src/platform/CloudPlatform.cpp
    src/utils/XMLWriter.cpp
)

target_link_libraries(enigma_platform ${SimGrid_LIBRARY})

# MQTT library (Communication module)
add_library(enigma_mqtt STATIC
    src/comms/mqtt/MQTTBroker.cpp
    src/comms/mqtt/MQTTPublisher.cpp
    src/comms/mqtt/MQTTSubscriber.cpp
)

target_link_libraries(enigma_mqtt ${SimGrid_LIBRARY})

# Platform generator (standalone tool)
add_executable(platform_generator
    src/tools/platform_generator_main.cpp
)

target_link_libraries(platform_generator enigma_platform ${SimGrid_LIBRARY})

# Test applications (formerly example applications)
add_executable(edge_computing_app
    tests/edge_computing.cpp
)
target_link_libraries(edge_computing_app enigma_platform enigma_mqtt ${SimGrid_LIBRARY})

add_executable(fog_analytics_app
    tests/fog_analytics.cpp
)
target_link_libraries(fog_analytics_app enigma_platform enigma_mqtt ${SimGrid_LIBRARY})

add_executable(hybrid_cloud_app
    tests/hybrid_cloud.cpp
)
target_link_libraries(hybrid_cloud_app enigma_platform enigma_mqtt ${SimGrid_LIBRARY})

add_executable(data_offloading_app
    tests/data_offloading.cpp
)
target_link_libraries(data_offloading_app enigma_platform enigma_mqtt ${SimGrid_LIBRARY})

# MQTT Test Application
add_executable(mqtt_edge_app
    tests/mqtt_edge_app.cpp
)
target_link_libraries(mqtt_edge_app enigma_platform enigma_mqtt ${SimGrid_LIBRARY})

# Ping-pong latency measurement app
add_executable(pingpong_fit_to_g5k_app
    tests/pingpong_fit_to_g5k_app.cpp
)
target_link_libraries(pingpong_fit_to_g5k_app enigma_platform ${SimGrid_LIBRARY})


# Distributed block write app for Grid'5000
add_executable(fit_to_g5k_app
    tests/fit_to_g5k_app.cpp
)
target_link_libraries(fit_to_g5k_app enigma_platform ${SimGrid_LIBRARY})

# Distributed block write app for Grid'5000
add_executable(fit_to_g5k_app_v6
    tests/fit_to_g5k_app_v6.cpp
)
target_link_libraries(fit_to_g5k_app_v6 enigma_platform ${SimGrid_LIBRARY})

# Generated template apps: Uncomment and customize after running platform_generator --generate-app
#add_executable(my_test_platform_app
#    tests/my_test_platform_app.cpp
#)
#target_link_libraries(my_test_platform_app enigma_platform ${SimGrid_LIBRARY})

# Example: generate all platforms
#add_executable(generate_all_platforms
#    examples/generate_all_platforms.cpp
#)
#target_link_libraries(generate_all_platforms enigma_platform ${SimGrid_LIBRARY})

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Create directories for platforms and deployments
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/platforms)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/deployments)

# Installation
install(TARGETS enigma_platform enigma_mqtt platform_generator
        edge_computing_app fog_analytics_app hybrid_cloud_app data_offloading_app
        mqtt_edge_app
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(DIRECTORY include/ DESTINATION include)

# Information messages
message(STATUS "ENIGMA Platform Generator Configuration")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
if(SimGrid_VERSION)
    message(STATUS "  SimGrid: ${SimGrid_VERSION}")
else()
    message(STATUS "  SimGrid: Found (version unknown)")
endif()
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
