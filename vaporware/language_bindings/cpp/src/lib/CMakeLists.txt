
add_library( vaporpp 
	client.cpp
	rgba_color.cpp
)

target_link_libraries( vaporpp 
	boost_system
	${CMAKE_THREAD_LIBS_INIT}
)
