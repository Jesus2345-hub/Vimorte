#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <random>
#include <algorithm>

// Estructura que representa una particula individual de la explosion
struct ParticulaExplosion {
    sf::CircleShape forma;
    sf::Vector2f velocidad;
    float tiempoVida;
    float tiempoVidaMaximo;
};

// Clase que gestiona el efecto visual de una explosion
class EfectoExplosion {
private:
    std::vector<ParticulaExplosion> particulas;
    sf::Text textoBoom;
    sf::Font fuente;
    sf::Clock reloj;
    
    float duracionTotal;
    float tiempoTranscurrido;
    bool activo;
    
    sf::Vector2f posicion;
    float intensidad;
    
    sf::RectangleShape capaFlash;
    float alphaFlash;
    
    float intensidadTemblor;
    sf::Vector2f centroOriginalCamara;
    
    sf::SoundBuffer bufferExplosion;
    sf::Sound sonidoExplosion;
    
    std::mt19937 generadorAleatorio;
    
public:
    EfectoExplosion();
    
    void iniciar(const sf::Vector2f& posicion, float intensidad = 1.0f);
    void actualizar(float dt);
    void dibujar(sf::RenderWindow& ventana);
    void dibujarUI(sf::RenderWindow& ventana);
    
    bool estaActivo() const { return activo; }
    bool haTerminado() const { return !activo && tiempoTranscurrido >= duracionTotal; }
    
    sf::Vector2f obtenerDesplazamientoTemblor();
    float obtenerAlphaFlash() const { return alphaFlash; }
};