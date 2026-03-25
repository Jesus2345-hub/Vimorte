# CMakeLists.txt - Vimorte con Makefile para Windows
# ==================================================

# 1. VERSIÓN MÍNIMA DE CMAKE
cmake_minimum_required(VERSION 3.16)

# 2. NOMBRE DEL PROYECTO
project(Vimorte VERSION 1.0.0)

# 3. CONFIGURACIÓN DE C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 4. DECIRLE A CMAKE QUE USE MAKEFILES (esto es para Windows con MinGW)
#    Si no tienes MinGW, más abajo te explico cómo instalarlo
#    Por ahora, usaremos el generador de Makefiles

# 5. FETCHCONTENT: DESCARGA SFML 3.0.2 AUTOMÁTICAMENTE
include(FetchContent)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW TRUE  # Descarga solo la versión, no todo el historial
)

# Esto descarga y compila SFML automáticamente
FetchContent_MakeAvailable(SFML)

# 6. BUSCAR TODOS LOS ARCHIVOS FUENTE EN src/
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

# 7. CREAR EL EJECUTABLE
add_executable(Vimorte ${SOURCES})

# 8. ENLAZAR CON LAS LIBRERÍAS DE SFML
target_link_libraries(Vimorte 
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)

# 9. SI ESTAMOS EN WINDOWS, COPIAR LAS DLLs AL EJECUTABLE
if(WIN32)
    add_custom_command(TARGET Vimorte POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:sfml-graphics>
            $<TARGET_FILE:sfml-window>
            $<TARGET_FILE:sfml-system>
            $<TARGET_FILE:sfml-audio>
            $<TARGET_FILE_DIR:Vimorte>
    )
endif()

# 10. MENSAJE DE ÉXITO AL TERMINAR
message(STATUS "Proyecto Vimorte configurado correctamente")