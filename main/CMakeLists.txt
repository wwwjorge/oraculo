set(SOURCES "main.cc")

set(INCLUDE_DIRS "." "audio" "protocols")

# Adicionar board personalizado
file(GLOB BOARD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/boards/oraculo-custom/*.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/boards/oraculo-custom/*.c
)
list(APPEND SOURCES ${BOARD_SOURCES})

# Configuração básica do projeto

# Registrar componente
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${INCLUDE_DIRS}
                    )

# Definir tipo de board
target_compile_definitions(${COMPONENT_LIB}
                    PRIVATE BOARD_TYPE="oraculo-custom" BOARD_NAME="Oraculo_Assistant"
                    )