# Windows specific stuff for compiling
if(WIN32)
	# allocator override, if the application uses some custom allocator, but even if that does 
	# not happen, and that API is unused, the default CMake configuration uses the debug CRT 
	# when building debug mode. Here we enforce always using the release CRT, otherwise debug 
	# builds of the sample code will cause difficult to debug memory corruptions.
	set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
endif(WIN32)

function(copy_runtime_deps_dlls __target)
	if(WIN32)
		set(DLL_PATH "${CMAKE_BINARY_DIR}")
		set(SDK_LOCATION ${DLL_PATH}/dolbyio_comms_sdk.dll)
		set(MEDIA_LOCATION ${DLL_PATH}/dolbyio_comms_media.dll)
		set(MEDIA_ADDON_LOCATION ${DLL_PATH}/dolbyio_comms_multimedia_streaming_addon.dll)
		set(AVCODEC_LOCATION ${DLL_PATH}/avcodec-59.dll)
		set(AVUTIL_LOCATION ${DLL_PATH}/avutil-57.dll)
		set(AVFORMAT_LOCATION ${DLL_PATH}/avformat-59.dll)
		set(DVC_LOCATION ${DLL_PATH}/dvclient.dll)
		set(DNR_LOCATION ${DLL_PATH}/dvdnr.dll)
		set(DNR_MODEL_LOCATION ${DLL_PATH}/model.dnr)

	add_custom_command(TARGET ${__target} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${SDK_LOCATION} $<TARGET_FILE_DIR:${__target}>
		COMMAND ${CMAKE_COMMAND} -E copy ${MEDIA_LOCATION} $<TARGET_FILE_DIR:${__target}>
		COMMAND ${CMAKE_COMMAND} -E copy ${MEDIA_ADDON_LOCATION} $<TARGET_FILE_DIR:${__target}>
		COMMAND ${CMAKE_COMMAND} -E copy ${AVCODEC_LOCATION} $<TARGET_FILE_DIR:${__target}>
		COMMAND ${CMAKE_COMMAND} -E copy ${AVUTIL_LOCATION} $<TARGET_FILE_DIR:${__target}>
		COMMAND ${CMAKE_COMMAND} -E copy ${AVFORMAT_LOCATION} $<TARGET_FILE_DIR:${__target}>
		COMMAND ${CMAKE_COMMAND} -E copy ${DVC_LOCATION} $<TARGET_FILE_DIR:${__target}>
		COMMAND ${CMAKE_COMMAND} -E copy ${DNR_LOCATION} $<TARGET_FILE_DIR:${__target}>
		COMMAND ${CMAKE_COMMAND} -E copy ${DNR_MODEL_LOCATION} $<TARGET_FILE_DIR:${__target}>
		COMMAND_EXPAND_LISTS
	)
endif(WIN32)
endfunction()
