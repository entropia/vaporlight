
add_executable(fade
	main.cpp
	color_calculation.cpp
)

target_link_libraries(fade
	vaporpp
	boost_program_options
)
