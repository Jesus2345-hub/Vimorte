#include "MinigameRoosterHunt.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

MinigameRoosterHunt::MinigameRoosterHunt()
    : m_isActive(false), m_gameWon(false), m_score(0), m_scoreToWin(5),
      m_ammo(10), m_gen(m_rd()), m_spawnTimer(0.f), m_spawnInterval(1.5f),
      m_messageTimer(0.f), m_roosterScale(0.3f), m_roosterSpeedMin(300.f), m_roosterSpeedMax(500.f)
{
    m_background.setFillColor(sf::Color(0, 0, 0, 200));
    m_background.setOutlineThickness(3.f);
    m_background.setOutlineColor(sf::Color(100, 100, 100));

    if (!m_deadTexture.loadFromFile("assets/images/niveles/nivel6/gallo/dead.png"))
    {
        std::cerr << "Error cargando dead.png" << std::endl;
    }

    // Cargar texturas de vuelo
    for (int i = 0; i < 4; i++)
    {
        sf::Texture tex;
        std::string path = "assets/images/niveles/nivel6/gallo/fly_" + std::to_string(i) + ".png";
        if (tex.loadFromFile(path))
        {
            m_flyTextures.push_back(tex);
        }
        else
        {
            std::cerr << "Error cargando: " << path << std::endl;
        }
    }
    // Cargar crosshair
    if (m_crosshairTexture.loadFromFile("assets/images/niveles/nivel6/crosshair.png"))
    {
        m_crosshair = std::make_unique<sf::Sprite>(m_crosshairTexture);
        m_crosshair->setScale(sf::Vector2f(0.35f, 0.35f));
        sf::FloatRect cb = m_crosshair->getLocalBounds();
        m_crosshair->setOrigin(sf::Vector2f(cb.size.x / 2.f, cb.size.y / 2.f));
        std::cout << "Crosshair cargado. Tamaño: " << cb.size.x << "x" << cb.size.y << std::endl;
    }
    else
    {
        std::cerr << "Error cargando crosshair.png" << std::endl;
    }

    // Cargar iconos de gallos
    if (!m_aliveIconTexture.loadFromFile("assets/images/niveles/nivel6/rooster_alive.png"))
    {
        std::cerr << "Error cargando rooster_alive.png" << std::endl;
    }
    if (!m_deadIconTexture.loadFromFile("assets/images/niveles/nivel6/rooster_dead.png"))
    {
        std::cerr << "Error cargando rooster_dead.png" << std::endl;
    }
}

void MinigameRoosterHunt::setPosition(const sf::Vector2f &pos)
{
    m_position = pos;
    m_background.setPosition(pos);
}

void MinigameRoosterHunt::setSize(const sf::Vector2f &size)
{
    m_size = size;
    m_background.setSize(size);

    // Fondo del minijuego
    if (m_bgTexture.loadFromFile("assets/images/niveles/nivel6/roosterhunt_bg.png"))
    {
        m_bgSprite = std::make_unique<sf::Sprite>(m_bgTexture);
        sf::Vector2u texSize = m_bgTexture.getSize();

        // Escala para ajustar al espacio disponible
        float scaleX = (m_size.x - 40.f) / static_cast<float>(texSize.x);
        float scaleY = (m_size.y - 80.f) / static_cast<float>(texSize.y);
        float scale = std::min(scaleX, scaleY);
        m_bgSprite->setScale(sf::Vector2f(scale, scale));
        sf::FloatRect bounds = m_bgSprite->getLocalBounds();
        m_bgSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));

        // POSICIÓN DEL BACKGROUND
        m_bgSprite->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f + 140.f,
            m_position.y + m_size.y / 2.f + 30.f));

        // Guardar los límites reales del background
        if (m_bgSprite)
        {
            m_bgBounds = m_bgSprite->getGlobalBounds();
            std::cout << "Background bounds: "
                      << m_bgBounds.position.x << ", " << m_bgBounds.position.y
                      << " - " << m_bgBounds.size.x << "x" << m_bgBounds.size.y << std::endl;
        }
    }

    // Textos
    if (m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf"))
    {
        // Titulo
        m_titleText = std::make_unique<sf::Text>(m_font);
        m_titleText->setString("CAZA DE GALLOS");
        m_titleText->setCharacterSize(28); // Tamaño del título
        m_titleText->setFillColor(sf::Color::Yellow);
        sf::FloatRect tb = m_titleText->getLocalBounds();
        m_titleText->setOrigin(sf::Vector2f(tb.size.x / 2.f, 0.f));
        m_titleText->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f,
            m_position.y + 65.f));

        m_ammoText = std::make_unique<sf::Text>(m_font);
        m_ammoText->setCharacterSize(25);
        m_ammoText->setFillColor(sf::Color(200, 150, 50));
        m_ammoText->setPosition(sf::Vector2f(
            m_position.x + 200.f,
            m_position.y + 660.f));

        m_closeText = std::make_unique<sf::Text>(m_font);
        m_closeText->setString("ESC para salir");
        m_closeText->setCharacterSize(20);
        m_closeText->setFillColor(sf::Color(150, 150, 150));
        m_closeText->setPosition(sf::Vector2f(m_position.x + m_size.x - 120.f, m_position.y + m_size.y - 30.f));

        m_messageText = std::make_unique<sf::Text>(m_font);
        m_messageText->setCharacterSize(30);
        m_messageText->setStyle(sf::Text::Bold);
    }
}

