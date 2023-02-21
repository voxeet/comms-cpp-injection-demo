cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIX AND NOT APPLE AND NOT ANDROID)
    # Non-standard preprocessor definition added by gcc:
	add_definitions(-Ulinux)
	set(LINUX TRUE)
endif()

project(cpp_injection_demo)

include(cmake/ffmpeg.cmake)
include(cmake/windows.cmake)

set(DOLBYIO_SHARE_DIR ${CMAKE_CURRENT_LIST_DIR}/ext-lib/sdk/share/)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# The SDK and the multimedia streaming addon provide the CMake package
# files, which define the imported targets. The recommended way of integrating
# the SDK in a cmake-based project is to use these:
if(NOT DOLBYIO_COMMS_MODULES_LOCATION)
	set(DOLBYIO_COMMS_MODULES_LOCATION ${DOLBYIO_SHARE_DIR}/dolbyio/comms/cmake)
endif()
find_package(DolbyioComms REQUIRED
	PATHS ${DOLBYIO_COMMS_MODULES_LOCATION}
)
find_package(DolbyioCommsMultiMediaStreamingAddon
	PATHS ${DOLBYIO_COMMS_MODULES_LOCATION}
)
get_target_property(DOLBYIO_SDK_HEADERS DolbyioComms::sdk_config INTERFACE_INCLUDE_DIRECTORIES)
if(NOT DOLBYIO_SDK_HEADERS)
	get_target_property(DOLBYIO_SDK_HEADERS DolbyioComms::sdk_config INCLUDE_DIRECTORIES)
endif()

# Use the media_source_file sample library from the SDK pacakge
add_subdirectory(${DOLBYIO_SHARE_DIR}/dolbyio/comms/sample/media_source)
add_subdirectory(src)

# Install the files to be injected and the python script
install(DIRECTORY conversations 
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/"
	USE_SOURCE_PERMISSIONS
)
install(FILES demo.py
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/"
)
install(FILES injection-input.json
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/"
)