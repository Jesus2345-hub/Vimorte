#include "MinigameBaloncesto.hpp"
#include <iostream>
#include <cmath>

MinigameBaloncesto::MinigameBaloncesto()
    : m_isActive(false), m_gameWon(false), m_score(0), m_scoreToWin(5),
      m_attempts(0), m_maxAttempts(7), m_fase(Fase::INTENSIDAD),
      m_pelotaVolando(false), m_mostrandoEnceste(false), m_tiempoEnceste(0.f),
      m_faseEnceste(0),
      m_indicadorVel(400.f), m_indicadorPos(0.f), m_indicadorSubiendo(true),
      m_intensidadFijada(false), m_intensidad(0.f),
      m_crosshairSpeed(0.f), m_messageTimer(0.f)
{
    m_background.setFillColor(sf::Color(0, 0, 0, 200));
    m_background.setOutlineThickness(3.f);
    m_background.setOutlineColor(sf::Color(100, 100, 100));

    // Barra de intensidad
    m_barraFondo.setFillColor(sf::Color(50, 50, 50));
    m_barraFondo.setOutlineThickness(2.f);
    m_barraFondo.setOutlineColor(sf::Color::White);

    m_barraVerde.setFillColor(sf::Color::Green);

    m_barraIndicador.setFillColor(sf::Color::Yellow);

    // Crosshair
    m_crosshair.setRadius(15.f);
    m_crosshair.setFillColor(sf::Color(255, 0, 0, 150));
    m_crosshair.setOutlineThickness(2.f);
    m_crosshair.setOutlineColor(sf::Color::Red);
    m_crosshair.setOrigin(sf::Vector2f(15.f, 15.f));
}

void MinigameBaloncesto::setPosition(const sf::Vector2f &pos)
{
    m_position = pos;
    m_background.setPosition(pos);
}

