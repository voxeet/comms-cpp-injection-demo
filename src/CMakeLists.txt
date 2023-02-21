add_executable(cpp_injection_demo 
	main.cc
	utils/commands_handler.h
	utils/commands_handler.cc
	utils/interactor.h
	wrappers/command_line_params.h
	wrappers/command_line_params.cc
	wrappers/mediaio.h
	wrappers/mediaio.cc
	wrappers/sdk.h
	wrappers/sdk.cc
)

if(LINUX)
	target_sources(cpp_injection_demo PRIVATE
		linux/daemonize.h
		linux/daemonize.cc
	)
endif(LINUX)

target_compile_definitions(cpp_injection_demo PUBLIC
	DOLBYIO_COMMS_ENABLE_DEPRECATED_WAIT
)

target_include_directories(cpp_injection_demo PUBLIC
	${DOLBYIO_SDK_HEADERS}
	${DOLBYIO_SHARE_DIR}
	${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(cpp_injection_demo
	DolbyioComms::sdk
	media_source_file
)

copy_runtime_deps_dlls(cpp_injection_demo)
