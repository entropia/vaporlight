add_subdirectory(lib)

if(BUILD_SHELL MATCHES ON)
	add_subdirectory(shell)
else()
	message("Won't build the shell")
endif()

if(BUILD_FADE MATCHES ON)
	add_subdirectory(fade)
else()
	message("Won't build the fade-program")
endif()
