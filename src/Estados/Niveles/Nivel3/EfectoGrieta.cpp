#include "Estados/Niveles/Nivel3/EfectoGrieta.hpp"
#include <cmath>
#include <random>

// Constructor: inicializa la grieta en estado inactivo
EfectoGrieta::EfectoGrieta()
    : activo(false)
    , progresoAnimacion(0.0f)
    , bordeAgujero(sf::PrimitiveType::TriangleFan, 20) 
{
    // El agujero base es completamente negro
    agujeroOscuro.setFillColor(sf::Color(0, 0, 0));
}

// Inicializa el efecto de grieta en una posicion y tamano especificos
void EfectoGrieta::iniciar(const sf::Vector2f& posicionPared, const sf::Vector2f& tamanoPared) {
    this->posicion = posicionPared;
    this->tamano = tamanoPared;
    this->activo = true;
    this->progresoAnimacion = 0.0f;
    
    lineasGrieta.clear();
    escombros.clear();
    
    std::random_device dispositivoAleatorio;
    std::mt19937 generador(dispositivoAleatorio());
    std::uniform_real_distribution<float> distribucionTemblor(-5.f, 5.f);
    
    // Calcular el centro de la grieta
    sf::Vector2f centro(posicion.x + tamano.x / 2.f, posicion.y + tamano.y / 2.f);
    int cantidadGrietas = 12;
    
    // Crear lineas de fractura que salen desde el centro hacia afuera
    for (int i = 0; i < cantidadGrietas; ++i) {
        sf::VertexArray linea(sf::PrimitiveType::LineStrip, 4);
        
        float angulo = (i * 2.0f * 3.14159f) / cantidadGrietas;
        float longitud = std::min(tamano.x, tamano.y) * 0.8f;
        
        // Cada linea tiene 4 puntos para que sea irregular
        for (int j = 0; j < 4; ++j) {
            float t = j / 3.0f;
            float longitudActual = longitud * t;
            sf::Vector2f punto(
                centro.x + std::cos(angulo) * longitudActual + distribucionTemblor(generador),
                centro.y + std::sin(angulo) * longitudActual + distribucionTemblor(generador)
            );
            
            linea[j].position = punto;
            linea[j].color = sf::Color(20, 20, 20);
        }
        
        lineasGrieta.push_back(linea);
    }
    
    // Crear un borde irregular para el agujero (no sera un rectangulo perfecto)
    bordeAgujero = sf::VertexArray(sf::PrimitiveType::TriangleFan, 20);
    bordeAgujero[0].position = centro;
    bordeAgujero[0].color = sf::Color::Black;
    
    // Generar vertices alrededor del centro con radios aleatorios
    for (int i = 1; i < 20; ++i) {
        float angulo = (i * 2.0f * 3.14159f) / 19;
        float radio = std::min(tamano.x, tamano.y) * 0.3f + distribucionTemblor(generador);
        
        bordeAgujero[i].position = sf::Vector2f(
            centro.x + std::cos(angulo) * radio,
            centro.y + std::sin(angulo) * radio
        );
        bordeAgujero[i].color = sf::Color::Black;
    }
    
    // Crear pequenos escombros alrededor del agujero
    for (int i = 0; i < 20; ++i) {
        sf::CircleShape roca(3.f + (rand() % 5));
        roca.setFillColor(sf::Color(80, 80, 80));
        roca.setPosition(sf::Vector2f(
            centro.x + (rand() % (int)tamano.x) - tamano.x / 2.f,
            centro.y + (rand() % (int)tamano.y) - tamano.y / 2.f
        ));
        escombros.push_back(roca);
    }
    
    // Configurar el rectangulo negro base
    agujeroOscuro.setSize(tamano);
    agujeroOscuro.setPosition(posicion);
}

// Actualiza la animacion de la grieta (aparece gradualmente)
void EfectoGrieta::actualizar(float dt) {
    if (!activo) return;
    
    // Aumentar el progreso hasta llegar a 1.0 (completamente visible)
    progresoAnimacion = std::min(1.0f, progresoAnimacion + dt * 0.5f);
}

// Dibuja todos los elementos de la grieta en orden de aparicion
void EfectoGrieta::dibujar(sf::RenderWindow& ventana) {
    if (!activo) return;
    
    // 1. Dibujar el agujero negro base (aparece primero)
    sf::Color colorAgujero = agujeroOscuro.getFillColor();
    colorAgujero.a = static_cast<uint8_t>(200 * progresoAnimacion);
    agujeroOscuro.setFillColor(colorAgujero);
    ventana.draw(agujeroOscuro);
    
    // 2. Dibujar el borde irregular (aparece al 30% del progreso)
    if (progresoAnimacion > 0.3f) {
        sf::Color colorBorde = bordeAgujero[0].color;
        uint8_t alpha = static_cast<uint8_t>(255 * std::min(1.0f, (progresoAnimacion - 0.3f) / 0.7f));
        
        for (size_t i = 0; i < bordeAgujero.getVertexCount(); ++i) {
            sf::Color c = bordeAgujero[i].color;
            c.a = alpha;
            bordeAgujero[i].color = c;
        }
        
        ventana.draw(bordeAgujero);
    }
    
    // 3. Dibujar las lineas de fractura (aparecen al 50% del progreso)
    if (progresoAnimacion > 0.5f) {
        for (auto& linea : lineasGrieta) {
            for (size_t i = 0; i < linea.getVertexCount(); ++i) {
                sf::Color c = linea[i].color;
                c.a = 100;
                linea[i].color = c;
            }
            ventana.draw(linea);
        }
    }
    
    // 4. Dibujar los escombros (aparecen al 70% del progreso)
    if (progresoAnimacion > 0.7f) {
        for (auto& roca : escombros) {
            ventana.draw(roca);
        }
    }
}