void MinigameRoosterHunt::activate()
{
    m_isActive = true;
    reset();
}

void MinigameRoosterHunt::deactivate()
{
    m_isActive = false;
    m_roosters.clear();
}

void MinigameRoosterHunt::reset()
{
    m_gameWon = false;
    m_score = 0;
    m_ammo = 10;
    m_roosters.clear();
    m_spawnTimer = 0.f;
    m_messageTimer = 0.f;

    initScoreIcons();

    if (m_ammoText)
        m_ammoText->setString("Balas: " + std::to_string(m_ammo));
    if (m_titleText)
    {
        m_titleText->setString("CAZA DE GALLOS");
        m_titleText->setFillColor(sf::Color::Yellow);
    }
}

void MinigameRoosterHunt::spawnRooster()
{
    if (m_flyTextures.empty())
        return;

    FlyingRooster rooster;
    rooster.sprite = std::make_unique<sf::Sprite>(m_flyTextures[0]);
    rooster.sprite->setScale(sf::Vector2f(m_roosterScale, m_roosterScale));

    sf::FloatRect bounds = rooster.sprite->getLocalBounds();
    rooster.sprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));

    // Bordes del background
    float bgLeft = m_bgBounds.position.x + 50.f;                      // ← 50px más a la derecha
    float bgRight = m_bgBounds.position.x + m_bgBounds.size.x - 50.f; // ← 50px más a la izquierda
    float bgTop = m_bgBounds.position.y;
    float bgBottom = m_bgBounds.position.y + m_bgBounds.size.y;

    std::uniform_real_distribution<float> speedXDist(-100.f, 100.f);
    std::uniform_real_distribution<float> speedYDist(-(m_roosterSpeedMax), -(m_roosterSpeedMin));

    // Elegir spawn aleatorio con MAYOR probabilidad de abajo
    int spawnSide = m_gen() % 10; // 0-9
    // 0-5 = abajo (60%), 6-7 = izquierda (20%), 8-9 = derecha (20%)

    float spawnX, spawnY;
    float velX, velY;

    if (spawnSide <= 5)
    {
        // Spawn desde ABAJO (60% de probabilidad)
        spawnX = bgLeft + (m_gen() % (int)(bgRight - bgLeft));
        spawnY = bgBottom + 30.f - 40.f; // ← 40px más arriba (antes +30.f, ahora -10.f)
        velX = speedXDist(m_gen);
        velY = speedYDist(m_gen);
    }
    else if (spawnSide <= 7)
    {
        // Spawn desde IZQUIERDA (20% de probabilidad)
        spawnX = bgLeft - 50.f + 50.f; // ← 50px más a la derecha (antes -50.f, ahora 0.f)
        spawnY = bgTop + 50.f + (m_gen() % (int)(bgBottom - bgTop - 100.f));
        velX = std::abs(speedXDist(m_gen));
        velY = speedYDist(m_gen) * 0.5f;
    }
    else
    {
        // Spawn desde DERECHA (20% de probabilidad)
        spawnX = bgRight + 50.f - 50.f; // ← 50px más a la izquierda (antes +50.f, ahora 0.f)
        spawnY = bgTop + 50.f + (m_gen() % (int)(bgBottom - bgTop - 100.f));
        velX = -std::abs(speedXDist(m_gen));
        velY = speedYDist(m_gen) * 0.5f;
    }

    rooster.sprite->setPosition(sf::Vector2f(spawnX, spawnY));
    rooster.velocity = sf::Vector2f(velX, velY);
    rooster.frameTime = 0.f;
    rooster.currentFrame = 0;
    rooster.alive = true;
    rooster.dying = false;
    rooster.speed = std::abs(rooster.velocity.y);

    float flipX = (rooster.velocity.x >= 0) ? m_roosterScale : -m_roosterScale;
    rooster.sprite->setScale(sf::Vector2f(flipX, m_roosterScale));

    m_roosters.push_back(std::move(rooster));
}

