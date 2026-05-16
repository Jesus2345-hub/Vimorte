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
// Incluye particulas, flash de pantalla, temblor de camara y texto BOOM
class EfectoExplosion {
private:
    std::vector<ParticulaExplosion> particulas;
    sf::Text textoBoom;
    sf::Font fuente;  // CORREGIDO: sf::Fuente -> sf::Font
    sf::Clock reloj;
    
    float duracionTotal;       // Cuanto dura todo el efecto
    float tiempoTranscurrido;
    bool activo;
    
    sf::Vector2f posicion;
    float intensidad;          // Que tan fuerte es la explosion (0.0 a 1.0)
    
    // Efecto de flash naranja en toda la pantalla
    sf::RectangleShape capaFlash;
    float alphaFlash;
    
    // Efecto de temblor de camara
    float intensidadTemblor;
    sf::Vector2f centroOriginalCamara;
    
    // Sonido de explosion (opcional, se carga si existe el archivo)
    sf::SoundBuffer bufferExplosion;
    sf::Sound sonidoExplosion;
    
    // Generador de numeros aleatorios para las particulas
    std::mt19937 generadorAleatorio;
    
public:
    EfectoExplosion();
    
    // Inicia la explosion en una posicion con una intensidad dada
    void iniciar(const sf::Vector2f& posicion, float intensidad = 1.0f);
    
    // Actualiza la animacion de la explosion
    void actualizar(float dt);
    
    // Dibuja las particulas en el mundo (usar con la vista de la camara)
    void dibujar(sf::RenderWindow& ventana);
    
    // Dibuja los efectos de UI como el flash y el texto BOOM (usar con vista por defecto)
    void dibujarUI(sf::RenderWindow& ventana);
    
    bool estaActivo() const { return activo; }
    bool haTerminado() const { return !activo && tiempoTranscurrido >= duracionTotal; }
    
    // Devuelve un desplazamiento aleatorio para simular el temblor de camara
    sf::Vector2f obtenerDesplazamientoTemblor();
    
    float obtenerAlphaFlash() const { return alphaFlash; }
};