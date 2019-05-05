set(COMPONENT_SRCS "mac_whitelist.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()

if(GCC_NOT_5_2_0)
    component_compile_options(-Wno-unused-const-variable)
endif()
