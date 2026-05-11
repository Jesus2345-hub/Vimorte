#pragma once
#include <SFML/Graphics.hpp>
#include <random> 
#include <vector>
#include <memory>

class MinigameTicTacToe {
private:
    // Dimensiones
    sf::Vector2f m_size;
    sf::Vector2f m_position;
    
    // Fondo
    sf::RectangleShape m_background;
    sf::Texture m_bgTexture;
    std::unique_ptr<sf::Sprite> m_bgSprite;
    
    // Tablero
    sf::Texture m_tableroTexture;
    std::unique_ptr<sf::Sprite> m_tableroSprite;
    
    // Fichas
    sf::Texture m_xTexture;
    sf::Texture m_oTexture;
    
    // Línea de victoria
    sf::Texture m_winLineTexture;
    std::unique_ptr<sf::Sprite> m_winLineSprite;
    
    // Estado del juego
    bool m_isActive;
    bool m_gameWon;
    bool m_gameLost;
    bool m_playerTurn;        // true = jugador (X), false = joven (O)
    
    // Tablero lógico
    std::vector<int> m_board;  // 0 = vacío, 1 = X, 2 = O
    std::vector<std::unique_ptr<sf::Sprite>> m_boardSprites;
    
    // UI
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_titleText;
    std::unique_ptr<sf::Text> m_turnText;
    std::unique_ptr<sf::Text> m_closeText;
    std::unique_ptr<sf::Text> m_resultText;
    
    // Mensaje temporal
    float m_messageTimer;

    // Random para la IA
    std::random_device m_rd;
    std::mt19937 m_gen;
public:
    MinigameTicTacToe();
    
    void setPosition(const sf::Vector2f& pos);
    void setSize(const sf::Vector2f& size);
    void activate();
    void deactivate();
    bool isActive() const { return m_isActive; }
    bool isGameWon() const { return m_gameWon; }
    
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    void reset();
    
private:
    void initBoard();
    void placeMark(int cellX, int cellY);
    void aiMove();
    int checkWinner();
    void showWinLine(int winner);
    int getCellFromMouse(const sf::Vector2f& mousePos);
    sf::Vector2f getCellPosition(int cellIndex);
};