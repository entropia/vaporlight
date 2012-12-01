add_executable(shell
	main.cpp
	console.cpp
	commands.cpp
)

target_link_libraries(shell
	vaporpp
	vputils
	readline
	boost_program_options
)
