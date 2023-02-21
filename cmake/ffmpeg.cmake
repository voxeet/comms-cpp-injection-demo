# The sample code uses ffmpeg library. The ffmpeg provided in the package
# is used by the SDK and the multimedia streaming addon, but it's not a part
# of the Dolby products' interfaces. The application does not have to explicitly
# use it. The sample code includes it in a simplest possible way, by hardcoding
# the paths:
set(FFMPEG_ROOT_DIR ${CMAKE_SOURCE_DIR}/ext-lib/sdk)
set(FFMPEG_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/ext-lib/sdk/share/dolbyio/comms/sample/ffmpeg-headers)
add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES
	IMPORTED_LOCATION ${FFMPEG_ROOT_DIR}/lib/libavcodec${CMAKE_SHARED_LIBRARY_SUFFIX}
	INTERFACE_INCLUDE_DIRECTORIES ${FFMPEG_INCLUDE_DIR}
)

add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES
	IMPORTED_LOCATION ${FFMPEG_ROOT_DIR}/lib/libavformat${CMAKE_SHARED_LIBRARY_SUFFIX}
	INTERFACE_INCLUDE_DIRECTORIES ${FFMPEG_INCLUDE_DIR}
)

add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES
	IMPORTED_LOCATION ${FFMPEG_ROOT_DIR}/lib/libavutil${CMAKE_SHARED_LIBRARY_SUFFIX}
	INTERFACE_INCLUDE_DIRECTORIES ${FFMPEG_INCLUDE_DIR}
)

if(WIN32)
	set(LIB_PATH "${FFMPEG_ROOT_DIR}/lib")
	set_target_properties(avcodec PROPERTIES
		IMPORTED_IMPLIB ${LIB_PATH}/${CMAKE_STATIC_LIBRARY_PREFIX}avcodec.lib
	)
	set_target_properties(avformat PROPERTIES
		IMPORTED_IMPLIB ${LIB_PATH}/${CMAKE_STATIC_LIBRARY_PREFIX}avformat.lib
	)
	set_target_properties(avutil PROPERTIES
		IMPORTED_IMPLIB ${LIB_PATH}/${CMAKE_STATIC_LIBRARY_PREFIX}avutil.lib
	)
endif(WIN32)

add_library(ffmpeg INTERFACE)
target_link_libraries(ffmpeg INTERFACE avcodec avformat avutil)
