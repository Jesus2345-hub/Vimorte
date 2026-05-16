#include "EfectoExplosion.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

// Constructor: inicializa el efecto en estado inactivo
EfectoExplosion::EfectoExplosion() 
    : textoBoom(fuente, "", 30)  // Inicializar con la fuente (aunque no este cargada aun)
    , sonidoExplosion(bufferExplosion)  // Inicializar con el buffer (aunque este vacio)
    , activo(false)
    , tiempoTranscurrido(0.0f)
    , intensidad(1.0f)
    , alphaFlash(0.0f)
    , intensidadTemblor(0.0f)
{
    // Inicializar el generador de numeros aleatorios
    std::random_device dispositivoAleatorio;
    generadorAleatorio = std::mt19937(dispositivoAleatorio());
    
    // Cargar la fuente (intentar, no es obligatorio)
    bool fuenteCargada = fuente.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    if (fuenteCargada) {
        textoBoom.setFont(fuente);
        textoBoom.setString("BOOM");
        textoBoom.setCharacterSize(120);
        textoBoom.setFillColor(sf::Color(255, 50, 0));
        textoBoom.setOutlineThickness(4.f);
        textoBoom.setOutlineColor(sf::Color::Yellow);
        textoBoom.setStyle(sf::Text::Bold);
    }
    
    // Configurar la capa de flash
    capaFlash.setSize(sf::Vector2f(1920.f, 1080.f));
    capaFlash.setFillColor(sf::Color(255, 200, 50, 0));
    
    // Intentar cargar el sonido de explosion (no es obligatorio)
    if (bufferExplosion.loadFromFile("assets/sounds/explosion.wav")) {
        sonidoExplosion.setBuffer(bufferExplosion);
        sonidoExplosion.setVolume(80.f);
    }
}

// Inicia el efecto de explosion en la posicion especificada
void EfectoExplosion::iniciar(const sf::Vector2f& pos, float intensidadExplosion) {
    // Verificar que la intensidad sea valida
    if (intensidadExplosion <= 0.0f) return;
    
    this->posicion = pos;
    this->intensidad = intensidadExplosion;
    this->activo = true;
    this->tiempoTranscurrido = 0.0f;
    this->duracionTotal = 2.5f;
    
    particulas.clear();
    
    // Calcular cuantas particulas crear
    int cantidadParticulas = static_cast<int>(80 * intensidad);
    if (cantidadParticulas < 1) cantidadParticulas = 1;
    if (cantidadParticulas > 200) cantidadParticulas = 200;
    
    // Crear cada particula
    for (int i = 0; i < cantidadParticulas; ++i) {
        ParticulaExplosion p;
        
        // Propiedades aleatorias
        float angulo = static_cast<float>(rand() % 628) / 100.0f;  // 0 a 2*PI
        float velocidad = 100.0f + static_cast<float>(rand() % 300) * intensidad;
        float radio = 3.0f + static_cast<float>(rand() % 12);
        
        p.forma.setRadius(radio);
        p.forma.setOrigin(sf::Vector2f(radio, radio));
        p.forma.setPosition(posicion);
        
        // Color aleatorio de explosion
        int colorIdx = rand() % 6;
        switch (colorIdx) {
            case 0: p.forma.setFillColor(sf::Color(255, 200, 0)); break;   // Amarillo
            case 1: p.forma.setFillColor(sf::Color(255, 100, 0)); break;   // Naranja
            case 2: p.forma.setFillColor(sf::Color(255, 50, 0)); break;    // Rojo anaranjado
            case 3: p.forma.setFillColor(sf::Color(255, 0, 0)); break;     // Rojo
            case 4: p.forma.setFillColor(sf::Color(255, 255, 100)); break; // Amarillo claro
            case 5: p.forma.setFillColor(sf::Color(200, 200, 200)); break; // Gris
        }
        
        p.velocidad = sf::Vector2f(std::cos(angulo) * velocidad, std::sin(angulo) * velocidad);
        p.tiempoVidaMaximo = 0.5f + static_cast<float>(rand() % 100) / 100.0f;
        p.tiempoVida = p.tiempoVidaMaximo;
        
        particulas.push_back(p);
    }
    
    // Flash inicial
    alphaFlash = 180.0f * intensidad;
    if (alphaFlash > 255.0f) alphaFlash = 255.0f;
    
       // Temblor de camara
    intensidadTemblor = 15.0f * intensidad;
    
    // Reproducir sonido de explosion si esta disponible
    if (bufferExplosion.getSampleCount() > 0) {
        sonidoExplosion.play();
    }
    
    reloj.restart();
    
    std::cout << "[EXPLOSION] Iniciada en (" << pos.x << ", " << pos.y << ") con " << cantidadParticulas << " particulas" << std::endl;
}

