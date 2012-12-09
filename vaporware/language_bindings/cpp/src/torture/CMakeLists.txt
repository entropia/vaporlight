
add_executable(torturerer
	main.cpp
)

target_link_libraries(torturerer
	vaporpp
	vputils
	boost_program_options
)