void MinigameBaloncesto::setSize(const sf::Vector2f &size)
{
    m_size = size;
    m_background.setSize(size);

    float escalaRef = std::min(m_size.x / 900.f, m_size.y / 650.f);
    if (escalaRef < 0.5f)
        escalaRef = 0.5f;
    if (escalaRef > 1.5f)
        escalaRef = 1.5f;

    float centroX = m_position.x + m_size.x / 2.f;
    float centroY = m_position.y + m_size.y / 2.f;

    // TABLERO
    float tableroAncho = m_size.x * 0.50f;
    float escalaTablero = tableroAncho / 376.f;
    float tableroAlto = 249.f * escalaTablero;

    if (!m_bgSprite)
    {
        if (m_bgTexture.loadFromFile("assets/images/niveles/nivel7/baloncesto_bg.png"))
        {
            m_bgSprite = std::make_unique<sf::Sprite>(m_bgTexture);
        }
    }

    if (m_bgSprite)
    {
        m_bgSprite->setScale(sf::Vector2f(escalaTablero, escalaTablero));
        sf::FloatRect bounds = m_bgSprite->getLocalBounds();
        m_bgSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_bgSprite->setPosition(sf::Vector2f(centroX + m_size.x * 0.10f, centroY));
    }

    // ARO (abajo)
    float aroOffsetY = tableroAlto * 0.40f;
    m_rimCenter = sf::Vector2f(
        centroX + m_size.x * 0.10f,
        centroY + aroOffsetY);

    if (!m_aroSprite)
    {
        if (m_aroVacioTexture.loadFromFile("assets/images/niveles/nivel7/aro_vacio.png"))
        {
            m_aroSprite = std::make_unique<sf::Sprite>(m_aroVacioTexture);
            m_aroEncestandoTexture.loadFromFile("assets/images/niveles/nivel7/aro_encestando.png");
            m_aroEncestadoTexture.loadFromFile("assets/images/niveles/nivel7/aro_encestado.png");
        }
    }

    if (m_aroSprite)
    {
        float escalaAro = escalaTablero * 0.40f;
        m_aroSprite->setScale(sf::Vector2f(escalaAro, escalaAro));
        sf::FloatRect bounds = m_aroSprite->getLocalBounds();
        m_aroSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_aroSprite->setPosition(sf::Vector2f(centroX + m_size.x * 0.10f, centroY + aroOffsetY));
    }

    // CENTRO DE APUNTADO (arriba)
    m_aimCenter = sf::Vector2f(
        centroX + m_size.x * 0.10f,
        centroY + tableroAlto * 0.07f);

    // ZONA DE TIRO (en el centro de apuntado)
    float zonaAncho = tableroAncho * 0.35f;
    float zonaAlto = tableroAlto * 0.35f;

    m_zonaTiro = sf::FloatRect(
        sf::Vector2f(
            m_aimCenter.x - zonaAncho / 2.f,
            m_aimCenter.y - zonaAlto / 2.f),
        sf::Vector2f(zonaAncho, zonaAlto));

    // PELOTA
    if (!m_pelotaSprite)
    {
        if (m_pelotaTexture.loadFromFile("assets/images/niveles/nivel7/pelota_basket.png"))
        {
            m_pelotaSprite = std::make_unique<sf::Sprite>(m_pelotaTexture);
        }
    }

    if (m_pelotaSprite)
    {
        float escalaPelota = escalaRef * 0.40f;
        m_pelotaSprite->setScale(sf::Vector2f(escalaPelota, escalaPelota));
        sf::FloatRect bounds = m_pelotaSprite->getLocalBounds();
        m_pelotaSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_pelotaSprite->setPosition(sf::Vector2f(centroX + m_size.x * 0.10f, m_position.y + m_size.y * 0.78f));
    }

    // BARRA DE INTENSIDAD
    float barraAncho = 35.f * escalaRef;
    float barraAlto = 250.f * escalaRef;
    m_barraX = m_position.x + m_size.x * 0.05f;
    m_barraY = centroY - barraAlto / 2.f;
    m_barraAlto = barraAlto;

    m_barraFondo.setSize(sf::Vector2f(barraAncho, barraAlto));
    m_barraFondo.setPosition(sf::Vector2f(m_barraX, m_barraY));

    float zonaVerdeAlto = barraAlto * 0.20f;
    float zonaVerdeY = m_barraY + (barraAlto - zonaVerdeAlto) / 2.f;
    m_barraVerde.setSize(sf::Vector2f(barraAncho, zonaVerdeAlto));
    m_barraVerde.setPosition(sf::Vector2f(m_barraX, zonaVerdeY));

    m_barraIndicador.setSize(sf::Vector2f(barraAncho + 10.f, 5.f * escalaRef));
    m_escalaRefGuardada = escalaRef;

    // TEXTOS
    if (m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf"))
    {
        int tamTitulo = static_cast<int>(24 * escalaRef);
        if (tamTitulo < 16) tamTitulo = 16;
        m_titleText = std::make_unique<sf::Text>(m_font, "TIROS LIBRES", tamTitulo);
        m_titleText->setFillColor(sf::Color::Yellow);

        int tamInfo = static_cast<int>(18 * escalaRef);
        if (tamInfo < 12) tamInfo = 12;
        m_scoreText = std::make_unique<sf::Text>(m_font, "", tamInfo);
        m_scoreText->setFillColor(sf::Color::White);
        m_attemptsText = std::make_unique<sf::Text>(m_font, "", tamInfo);
        m_attemptsText->setFillColor(sf::Color(200, 200, 200));

        int tamClose = static_cast<int>(12 * escalaRef);
        if (tamClose < 9) tamClose = 9;
        m_closeText = std::make_unique<sf::Text>(m_font, "ESPACIO: Tiro | ESC: Salir", tamClose);
        m_closeText->setFillColor(sf::Color(150, 150, 150));

        m_messageText = std::make_unique<sf::Text>(m_font, "", static_cast<unsigned int>(20 * escalaRef));
        m_messageText->setStyle(sf::Text::Bold);
    }

    if (m_scoreText)
        m_scoreText->setString("Canastas: " + std::to_string(m_score) + "/" + std::to_string(m_scoreToWin));
    if (m_attemptsText)
        m_attemptsText->setString("Tiros: " + std::to_string(m_attempts) + "/" + std::to_string(m_maxAttempts));
}

void MinigameBaloncesto::activate()
{
    m_isActive = true;
    reset();
}

void MinigameBaloncesto::deactivate()
{
    m_isActive = false;
}

void MinigameBaloncesto::reset()
{
    m_score = 0;
    m_attempts = 0;
    m_gameWon = false;
    m_fase = Fase::INTENSIDAD;
    m_indicadorPos = 150.f;
    m_indicadorSubiendo = true;
    m_intensidadFijada = false;
    m_pelotaVolando = false;
    m_mostrandoEnceste = false;
    m_faseEnceste = 0;
    m_messageTimer = 0.f;
    m_cerrando = false;
    m_tiempoCierre = 0.f;
    m_gameLost = false;

    if (m_scoreText)
        m_scoreText->setString("Canastas: 0/" + std::to_string(m_scoreToWin));
    if (m_attemptsText)
        m_attemptsText->setString("Tiros: 0/" + std::to_string(m_maxAttempts));
    if (m_aroSprite)
        m_aroSprite->setTexture(m_aroVacioTexture);

    if (m_pelotaSprite)
    {
        float centroX = m_position.x + m_size.x / 2.f;
        m_pelotaSprite->setPosition(sf::Vector2f(centroX + m_size.x * 0.10f, m_position.y + m_size.y * 0.82f));
    }
}

void MinigameBaloncesto::handleEvent(const sf::Event &event, const sf::RenderWindow &window)
{
    if (!m_isActive || m_gameWon)
        return;

    if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->code == sf::Keyboard::Key::Space || keyPressed->code == sf::Keyboard::Key::Enter)
        {
            if (m_fase == Fase::INTENSIDAD && !m_intensidadFijada)
            {
                m_intensidadFijada = true;

                float centroIdeal = m_barraAlto / 2.f;
                float distancia = std::abs(m_indicadorPos - centroIdeal);
                float maxDistancia = m_barraAlto / 2.f;

                m_intensidad = 1.0f - (distancia / maxDistancia);
                if (m_intensidad < 0.1f)
                    m_intensidad = 0.1f;

                m_fase = Fase::APUNTAR;
                m_crosshairPos = m_aimCenter;
            }
            else if (m_fase == Fase::APUNTAR)
            {
                iniciarLanzamiento();
                m_fase = Fase::VOLANDO;
            }
        }
    }
}

