#include "EfectoExplosion.hpp"
#include <cmath>

// Constructor: inicializa el efecto en estado inactivo
// Los objetos sf::Text y sf::Sound requieren inicializacion especial
EfectoExplosion::EfectoExplosion() 
    : textoBoom(fuente, "", 30)  // Inicializar sf::Text con una fuente y tamano por defecto
    , sonidoExplosion(bufferExplosion)  // Inicializar sf::Sound con un buffer por defecto
    , activo(false)
    , tiempoTranscurrido(0.0f)
    , intensidad(1.0f)
    , alphaFlash(0.0f)
    , intensidadTemblor(0.0f)
{
    // Inicializar el generador de numeros aleatorios
    std::random_device dispositivoAleatorio;
    generadorAleatorio = std::mt19937(dispositivoAleatorio());
    
    // Cargar la fuente para el texto BOOM que aparece en pantalla
    if (fuente.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        textoBoom.setFont(fuente);
        textoBoom.setString("BOOM");
        textoBoom.setCharacterSize(120);
        textoBoom.setFillColor(sf::Color(255, 50, 0));
        textoBoom.setOutlineThickness(4.f);
        textoBoom.setOutlineColor(sf::Color::Yellow);
        textoBoom.setStyle(sf::Text::Bold);
    }
    
    // Configurar la capa de flash que cubre toda la pantalla
    capaFlash.setSize(sf::Vector2f(1920.f, 1080.f));
    capaFlash.setFillColor(sf::Color(255, 200, 50, 0));
    
    // Intentar cargar el sonido de explosion (no es obligatorio)
    if (bufferExplosion.loadFromFile("assets/sounds/explosion.wav")) {
        sonidoExplosion.setBuffer(bufferExplosion);
        sonidoExplosion.setVolume(80.f);
    }
}

// Inicia el efecto de explosion en la posicion especificada
// La intensidad va de 0.0 (debil) a 1.0 (muy fuerte)
void EfectoExplosion::iniciar(const sf::Vector2f& pos, float intensidadExplosion) {
    this->posicion = pos;
    this->intensidad = intensidadExplosion;
    this->activo = true;
    this->tiempoTranscurrido = 0.0f;
    this->duracionTotal = 2.5f;
    
    particulas.clear();
    
    // Calcular cuantas particulas crear segun la intensidad
    int cantidadParticulas = static_cast<int>(80 * intensidad);
    
    // Distribuciones aleatorias para las propiedades de las particulas
    std::uniform_real_distribution<float> distribucionAngulo(0.0f, 2.0f * 3.14159f);
    std::uniform_real_distribution<float> distribucionVelocidad(100.f, 400.f * intensidad);
    std::uniform_real_distribution<float> distribucionTamano(3.f, 15.f);
    std::uniform_real_distribution<float> distribucionTiempoVida(0.5f, 1.5f);
    
    // Colores que pueden tener las particulas de la explosion
    std::vector<sf::Color> coloresExplosion = {
        sf::Color(255, 200, 0),    // Amarillo fuego
        sf::Color(255, 100, 0),    // Naranja
        sf::Color(255, 50, 0),     // Rojo anaranjado
        sf::Color(255, 0, 0),      // Rojo intenso
        sf::Color(255, 255, 100),  // Amarillo claro
        sf::Color(200, 200, 200),  // Gris humo
    };
    std::uniform_int_distribution<int> distribucionColor(0, coloresExplosion.size() - 1);
    
    // Crear cada particula con propiedades aleatorias
    for (int i = 0; i < cantidadParticulas; ++i) {
        ParticulaExplosion p;
        float angulo = distribucionAngulo(generadorAleatorio);
        float velocidad = distribucionVelocidad(generadorAleatorio);
        
        p.forma.setRadius(distribucionTamano(generadorAleatorio));
        p.forma.setOrigin(sf::Vector2f(p.forma.getRadius(), p.forma.getRadius()));
        p.forma.setPosition(posicion);
        p.forma.setFillColor(coloresExplosion[distribucionColor(generadorAleatorio)]);
        
        p.velocidad = sf::Vector2f(std::cos(angulo) * velocidad, std::sin(angulo) * velocidad);
        p.tiempoVidaMaximo = distribucionTiempoVida(generadorAleatorio);
        p.tiempoVida = p.tiempoVidaMaximo;
        
        particulas.push_back(p);
    }
    
    // Configurar el flash inicial (mas intenso al principio)
    alphaFlash = 180.f * intensidad;
    
    // Configurar el temblor de camara
    intensidadTemblor = 15.f * intensidad;
    centroOriginalCamara = sf::Vector2f(640.f, 360.f);
    
    // Configurar el texto BOOM centrado en pantalla
    textoBoom.setCharacterSize(static_cast<unsigned int>(120 * intensidad));
    textoBoom.setOrigin(sf::Vector2f(
        textoBoom.getLocalBounds().size.x / 2.f,
        textoBoom.getLocalBounds().size.y / 2.f
    ));
    
    // Reproducir el sonido de explosion si esta disponible
    // Verificar si el buffer tiene datos (puntero no nulo)
    if (bufferExplosion.getSampleCount() > 0) {
        sonidoExplosion.play();
    }
    
    reloj.restart();
}

