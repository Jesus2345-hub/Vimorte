#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

class Config {
private:
    // Miembros estáticos privados
    static float s_volGeneral;
    static float s_volMusica;
    static float s_volEfectos;
    static bool s_pantallaCompleta;
    static sf::Vector2u s_resolucionActual;
    
public:
    // Getters estáticos
    static float getVolGeneral() { return s_volGeneral; }
    static float getVolMusica() { return s_volMusica; }
    static float getVolEfectos() { return s_volEfectos; }
    static bool isPantallaCompleta() { return s_pantallaCompleta; }
    static sf::Vector2u getResolucion() { return s_resolucionActual; }
    
    // Setters estáticos
    static void setVolGeneral(float v) { 
        s_volGeneral = std::clamp(v, 0.f, 100.f); 
        guardar(); 
    }
    static void setVolMusica(float v) { 
        s_volMusica = std::clamp(v, 0.f, 100.f); 
        guardar(); 
    }
    static void setVolEfectos(float v) { 
        s_volEfectos = std::clamp(v, 0.f, 100.f); 
        guardar(); 
    }
    static void setPantallaCompleta(bool fullscreen) { 
        s_pantallaCompleta = fullscreen; 
        guardar();
    }
    
    // Métodos estáticos públicos
    static void cargar();
    static void guardar();
    static void alternarPantalla(sf::RenderWindow* window);
};

#endif