void MinigameBaloncesto::iniciarLanzamiento()
{
    m_pelotaVolando = true;

    m_attempts++;

    if (m_attemptsText)
    {
        m_attemptsText->setString(
            "Tiros: " +
            std::to_string(m_attempts) +
            "/" +
            std::to_string(m_maxAttempts));
    }

    m_ballStart = m_pelotaSprite->getPosition();

    m_ballTarget = m_crosshairPos;

    // Error si intensidad mala
    if (m_intensidad < 0.5f)
    {
        float error = (0.5f - m_intensidad) * 120.f;

        m_ballTarget.x += ((rand() % 2) ? error : -error);
        m_ballTarget.y += ((rand() % 2) ? error * 0.5f : -error * 0.5f);
    }

    m_ballTime = 0.f;
    m_ballDuration = 0.85f;
}

void MinigameBaloncesto::verificarEnceste()
{
    sf::Vector2f pelotaPos = m_pelotaSprite->getPosition();

    if (m_zonaTiro.contains(pelotaPos) && !m_mostrandoEnceste)
    {
        if (m_intensidad > 0.35f)
        {
            m_score++;
            m_mostrandoEnceste = true;
            m_tiempoEnceste = 0.4f;
            m_faseEnceste = 1;
            m_pelotaVolando = false;

            if (m_aroSprite)
                m_aroSprite->setTexture(m_aroEncestandoTexture);

            if (m_scoreText)
                m_scoreText->setString("Canastas: " + std::to_string(m_score) + "/" + std::to_string(m_scoreToWin));

            if (m_score >= m_scoreToWin)
            {
                m_gameWon = true;
                if (m_titleText)
                {
                    m_titleText->setString("¡VICTORIA!");
                    m_titleText->setFillColor(sf::Color::Green);
                }
                m_cerrando = true;
                m_tiempoCierre = 1.5f;
            }
        }
    }
}

