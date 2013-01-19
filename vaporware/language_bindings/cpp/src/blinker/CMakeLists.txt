
add_executable(blinker
	main.cpp
)

target_link_libraries(blinker
	vaporpp
	vputils
	pthread
	boost_program_options
)
