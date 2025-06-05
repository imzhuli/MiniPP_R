cmake_minimum_required(VERSION 3.14)
project (pp_apps)

include_directories(
	../src_lib_common/
	../src_lib_protocol/
	../src_app/lib_backend_connection/
	../src_app/lib_server_util/
	${LOCAL_3RD_INCLUDE_DIR}
	${X_INCLUDE_DIRS}
)

link_directories(
	${LIBRARY_OUTPUT_PATH}
	${LOCAL_3RD_LIBRARY_DIR}
	${X_LIBRARY_DIRS}
)

message(STATUS ${LIBRARY_OUTPUT_PATH})
set(LOCAL_PP_LIBRARIES ppprotocol ppdb ppcommon)

# enable specific macors by system type:
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(PLATFORM_LIBRARIES ws2_32 Mswsock)
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(PLATFORM_LIBRARIES dl)
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
	find_library(COCOA_LIBRARY Cocoa REQUIRED)
	find_library(IOKIT_LIBRARY IOKit REQUIRED)
	find_library(FUNDATION_LIBRARY Foundation REQUIRED)
	find_library(SYSTEM_CONFIGURATION_LIBRARY SystemConfiguration REQUIRED)
	set(PLATFORM_LIBRARIES dl ${SYSTEM_CONFIGURATION_LIBRARY} ${FUNDATION_LIBRARY} ${IOKIT_LIBRARY} ${COCOA_LIBRARY} )
endif()


aux_source_directory("../src_app/lib_backend_connection"    LIB_SRC)
aux_source_directory("../src_app/lib_server_util"           LIB_SRC)

add_library(app_server ${LIB_SRC})
target_compile_definitions(app_server PRIVATE X_OPTION_STATIC)


set(lib_dependency
	app_server
	${LOCAL_PP_LIBRARIES}
	${LOCAL_3RD_LIBRARIES}
	${X_LIBRARIES}
	${PLATFORM_LIBRARIES}
	${cpp})

# applications
function(build_app sub_dir_name)
	aux_source_directory(${sub_dir_name} LOCAL_SOURCE_CODE)
	add_executable(_${sub_dir_name} ${LOCAL_SOURCE_CODE})
	target_compile_definitions(_${sub_dir_name} PRIVATE X_OPTION_STATIC)
	target_link_libraries(_${sub_dir_name} PRIVATE ${lib_dependency})
endfunction()

build_app(cpp)
build_app(ip2loc)
build_app(server_id_client)