void MinigameBaloncesto::showMessage(const std::string &msg, const sf::Color &color)
{
    if (!m_messageText)
        return;
    m_messageText->setString(msg);
    m_messageText->setFillColor(color);
    sf::FloatRect bounds = m_messageText->getLocalBounds();
    m_messageText->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    m_messageText->setPosition(sf::Vector2f(
        m_position.x + m_size.x / 2.f,
        m_position.y + m_size.y - 30.f));
    m_messageTimer = 1.5f;
}

void MinigameBaloncesto::update(float dt)
{
    if (!m_isActive)
        return;

    // Cierre suave del minijuego
    if (m_cerrando)
    {
        m_tiempoCierre -= dt;
        if (m_tiempoCierre <= 0.f)
        {
            deactivate();
            m_cerrando = false;
        }
        return;
    }

    if (m_messageTimer > 0.f)
    {
        m_messageTimer -= dt;
        if (m_messageTimer <= 0.f && m_messageText)
            m_messageText->setString("");
    }

    // FASE INTENSIDAD
    if (m_fase == Fase::INTENSIDAD && !m_intensidadFijada)
    {
        if (m_indicadorSubiendo)
        {
            m_indicadorPos += m_indicadorVel * dt;
            if (m_indicadorPos >= m_barraAlto)
            {
                m_indicadorPos = m_barraAlto;
                m_indicadorSubiendo = false;
            }
        }
        else
        {
            m_indicadorPos -= m_indicadorVel * dt;
            if (m_indicadorPos <= 0.f)
            {
                m_indicadorPos = 0.f;
                m_indicadorSubiendo = true;
            }
        }
    }

    // FASE APUNTAR
    if (m_fase == Fase::APUNTAR)
    {
        static float tiempoCross = 0.f;
        tiempoCross += dt;

        float amplitudX = 200.f;
        float amplitudY = 120.f;

        m_crosshairPos.x = m_aimCenter.x + std::sin(tiempoCross * 2.0f) * amplitudX;
        m_crosshairPos.y = m_aimCenter.y + std::sin(tiempoCross * 4.0f) * amplitudY;
    }

    // FASE VOLANDO
    if (m_fase == Fase::VOLANDO && m_pelotaVolando)
    {
        m_ballTime += dt;

        float t = m_ballTime / m_ballDuration;
        if (t > 1.f) t = 1.f;

        sf::Vector2f pos;
        pos.x = m_ballStart.x + (m_ballTarget.x - m_ballStart.x) * t;
        pos.y = m_ballStart.y + (m_ballTarget.y - m_ballStart.y) * t;

        float arco = 260.f * std::sin(t * 3.141592f);
        pos.y -= arco;

        m_pelotaSprite->setPosition(pos);

        verificarEnceste();

        if (t >= 1.f && !m_mostrandoEnceste)
        {
            m_pelotaVolando = false;
            m_intensidadFijada = false;

            if (m_aroSprite)
                m_aroSprite->setTexture(m_aroVacioTexture);

            float centroX = m_position.x + m_size.x / 2.f;
            m_pelotaSprite->setPosition(sf::Vector2f(centroX + m_size.x * 0.10f, m_position.y + m_size.y * 0.82f));

            if (m_attempts >= m_maxAttempts && m_score < m_scoreToWin)
            {
                m_gameLost = true;
                if (m_titleText)
                {
                    m_titleText->setString("SIN TIROS");
                    m_titleText->setFillColor(sf::Color::Red);
                }
                m_cerrando = true;
                m_tiempoCierre = 1.5f;
                return;
            }

            m_fase = Fase::INTENSIDAD;
            m_indicadorPos = m_barraAlto / 2.f;
            m_indicadorSubiendo = true;
        }
    }

    // Animación de enceste
    if (m_mostrandoEnceste && m_faseEnceste > 0)
    {
        m_tiempoEnceste -= dt;

        if (m_faseEnceste == 1 && m_tiempoEnceste <= 0.f)
        {
            m_faseEnceste = 2;
            m_tiempoEnceste = 0.4f;
            if (m_aroSprite)
                m_aroSprite->setTexture(m_aroEncestadoTexture);
        }
        else if (m_faseEnceste == 2 && m_tiempoEnceste <= 0.f)
        {
            m_faseEnceste = 0;
            m_mostrandoEnceste = false;
            m_pelotaVolando = false;
            m_intensidadFijada = false;

            if (m_aroSprite)
                m_aroSprite->setTexture(m_aroVacioTexture);

            float centroX = m_position.x + m_size.x / 2.f;
            m_pelotaSprite->setPosition(sf::Vector2f(centroX + m_size.x * 0.10f, m_position.y + m_size.y * 0.82f));

            m_fase = Fase::INTENSIDAD;
            m_indicadorPos = m_barraAlto / 2.f;
            m_indicadorSubiendo = true;
        }
    }
}