// Actualiza la animacion de la explosion cada frame
void EfectoExplosion::actualizar(float dt) {
    if (!activo) return;
    
    tiempoTranscurrido += dt;
    
    // Actualizar cada particula
    for (auto& p : particulas) {
        p.tiempoVida -= dt;
        p.forma.move(p.velocidad * dt);
        p.velocidad *= 0.98f;
        
        // Transparencia segun tiempo de vida
        if (p.tiempoVidaMaximo > 0.0f) {
            float proporcion = p.tiempoVida / p.tiempoVidaMaximo;
            if (proporcion < 0.0f) proporcion = 0.0f;
            
            sf::Color color = p.forma.getFillColor();
            color.a = static_cast<uint8_t>(255.0f * proporcion);
            p.forma.setFillColor(color);
            
            p.forma.setScale(sf::Vector2f(proporcion, proporcion));
        }
    }
    
    // Eliminar particulas muertas
    particulas.erase(
        std::remove_if(particulas.begin(), particulas.end(),
            [](const ParticulaExplosion& p) { return p.tiempoVida <= 0.0f; }),
        particulas.end()
    );
    
    // Reducir flash
    alphaFlash = std::max(0.0f, alphaFlash - dt * 200.0f);
    capaFlash.setFillColor(sf::Color(255, 200, 50, static_cast<uint8_t>(alphaFlash)));
    
    // Reducir temblor
    intensidadTemblor = std::max(0.0f, intensidadTemblor - dt * 20.0f);
    
    // Desactivar cuando termine
    if (tiempoTranscurrido >= duracionTotal) {
        activo = false;
    }
}

// Dibuja las particulas en el mundo
void EfectoExplosion::dibujar(sf::RenderWindow& ventana) {
    if (!activo && particulas.empty()) return;
    
    for (const auto& p : particulas) {
        ventana.draw(p.forma);
    }
}

// Dibuja los efectos de UI
void EfectoExplosion::dibujarUI(sf::RenderWindow& ventana) {
    if (!activo) return;
    
    // Flash naranja
    if (alphaFlash > 5.0f) {
        ventana.draw(capaFlash);
    }
    
    // Texto BOOM (solo si la fuente se cargo)
    if (tiempoTranscurrido < 1.5f && fuente.getInfo().family != "") {
        float alpha = 1.0f;
        if (tiempoTranscurrido > 0.8f) {
            alpha = 1.0f - ((tiempoTranscurrido - 0.8f) / 0.7f);
        }
        
        float escala = 1.0f + tiempoTranscurrido * 2.0f;
        textoBoom.setScale(sf::Vector2f(escala, escala));
        
        sf::Color colorTexto = textoBoom.getFillColor();
        colorTexto.a = static_cast<uint8_t>(255.0f * alpha);
        textoBoom.setFillColor(colorTexto);
        
        textoBoom.setPosition(sf::Vector2f(
            ventana.getSize().x / 2.0f,
            ventana.getSize().y / 2.0f
        ));
        
        ventana.draw(textoBoom);
    }
}

// Devuelve desplazamiento para temblor de camara
sf::Vector2f EfectoExplosion::obtenerDesplazamientoTemblor() {
    if (intensidadTemblor <= 0.0f) return sf::Vector2f(0.f, 0.f);
    
    float dx = static_cast<float>(rand() % 100) / 100.0f * intensidadTemblor * 2.0f - intensidadTemblor;
    float dy = static_cast<float>(rand() % 100) / 100.0f * intensidadTemblor * 2.0f - intensidadTemblor;
    
    return sf::Vector2f(dx, dy);
}