project(SBE-framework)

file(GLOB PROJECT_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

#if(SBE_SINGLE_BINARY)
	set(LIB_TYPE STATIC)
#else()
	#set(LIB_TYPE MODULE)
#endif()

add_library(${PROJECT_NAME} ${LIB_TYPE} ${PROJECT_SOURCES})