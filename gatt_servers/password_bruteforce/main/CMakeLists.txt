set(COMPONENT_SRCS "password_bruteforce.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()

if(GCC_NOT_5_2_0)
    component_compile_options(-Wno-unused-const-variable)
endif()
