add_executable(shell
	main.cpp
	console.cpp
	ids.cpp
	commands.cpp
)

target_link_libraries(shell
	vaporpp
	readline
	boost_program_options
)
