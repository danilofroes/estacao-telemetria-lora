# CMakeLists principal do projeto
set(SRC_FILES "")

get_property(comp_defs DIRECTORY PROPERTY COMPILE_DEFINITIONS)

# Verificação do ambiente para definir a main
if("TRANSMISSOR" IN_LIST comp_defs)
    list(APPEND SRC_FILES "transmissor/main.cpp")
elseif("RECEPTOR" IN_LIST comp_defs)
    list(APPEND SRC_FILES "receptor/main.cpp")
else()
    list(APPEND SRC_FILES "transmissor/main.cpp") # fallback padrão
endif()

idf_component_register(SRCS ${SRC_FILES}
                       INCLUDE_DIRS ".")