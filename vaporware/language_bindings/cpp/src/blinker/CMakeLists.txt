
add_executable(blinker
	main.cpp
)

target_link_libraries(blinker
	vaporpp
	vputils
	boost_program_options
)