void MinigameRoosterHunt::updateRoosters(float dt)
{
    std::uniform_real_distribution<float> dirChangeDist(-1.f, 1.f);

    // Bordes del background para VUELO (más estrecho que el bg real)
    float bgLeft = m_bgBounds.position.x + 50.f;
    float bgRight = m_bgBounds.position.x + m_bgBounds.size.x - 50.f;
    float bgTop = m_bgBounds.position.y;
    float bgBottom = m_bgBounds.position.y + m_bgBounds.size.y;

    for (auto &r : m_roosters)
    {
        if (!r.alive)
            continue;

        sf::Vector2f pos = r.sprite->getPosition();

        // Si está muriendo (cayendo)
        if (r.dying)
        {
            r.velocity.y += 500.f * dt;
            r.velocity.x *= 0.98f;
            pos += r.velocity * dt;
            r.sprite->setPosition(pos);

            if (pos.y > bgBottom + 30.f)
            {
                r.alive = false;
            }
            continue;
        }

        // Movimiento normal
        pos += r.velocity * dt;
        r.sprite->setPosition(pos);

        // Cambio de dirección aleatorio
        if (m_gen() % 100 < 4)
        {
            r.velocity.x += dirChangeDist(m_gen) * 60.f;
            r.velocity.y += dirChangeDist(m_gen) * 40.f;
            r.velocity.x = std::clamp(r.velocity.x, -250.f, 250.f);
            r.velocity.y = std::clamp(r.velocity.y, -(m_roosterSpeedMax), 150.f);
        }

        r.velocity.y += 40.f * dt;

        // Animación de vuelo
        r.frameTime += dt;
        if (r.frameTime >= 0.08f)
        {
            r.frameTime = 0.f;
            r.currentFrame = (r.currentFrame + 1) % m_flyTextures.size();
            r.sprite->setTexture(m_flyTextures[r.currentFrame]);
            float flipX = (r.velocity.x >= 0) ? m_roosterScale : -m_roosterScale;
            r.sprite->setScale(sf::Vector2f(flipX, m_roosterScale));
        }

        // VERIFICAR SI SE ESCAPÓ
        bool escaped = false;

        // Por arriba (40px más abajo que antes)
        if (pos.y < bgTop + 20.f)
        {
            escaped = true;
        }
        // Por abajo
        if (pos.y > bgBottom + 80.f && !r.dying)
        {
            escaped = true;
        }
        // Por izquierda (50px más estrecho)
        if (pos.x < bgLeft - 70.f)
        { // ← Antes -120.f, ahora -70.f
            escaped = true;
        }
        // Por derecha (50px más estrecho)
        if (pos.x > bgRight + 70.f)
        { // ← Antes +120.f, ahora +70.f
            escaped = true;
        }

        if (escaped)
        {
            r.alive = false;
        }
    }

    // Limpiar muertos
    m_roosters.erase(
        std::remove_if(m_roosters.begin(), m_roosters.end(),
                       [](const FlyingRooster &r)
                       { return !r.alive; }),
        m_roosters.end());
}

void MinigameRoosterHunt::shoot(const sf::Vector2f &mousePos)
{
    if (m_ammo <= 0 || m_gameWon)
        return;

    m_ammo--;
    if (m_ammoText)
        m_ammoText->setString("Balas: " + std::to_string(m_ammo));

    bool hit = false;
    for (auto &r : m_roosters)
    {
        if (!r.alive || r.dying)
            continue;
        if (r.sprite->getGlobalBounds().contains(mousePos))
        {
            // ¡Impacto! El gallo muere y cae
            r.dying = true;
            r.velocity = sf::Vector2f(
                (r.velocity.x >= 0 ? -50.f : 50.f), // Pequeño impulso lateral
                50.f                                // Empieza a caer
            );

            // Cambiar a textura de muerto
            r.sprite->setTexture(m_deadTexture);
            float flipX = (r.velocity.x >= 0) ? m_roosterScale : -m_roosterScale;
            r.sprite->setScale(sf::Vector2f(flipX, m_roosterScale));

            m_score++;
            hit = true;

            // Cambiar el icono correspondiente a gallo cazado
            if (m_score - 1 < (int)m_scoreIcons.size())
            {
                m_scoreIcons[m_score - 1]->setTexture(m_deadIconTexture);
            }
            if (m_score >= m_scoreToWin)
            {
                m_gameWon = true;

                // Eliminar todos los gallos vivos excepto el que está muriendo
                for (auto &other : m_roosters)
                {
                    if (!other.dying && other.alive)
                    {
                        other.alive = false; // Desaparecen los demás
                    }
                }

                if (m_titleText)
                {
                    m_titleText->setString("Cazaste " + std::to_string(m_scoreToWin) + " gallos");
                    m_titleText->setFillColor(sf::Color::Green);
                }
                showMessage("OBTUVISTE UN GALLO", sf::Color::Green);
            }
            break;
        }
    }

    if (!hit)
    {
        showMessage("Fallaste", sf::Color::Red);
    }

    if (m_ammo <= 0 && !m_gameWon)
    {
        if (m_titleText)
        {
            m_titleText->setString("SIN BALAS - ESC para salir");
            m_titleText->setFillColor(sf::Color::Red);
        }
    }
}

