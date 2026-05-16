# CMakeLists.txt - Vimorte (Windows, Linux, Mac)
# ==================================================

cmake_minimum_required(VERSION 3.16)
project(Vimorte VERSION 1.0.0)

# Configuración de C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# DEPENDENCIAS DE LINUX (X11)
# ============================================================
if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
endif()

# ============================================================
# SFML 3.0.2 - DESCARGA AUTOMÁTICA
# ============================================================
include(FetchContent)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW TRUE
)

# Desactivar lo que no necesitamos para compilar más rápido
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SFML)

# ============================================================
# ARCHIVOS FUENTE
# ============================================================
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

# ============================================================
# CREAR EJECUTABLE
# ============================================================
add_executable(Vimorte ${SOURCES})

target_include_directories(Vimorte PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ============================================================
# ENLAZAR SFML
# ============================================================
target_link_libraries(Vimorte 
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)

# ============================================================
# WINDOWS: Copiar DLLs al ejecutable
# ============================================================
if(WIN32)
    add_custom_command(TARGET Vimorte POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:sfml-graphics>
            $<TARGET_FILE:sfml-window>
            $<TARGET_FILE:sfml-system>
            $<TARGET_FILE:sfml-audio>
            $<TARGET_FILE_DIR:Vimorte>
    )
endif()

# ============================================================
# COPIAR ASSETS AL DIRECTORIO DEL EJECUTABLE
# ============================================================
add_custom_command(TARGET Vimorte POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:Vimorte>/assets
    COMMENT "Copiando assets a la carpeta del ejecutable..."
)

message(STATUS "Proyecto Vimorte configurado correctamente")