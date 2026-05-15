#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <vector>
#include <memory>
#include <map>
#include "VitalSigns.hpp"

// ESTRUCCTURA PARA BLOQUES DE COLORES
struct ColorBlock {
    sf::Color color;
    sf::Vector2i gridPos;
    sf::FloatRect worldRect;
    bool isPlaced;
};

class MiniGameTetris {
public:
    MiniGameTetris();
    
    void setPosition(const sf::Vector2f& pos);
    void setSize(const sf::Vector2f& size);
    void activate();
    void deactivate();
    bool isActive() const { return m_isActive; }
    bool isWon() const { return m_gameWon; }

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    void reset();
    void setVitalSigns(VitalSigns* signs) { m_vitalSigns = signs; }
    
    void generateRandomPattern();
    void showPatternTemporarily();

private:

    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_backgroundSprite;
    bool m_hasBackgroundTexture;

    std::vector<sf::Vector2f> m_cellPositions;  
    std::vector<sf::Vector2f> m_inventoryPositions;
    sf::Vector2u m_lastWindowSize;
    std::unique_ptr<sf::Text> m_cachedTargetLabel;
    std::unique_ptr<sf::Text> m_cachedPlayerLabel;
    float m_cachedScreenW;
    float m_cachedScreenH;
    float m_cachedScale;
    float m_cachedPlayerStartX;
    float m_cachedGridSize;
    float m_cachedStartY;
    
    void recalculateLayout(float screenW, float screenH);
    static constexpr int GRID_SIZE = 3;
    static constexpr int BASE_BLOCK_SIZE = 80;
    static constexpr float PATTERN_VISIBLE_TIME = 3.0f;  
    
    std::vector<sf::Color> m_availableColors;
    std::vector<std::vector<sf::Color>> m_currentTargetPattern; 
    std::vector<std::vector<sf::Color>> m_playerPattern;
    std::vector<ColorBlock> m_availableBlocks;
    
    // PARA LOS TRES PATRONES
    std::vector<std::vector<std::vector<sf::Color>>> m_allPatterns;
    int m_currentPatternIndex;
    int m_patternsCompleted;
    
    bool m_isActive;
    bool m_gameWon;
    int m_blocksPlaced;
    
    sf::Vector2f m_position;
    sf::Vector2f m_size;
    float m_scaleFactor;
    float m_currentBlockSize;
    
    // UI 
    sf::RectangleShape m_background;
    sf::Font m_font;
    bool m_fontLoaded;
    
    std::unique_ptr<sf::Text> m_titleText;
    std::unique_ptr<sf::Text> m_instructionText;
    std::unique_ptr<sf::Text> m_patternCounterText;
    std::unique_ptr<sf::Text> m_messageText;
    std::unique_ptr<sf::Text> m_closeText;
    std::unique_ptr<sf::Text> m_verifyButtonText;
    std::unique_ptr<sf::Text> m_hintText; 
    
    // FONDOS
    sf::RectangleShape m_targetGridBackground;
    sf::RectangleShape m_playerGridBackground;
    sf::RectangleShape m_inventoryBackground;
    sf::RectangleShape m_verifyButton;
    
    std::vector<sf::RectangleShape> m_targetCells;
    std::vector<sf::RectangleShape> m_playerCells;
    std::vector<sf::RectangleShape> m_inventoryBlocks;
    
    // PARA ALMACENAR LOS BORDES DE VERIFICACION
    std::vector<int> m_wrongCells;  
    std::vector<int> m_correctCells; 
    bool m_showVerification;
    float m_verificationTimer;
    
    int m_selectedBlockIndex;
    bool m_isDragging;
    sf::Vector2f m_dragOffset;
    
    float m_messageTimer;
    std::string m_currentMessage;
    sf::Color m_messageColor;
    
    // TEMPORIZADOR PARA MOSTRAR EL PATRON
    float m_patternVisibleTimer;
    bool m_isPatternVisible;
    bool m_waitingForVerification;  
    bool m_roundCompleted;  
    float m_roundCompleteTimer;
    
    VitalSigns* m_vitalSigns = nullptr;
    
    void initUI();
    void updateLayout();
    void checkPatternComplete();  
    void showTemporaryMessage(const std::string& msg, const sf::Color& color, float duration);
    void updateMessage(float dt);
    void resetPatterns();
    void generateBlocksFromPattern();
    void generateAllPatterns();  
    void loadNextPattern();  
    void verifyPattern();  
    void showVerificationResult();  
    void markCellAsCorrect(int index);
    void markCellAsWrong(int index);
    void clearVerificationMarks();
    void nextRound();  
    sf::Vector2i getGridCellFromPosition(const sf::Vector2f& pos);
    bool isValidPlacement(int row, int col, const sf::Color& color);
    void placeBlock(int blockIndex, int row, int col);
    bool isGridComplete();  
};