#include "Entidades/Gallo.hpp"
#include <iostream>
#include <cmath>

Gallo::Gallo()
    : m_estado(Estado::CAMINANDO),
      m_position(0.f, 0.f),
      m_speed(80.f),
      m_mirandoDerecha(true),
      m_frameActual(0),
      m_tiempoFrame(0.f),
      m_duracionFrame(0.15f),
      m_limiteIzquierdo(0.f),
      m_limiteDerecho(200.f),
      m_tiempoComer(0.f),
      m_tiempoEntreComidas(3.f),
      m_duracionComida(1.5f)
{
    m_escala = sf::Vector2f(0.4f, 0.4f);
    // Cargar idle
    if (!m_idleTexture.loadFromFile("assets/images/niveles/nivel6/gallo/idle.png"))
    {
        std::cerr << "❌ Error cargando idle del gallo" << std::endl;
    }

    // Cargar walk (4 frames)
    for (int i = 0; i < 4; i++)
    {
        sf::Texture tex;
        std::string path = "assets/images/niveles/nivel6/gallo/walk_" + std::to_string(i) + ".png";
        if (tex.loadFromFile(path))
        {
            m_walkTextures.push_back(tex);
        }
        else
        {
            std::cerr << "❌ Error cargando: " << path << std::endl;
        }
    }

    // Cargar eat (2 frames)
    for (int i = 0; i < 2; i++)
    {
        sf::Texture tex;
        std::string path = "assets/images/niveles/nivel6/gallo/eat_" + std::to_string(i) + ".png";
        if (tex.loadFromFile(path))
        {
            m_eatTextures.push_back(tex);
        }
        else
        {
            std::cerr << "❌ Error cargando: " << path << std::endl;
        }
    }

    // Crear sprite con idle
    m_sprite = std::make_unique<sf::Sprite>(m_idleTexture);

    // Escalar el gallo (ajusta este valor)
    m_sprite->setScale(m_escala); // ← AÑADIR (0.1f = más pequeño, 0.2f = más grande)

    // Centrar origen
    sf::FloatRect bounds = m_sprite->getLocalBounds();
    m_sprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));

    m_tiempoComer = m_tiempoEntreComidas;

    std::cout << "🐔 Gallo creado correctamente" << std::endl;

    // Iniciar con textura de caminar
    if (!m_walkTextures.empty())
    {
        m_sprite->setTexture(m_walkTextures[0]);
    }
}

void Gallo::setLimites(float izquierda, float derecha)
{
    m_limiteIzquierdo = izquierda;
    m_limiteDerecho = derecha;
}

void Gallo::setPosition(float x, float y)
{
    m_position = sf::Vector2f(x, y);
    m_sprite->setPosition(m_position);
}

void Gallo::cambiarEstado(Estado nuevo)
{
    if (m_estado == nuevo)
        return;

    m_estado = nuevo;
    m_frameActual = 0;
    m_tiempoFrame = 0.f;

    switch (m_estado)
    {
    case Estado::IDLE:
    case Estado::BLOQUEADO: // ← Usar misma textura que IDLE
        m_sprite->setTexture(m_idleTexture);
        break;
    case Estado::CAMINANDO:
        if (!m_walkTextures.empty())
            m_sprite->setTexture(m_walkTextures[0]);
        break;
    case Estado::COMIENDO:
        if (!m_eatTextures.empty())
            m_sprite->setTexture(m_eatTextures[0]);
        break;
    }

    m_sprite->setScale(m_escala);
    float flipX = m_mirandoDerecha ? m_escala.x : -m_escala.x;
    m_sprite->setScale(sf::Vector2f(flipX, m_escala.y));
}

void Gallo::actualizarAnimacion(float dt)
{
    m_tiempoFrame += dt;

    if (m_tiempoFrame >= m_duracionFrame)
    {
        m_tiempoFrame = 0.f;

        switch (m_estado)
        {
        case Estado::CAMINANDO:
            if (!m_walkTextures.empty())
            {
                m_frameActual = (m_frameActual + 1) % m_walkTextures.size();
                m_sprite->setTexture(m_walkTextures[m_frameActual]);
            }
            break;
        case Estado::COMIENDO:
            if (!m_eatTextures.empty())
            {
                m_frameActual = (m_frameActual + 1) % m_eatTextures.size();
                m_sprite->setTexture(m_eatTextures[m_frameActual]);
            }
            break;
        default:
            break;
        }

        // REAPLICAR ESCALA después de cambiar textura
        float flipX = m_mirandoDerecha ? 1.f : -1.f;
        m_sprite->setScale(sf::Vector2f(flipX * m_escala.x, m_escala.y));
    }
}

void Gallo::update(float dt)
{
    switch (m_estado)
    {
    case Estado::CAMINANDO:
    {
        // Moverse horizontalmente
        float direccion = m_mirandoDerecha ? 1.f : -1.f;
        m_position.x += direccion * m_speed * dt;

        // Rebote en límites
        if (m_position.x >= m_limiteDerecho)
        {
            m_position.x = m_limiteDerecho;
            m_mirandoDerecha = false;
        }
        else if (m_position.x <= m_limiteIzquierdo)
        {
            m_position.x = m_limiteIzquierdo;
            m_mirandoDerecha = true;
        }

        // Flip horizontal según dirección
        m_sprite->setScale(sf::Vector2f(m_mirandoDerecha ? m_escala.x : -m_escala.x, m_escala.y));

        // Actualizar animación
        actualizarAnimacion(dt);

        // Temporizador para comer
        m_tiempoComer -= dt;
        if (m_tiempoComer <= 0.f)
        {
            cambiarEstado(Estado::COMIENDO);
            m_tiempoComer = m_tiempoEntreComidas + m_duracionComida;
        }
        break;
    }

    case Estado::COMIENDO:
    {
        actualizarAnimacion(dt);
        m_tiempoComer -= dt;
        if (m_tiempoComer <= m_tiempoEntreComidas)
        {
            cambiarEstado(Estado::CAMINANDO);
        }
        break;
    }

    case Estado::BLOQUEADO:
    {
        // Simplemente no hacer nada, esperar a que el jugador se vaya
        m_sprite->setTexture(m_idleTexture);
        break;
    }

    case Estado::IDLE:
    default:
        break;
    }

    m_sprite->setPosition(m_position);
}

void Gallo::draw(sf::RenderWindow &window)
{
    if (m_sprite)
    {
        window.draw(*m_sprite);
    }
}

sf::FloatRect Gallo::getBounds() const
{
    if (m_sprite)
    {
        return m_sprite->getGlobalBounds();
    }
    return sf::FloatRect();
}

sf::Vector2f Gallo::getPosition() const
{
    return m_position;
}

void Gallo::verificarColisionJugador(const sf::FloatRect& playerBounds) {
    if (!m_sprite) return;
    
    sf::FloatRect galloBounds = m_sprite->getGlobalBounds();
    
    if (galloBounds.findIntersection(playerBounds).has_value()) {
        if (m_estado == Estado::CAMINANDO) {
            cambiarEstado(Estado::BLOQUEADO);
        }
    } else {
        if (m_estado == Estado::BLOQUEADO) {
            cambiarEstado(Estado::CAMINANDO);
        }
    }
}