void MinigameBaloncesto::draw(sf::RenderWindow &window)
{
    if (!m_isActive)
        return;

    // Fondo
    window.draw(m_background);

    // Tablero
    if (m_bgSprite)
        window.draw(*m_bgSprite);

    // Zona de tiro (rectángulo verde)
    sf::RectangleShape zonaDebug(sf::Vector2f(m_zonaTiro.size.x, m_zonaTiro.size.y));
    zonaDebug.setPosition(sf::Vector2f(m_zonaTiro.position.x, m_zonaTiro.position.y));
    zonaDebug.setFillColor(sf::Color(0, 255, 0, 80));
    zonaDebug.setOutlineThickness(2.f);
    zonaDebug.setOutlineColor(sf::Color::Green);
    window.draw(zonaDebug);

    // Barra de intensidad
    if (m_fase == Fase::INTENSIDAD && !m_intensidadFijada)
    {
        window.draw(m_barraFondo);
        window.draw(m_barraVerde);

        // Posicionar indicador correctamente
        m_barraIndicador.setPosition(sf::Vector2f(
            m_barraX - 5.f,
            m_barraY + m_indicadorPos));
        window.draw(m_barraIndicador);
    }

    // Aro
    if (m_aroSprite)
        window.draw(*m_aroSprite);

    // Pelota
    if (m_pelotaSprite && m_pelotaVolando)
        window.draw(*m_pelotaSprite);

    // Crosshair
    if (m_fase == Fase::APUNTAR && !m_pelotaVolando)
    {
        m_crosshair.setPosition(m_crosshairPos);
        window.draw(m_crosshair);
    }

    // Textos
    float escalaRef = m_escalaRefGuardada;
    float centroX = m_position.x + m_size.x / 2.f;

    if (m_titleText)
    {
        sf::FloatRect tb = m_titleText->getLocalBounds();
        m_titleText->setOrigin(sf::Vector2f(tb.size.x / 2.f, 0.f));
        m_titleText->setPosition(sf::Vector2f(centroX, m_position.y + 8.f * escalaRef));
        window.draw(*m_titleText);
    }

    if (m_scoreText)
    {
        m_scoreText->setPosition(sf::Vector2f(m_position.x + 10.f, m_position.y + 8.f * escalaRef));
        window.draw(*m_scoreText);
    }

    if (m_attemptsText)
    {
        sf::FloatRect ab = m_attemptsText->getLocalBounds();
        m_attemptsText->setOrigin(sf::Vector2f(ab.size.x, 0.f));
        m_attemptsText->setPosition(sf::Vector2f(m_position.x + m_size.x - 10.f, m_position.y + 8.f * escalaRef));
        window.draw(*m_attemptsText);
    }

    if (m_closeText)
    {
        sf::FloatRect cb = m_closeText->getLocalBounds();
        m_closeText->setOrigin(sf::Vector2f(cb.size.x / 2.f, 0.f));
        m_closeText->setPosition(sf::Vector2f(centroX, m_position.y + m_size.y - 22.f * escalaRef));
        window.draw(*m_closeText);
    }

    if (m_messageText && m_messageTimer > 0.f)
    {
        window.draw(*m_messageText);
    }
}