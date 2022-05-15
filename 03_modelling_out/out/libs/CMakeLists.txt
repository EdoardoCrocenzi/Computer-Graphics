add_subdirectory(yocto)
add_subdirectory(yocto_model)

if(YOCTO_OPENGL)
add_subdirectory(yocto_gui)
endif(YOCTO_OPENGL)
