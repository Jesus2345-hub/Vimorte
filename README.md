# 🎮 VIMORTE

![CMake](https://img.shields.io/badge/CMake-3.16+-blue)
![SFML](https://img.shields.io/badge/SFML-3.0.2-green)
![C++](https://img.shields.io/badge/C++-17-purple)

## 📝 DESCRIPCIÓN
Vimorte es un juego desarrollado en C++ con SFML 3.0.2.  
*[Aquí pondremos la descripción próximamente]*

## ✨ CARACTERÍSTICAS (PRÓXIMAMENTE)
- [ ] Movimiento del jugador
- [ ] Sistema de colisiones
- [ ] Niveles
- [ ] Música y efectos

---

## 🛠️ REQUISITOS DEL SISTEMA

### Windows (MSYS2)
- **MSYS2** con UCRT64
- **CMake** 3.16+
- **Compilador:** GCC (viene con MSYS2)
- **C++17**

### Linux/Mac
- **CMake** 3.16+
- **Compilador:** GCC o Clang
- **C++17**

---

## 📦 INSTALACIÓN Y COMPILACIÓN

### 🔹 PRIMERA VEZ (Windows con MSYS2)

1. **Instalar MSYS2** desde https://www.msys2.org/
2. Abrir **MSYS2 UCRT64** (terminal verde)
3. Instalar herramientas necesarias:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-cmake
   pacman -S mingw-w64-ucrt-x86_64-toolchain
   pacman -S make