// Actualiza la animacion de la explosion cada frame
void EfectoExplosion::actualizar(float dt) {
    if (!activo) return;
    
    tiempoTranscurrido += dt;
    
    // Actualizar cada particula: mover, reducir velocidad, desvanecer
    for (auto& p : particulas) {
        p.tiempoVida -= dt;
        p.forma.move(p.velocidad * dt);
        
        // Aplicar friccion para que las particulas se frenen
        p.velocidad *= 0.98f;
        
        // Calcular la transparencia segun el tiempo de vida restante
        float proporcion = p.tiempoVida / p.tiempoVidaMaximo;
        sf::Color color = p.forma.getFillColor();
        color.a = static_cast<uint8_t>(255 * proporcion);
        p.forma.setFillColor(color);
        
        // Reducir el tamano de la particula conforme se desvanece
        float escala = proporcion;
        p.forma.setScale(sf::Vector2f(escala, escala));
    }
    
    // Eliminar las particulas que ya cumplieron su tiempo de vida
    particulas.erase(
        std::remove_if(particulas.begin(), particulas.end(),
                       [](const ParticulaExplosion& p) { return p.tiempoVida <= 0.0f; })
    );
    
    // Reducir gradualmente el flash naranja de la pantalla
    alphaFlash = std::max(0.0f, alphaFlash - dt * 200.f);
    capaFlash.setFillColor(sf::Color(255, 200, 50, static_cast<uint8_t>(alphaFlash)));
    
    // Reducir gradualmente la intensidad del temblor
    intensidadTemblor = std::max(0.0f, intensidadTemblor - dt * 20.f);
    
    // Desactivar cuando termine la duracion total
    if (tiempoTranscurrido >= duracionTotal) {
        activo = false;
    }
}

// Dibuja las particulas en las coordenadas del mundo del juego
void EfectoExplosion::dibujar(sf::RenderWindow& ventana) {
    if (!activo && particulas.empty()) return;
    
    for (const auto& p : particulas) {
        ventana.draw(p.forma);
    }
}

// Dibuja los efectos de interfaz: flash naranja y texto BOOM
void EfectoExplosion::dibujarUI(sf::RenderWindow& ventana) {
    if (!activo) return;
    
    // Dibujar el flash naranja sobre toda la pantalla
    if (alphaFlash > 5.0f) {
        ventana.draw(capaFlash);
    }
    
    // Dibujar el texto BOOM durante el primer segundo y medio
    if (tiempoTranscurrido < 1.5f) {
        float alpha = 1.0f;
        if (tiempoTranscurrido > 0.8f) {
            alpha = 1.0f - ((tiempoTranscurrido - 0.8f) / 0.7f);
        }
        
        // El texto crece con el tiempo
        float escala = 1.0f + tiempoTranscurrido * 2.0f;
        textoBoom.setScale(sf::Vector2f(escala, escala));
        
        // Aplicar transparencia al texto
        sf::Color colorTexto = textoBoom.getFillColor();
        colorTexto.a = static_cast<uint8_t>(255 * alpha);
        textoBoom.setFillColor(colorTexto);
        
        // Posicion centrada con un pequeno movimiento vibratorio
        textoBoom.setPosition(sf::Vector2f(
            ventana.getSize().x / 2.f + std::sin(tiempoTranscurrido * 20.f) * 5.f,
            ventana.getSize().y / 2.f + std::cos(tiempoTranscurrido * 15.f) * 5.f
        ));
        
        ventana.draw(textoBoom);
    }
}

// Devuelve un desplazamiento aleatorio para simular el temblor de camara
// CORREGIDO: se quito el 'const' para poder usar el generador aleatorio
sf::Vector2f EfectoExplosion::obtenerDesplazamientoTemblor() {
    if (intensidadTemblor <= 0.0f) return sf::Vector2f(0.f, 0.f);
    
    std::uniform_real_distribution<float> distribucionTemblor(-intensidadTemblor, intensidadTemblor);
    return sf::Vector2f(distribucionTemblor(generadorAleatorio), distribucionTemblor(generadorAleatorio));
}