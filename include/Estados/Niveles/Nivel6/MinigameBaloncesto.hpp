#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <random>

class MinigameBaloncesto
{
private:
    sf::Vector2f m_size;
    sf::Vector2f m_position;

    // Fondo
    sf::RectangleShape m_background;
    sf::Texture m_bgTexture;
    std::unique_ptr<sf::Sprite> m_bgSprite;

    float m_barraX;
    float m_barraY;
    float m_barraAlto;
    float m_barraAnchoGuardado;
    float m_escalaRefGuardada;
    bool m_cerrando = false;
    float m_tiempoCierre = 0.f;

    // Aro
    sf::Texture m_aroVacioTexture;
    sf::Texture m_aroEncestandoTexture;
    sf::Texture m_aroEncestadoTexture;
    std::unique_ptr<sf::Sprite> m_aroSprite;
    sf::FloatRect m_zonaTiro;
    sf::Vector2f m_rimCenter;
    sf::Vector2f m_aimCenter;
    float m_tiempoEnceste;
    int m_faseEnceste;

    // Pelota
    sf::Texture m_pelotaTexture;
    std::unique_ptr<sf::Sprite> m_pelotaSprite;
    sf::Vector2f m_pelotaVel;
    sf::Vector2f m_ballStart;
    sf::Vector2f m_ballTarget;
    float m_ballTime;
    float m_ballDuration;
    bool m_pelotaVolando;
    bool m_mostrandoEnceste;

    // Barra de intensidad
    sf::RectangleShape m_barraFondo;
    sf::RectangleShape m_barraVerde;
    sf::RectangleShape m_barraIndicador;
    float m_indicadorVel;
    float m_indicadorPos;
    bool m_indicadorSubiendo;
    bool m_intensidadFijada;
    float m_intensidad;

    // Crosshair
    sf::CircleShape m_crosshair;
    sf::Vector2f m_crosshairPos;
    float m_crosshairSpeed;

    // Estados del juego
    enum class Fase
    {
        INTENSIDAD,
        APUNTAR,
        VOLANDO,
        RESULTADO
    };
    Fase m_fase;

    // Puntuación
    bool m_isActive;
    bool m_gameWon;
    bool m_gameLost = false;
    int m_score;
    int m_scoreToWin;
    int m_attempts;
    int m_maxAttempts;

    // UI
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_titleText;
    std::unique_ptr<sf::Text> m_scoreText;
    std::unique_ptr<sf::Text> m_attemptsText;
    std::unique_ptr<sf::Text> m_closeText;
    std::unique_ptr<sf::Text> m_messageText;
    float m_messageTimer;

public:
    MinigameBaloncesto();

    void setPosition(const sf::Vector2f &pos);
    void setSize(const sf::Vector2f &size);
    void activate();
    void deactivate();
    bool isActive() const { return m_isActive; }
    bool isGameWon() const { return m_gameWon; }
    bool isGameLost() const { return m_gameLost; }

    void handleEvent(const sf::Event &event, const sf::RenderWindow &window);
    void update(float dt);
    void draw(sf::RenderWindow &window);
    void reset();

private:
    void iniciarLanzamiento();
    void verificarEnceste();
    void showMessage(const std::string &msg, const sf::Color &color);
};