void MinigameRoosterHunt::showMessage(const std::string &msg, const sf::Color &color)
{
    if (!m_messageText)
        return;
    m_messageText->setString(msg);
    m_messageText->setFillColor(color);
    sf::FloatRect bounds = m_messageText->getLocalBounds();
    m_messageText->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    m_messageText->setPosition(sf::Vector2f(
        m_position.x + m_size.x / 2.f,
        m_position.y + m_size.y - 60.f));
    m_messageTimer = 1.5f;
}

void MinigameRoosterHunt::handleEvent(const sf::Event &event, const sf::RenderWindow &window)
{
    if (!m_isActive || m_gameWon)
        return;

    // Disparar con click
    if (event.is<sf::Event::MouseButtonPressed>())
    {
        const auto &mouseEvent = event.getIf<sf::Event::MouseButtonPressed>();
        if (mouseEvent->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mousePos = window.mapPixelToCoords(
                sf::Mouse::getPosition(window), window.getDefaultView());
            shoot(mousePos);
        }
    }
}

void MinigameRoosterHunt::update(float dt)
{
    if (!m_isActive)
        return;

    if (m_messageTimer > 0.f)
    {
        m_messageTimer -= dt;
        if (m_messageTimer <= 0.f && m_messageText)
        {
            m_messageText->setString("");
        }
    }
    if (m_gameWon)
    {
        // Solo actualizar gallos para que el último termine de caer
        updateRoosters(dt);
        return;
    }
    if (m_ammo <= 0)
        return;

    updateRoosters(dt);

    m_spawnTimer += dt;
    if (m_spawnTimer >= m_spawnInterval || m_roosters.size() < 2)
    {
        m_spawnTimer = 0.f;
        if (m_roosters.size() < 4)
        {
            spawnRooster();
        }
    }

    if (m_roosters.empty())
    {
        spawnRooster();
        spawnRooster();
    }
}

void MinigameRoosterHunt::draw(sf::RenderWindow &window)
{
    if (!m_isActive)
        return;

    window.draw(m_background);

    if (m_bgSprite)
        window.draw(*m_bgSprite);

    // Dibujar gallos volando
    for (const auto &r : m_roosters)
    {
        if (r.alive)
            window.draw(*r.sprite);
    }

    // Dibujar crosshair
    sf::Vector2f mousePos = window.mapPixelToCoords(
        sf::Mouse::getPosition(window), window.getDefaultView());

    if (m_crosshair)
    {
        m_crosshair->setPosition(mousePos);
        window.draw(*m_crosshair);
    }
    else
    {
        // Respaldo: círculo rojo
        sf::CircleShape fallback(15.f);
        fallback.setFillColor(sf::Color(255, 0, 0, 150));
        fallback.setOutlineThickness(2.f);
        fallback.setOutlineColor(sf::Color::Red);
        fallback.setOrigin(sf::Vector2f(15.f, 15.f));
        fallback.setPosition(mousePos);
        window.draw(fallback);
    }

    if (m_titleText)
        window.draw(*m_titleText);
    // Dibujar iconos de gallos
    for (auto &icon : m_scoreIcons)
    {
        window.draw(*icon);
    }
    if (m_ammoText)
        window.draw(*m_ammoText);
    if (m_closeText)
        window.draw(*m_closeText);
    if (m_messageText && m_messageTimer > 0.f)
        window.draw(*m_messageText);
}

void MinigameRoosterHunt::initScoreIcons()
{
    m_scoreIcons.clear();

    float iconScale = 0.6f; // Escala para iconos de 68x70
    float startX = m_position.x + 30.f;
    float startY = m_position.y + 560.f;
    float spacing = 50.f; // Espacio entre iconos

    for (int i = 0; i < m_scoreToWin; i++)
    {
        auto icon = std::make_unique<sf::Sprite>(m_aliveIconTexture);
        icon->setScale(sf::Vector2f(iconScale, iconScale));
        icon->setPosition(sf::Vector2f(startX + i * spacing, startY));
        m_scoreIcons.push_back(std::move(icon));
    }
}