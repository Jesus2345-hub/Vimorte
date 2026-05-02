#include "MiniGameTetris.hpp"
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>
#include <cmath>

namespace Colors {
    const sf::Color Orange(255, 165, 0);
    const sf::Color DarkOrange(255, 140, 0);
    const sf::Color LightOrange(255, 200, 100);
}

// Constructor
MiniGameTetris::MiniGameTetris()
    : m_hasBackgroundTexture(false), m_isActive(false), m_gameWon(false), m_blocksPlaced(0),
      m_selectedBlockIndex(-1), m_isDragging(false), m_messageTimer(0.f), 
      m_currentBlockSize(60.f), m_currentPatternIndex(0), m_patternsCompleted(0),
      m_patternVisibleTimer(0.f), m_isPatternVisible(true), m_waitingForVerification(false),
      m_roundCompleted(false), m_roundCompleteTimer(0.f), m_showVerification(false),
      m_verificationTimer(0.f), m_lastWindowSize(0, 0), m_backgroundSprite(nullptr)
{
    // Inicializar colores disponibles
    m_availableColors = {
        sf::Color(100, 150, 200),  // Azul
        sf::Color(150, 100, 100),  // Rojo
        sf::Color(100, 150, 100),  // Verde
        sf::Color(200, 180, 100),  // Amarillo
        sf::Color(180, 140, 200)   // Morado
    };
    
    // Cargar textura de fondo
    if (m_backgroundTexture.loadFromFile("assets/images/niveles/nivel_sara/colores.jpg")) {
        m_backgroundSprite = std::make_unique<sf::Sprite>(m_backgroundTexture);
        m_hasBackgroundTexture = true;
        std::cout << "Fondo de Tetris cargado correctamente" << std::endl;
    } else {
        std::cout << "No se pudo cargar fondo de Tetris, usando fondo por defecto" << std::endl;
    }
    
    // Configurar fondo del panel
    m_background.setFillColor(sf::Color(0, 0, 0, 230));
    m_background.setOutlineThickness(3.f);
    m_background.setOutlineColor(sf::Color(150, 150, 150));
    
    // Cargar fuente
    if (!m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "Error cargando fuente en MiniGameTetris" << std::endl;
        m_fontLoaded = false;
    } else {
        m_fontLoaded = true;
    }
    
    initUI();
    m_cachedTargetLabel = nullptr;
    m_cachedPlayerLabel = nullptr;
    generateAllPatterns();
}

// Generar patrones aleatorios
void MiniGameTetris::generateAllPatterns() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> colorDist(0, m_availableColors.size() - 1);
    
    m_allPatterns.clear();
    
    for (int p = 0; p < 3; p++) {
        std::vector<std::vector<sf::Color>> pattern;
        pattern.assign(GRID_SIZE, std::vector<sf::Color>(GRID_SIZE));
        
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                pattern[i][j] = m_availableColors[colorDist(gen)];
            }
        }
        m_allPatterns.push_back(pattern);
    }
    
    loadNextPattern();
}

// Cargar siguiente patrón
void MiniGameTetris::loadNextPattern() {
     if (m_currentPatternIndex < (int)m_allPatterns.size()) {
        // Limpiar datos existentes
        m_playerCells.clear();
        m_inventoryBlocks.clear();
        m_targetCells.clear();
        m_playerPattern.assign(GRID_SIZE, std::vector<sf::Color>(GRID_SIZE, sf::Color::Transparent));
        
        // Configurar patrón actual
        m_currentTargetPattern = m_allPatterns[m_currentPatternIndex];
        
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                m_playerPattern[i][j] = sf::Color::Transparent;
            }
        }
        
        m_currentTargetPattern = m_allPatterns[m_currentPatternIndex];
        
        // Reiniciar estado de la ronda
        m_blocksPlaced = 0;
        m_waitingForVerification = false;
        m_roundCompleted = false;
        m_showVerification = false;
        clearVerificationMarks();
        
        generateBlocksFromPattern();
        
        // Mostrar patrón temporalmente
        m_isPatternVisible = true;
        m_patternVisibleTimer = PATTERN_VISIBLE_TIME;
        
        // Actualizar textos de UI
        if (m_hintText) {
            m_hintText->setString("¡MEMORIZA EL PATRON!");
            m_hintText->setFillColor(sf::Color::Yellow);
        }
        
        if (m_patternCounterText) {
            m_patternCounterText->setString("Patron " + std::to_string(m_currentPatternIndex + 1) + "/3");
        }
        
        m_lastWindowSize = sf::Vector2u(0, 0);
        std::cout << "Cargando patron " << m_currentPatternIndex + 1 << "/3" << std::endl;
    } else {
        // Juego completado
        m_gameWon = true;
        if (m_titleText) m_titleText->setString("COMPLETADO");
        showTemporaryMessage("Has completado los 3 patrones!", sf::Color::Green, 2.f);
        
        if (m_vitalSigns) {
            m_vitalSigns->stabilize();
            m_vitalSigns->applyEffect(15);
        }
    }
}

// Generar patrón aleatorio (wrapper)
void MiniGameTetris::generateRandomPattern() {
    generateAllPatterns();
}

// Generar bloques desde el patrón actual
void MiniGameTetris::generateBlocksFromPattern() {
    m_availableBlocks.clear();
    m_inventoryBlocks.clear();
    
    // Crear bloque por cada celda del patrón
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            ColorBlock block;
            block.color = m_currentTargetPattern[i][j];
            block.gridPos = sf::Vector2i(-1, -1);
            block.isPlaced = false;
            m_availableBlocks.push_back(block);
        }
    }
    
    // Mezclar bloques aleatoriamente
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::shuffle(m_availableBlocks.begin(), m_availableBlocks.end(), gen);
}

// Inicializar elementos de UI
void MiniGameTetris::initUI() {
    if (!m_fontLoaded) return;
    
    m_titleText = std::make_unique<sf::Text>(m_font);
    m_titleText->setString("MEMORIA DE PATRONES");
    m_titleText->setFillColor(sf::Color::Yellow);
    m_titleText->setStyle(sf::Text::Bold);
    
    m_instructionText = std::make_unique<sf::Text>(m_font);
    m_instructionText->setString("Memoriza el patron de colores y replicado");
    m_instructionText->setFillColor(sf::Color(200, 200, 200));
    
    m_hintText = std::make_unique<sf::Text>(m_font);
    m_hintText->setFillColor(sf::Color::Yellow);
    
    m_patternCounterText = std::make_unique<sf::Text>(m_font);
    m_patternCounterText->setFillColor(sf::Color::Cyan);
    m_patternCounterText->setStyle(sf::Text::Bold);
    
    m_closeText = std::make_unique<sf::Text>(m_font);
    m_closeText->setString("ESC para salir");
    m_closeText->setFillColor(sf::Color(150, 150, 150));
    
    m_messageText = std::make_unique<sf::Text>(m_font);
    m_messageText->setStyle(sf::Text::Bold);
    
    m_verifyButtonText = std::make_unique<sf::Text>(m_font);
    m_verifyButtonText->setString("VERIFICAR");
    m_verifyButtonText->setFillColor(sf::Color::Black);
    m_verifyButtonText->setStyle(sf::Text::Bold);
    
    m_verifyButton.setFillColor(sf::Color::Yellow);
    m_verifyButton.setOutlineThickness(2.f);
    m_verifyButton.setOutlineColor(sf::Color::White);
}

// Setters de posición y tamaño
void MiniGameTetris::setPosition(const sf::Vector2f& pos) {
    m_position = pos;
}

void MiniGameTetris::setSize(const sf::Vector2f& size) {
    m_size = size;
}

void MiniGameTetris::updateLayout() {
}

// Recalcular disposición de la UI
void MiniGameTetris::recalculateLayout(float screenW, float screenH) {
    float baseHeight = 720.f;
    float scale = screenH / baseHeight;
    scale = std::clamp(scale, 0.65f, 1.4f);
    
    float blockSize = 48.f * scale;
    blockSize = std::clamp(blockSize, 32.f, 70.f);
    
    float innerSize = GRID_SIZE * blockSize;
    float gridSize = innerSize + 24.f;
    
    float spacingBetweenGrids = 50.f * scale;
    float topMargin = screenH * 0.22f;
    
    float totalWidth = gridSize * 2 + spacingBetweenGrids;
    float startX = (screenW - totalWidth) / 2;
    float startY = topMargin;
    
    // Configurar cuadrícula del patrón objetivo
    m_targetGridBackground.setSize(sf::Vector2f(gridSize, gridSize));
    m_targetGridBackground.setPosition(sf::Vector2f(startX, startY));
    m_targetGridBackground.setFillColor(sf::Color(40, 40, 50, 200));
    m_targetGridBackground.setOutlineThickness(2.f);
    m_targetGridBackground.setOutlineColor(sf::Color(150, 150, 150));
    
    // Configurar cuadrícula del jugador
    float playerStartX = startX + gridSize + spacingBetweenGrids;
    m_playerGridBackground.setSize(sf::Vector2f(gridSize, gridSize));
    m_playerGridBackground.setPosition(sf::Vector2f(playerStartX, startY));
    m_playerGridBackground.setFillColor(sf::Color(40, 40, 50, 200));
    m_playerGridBackground.setOutlineThickness(2.f);
    m_playerGridBackground.setOutlineColor(sf::Color(150, 150, 150));
    
    // Configurar botón de verificación
    float btnW = 160.f * scale;
    float btnH = 48.f * scale;
    float btnY = startY + gridSize + 30.f * scale;
    m_verifyButton.setSize(sf::Vector2f(btnW, btnH));
    m_verifyButton.setPosition(sf::Vector2f(screenW / 2 - btnW / 2, btnY));
    
    if (m_verifyButtonText) {
        m_verifyButtonText->setCharacterSize(static_cast<unsigned int>(22 * scale));
        sf::FloatRect bounds = m_verifyButtonText->getLocalBounds();
        m_verifyButtonText->setOrigin(sf::Vector2f(bounds.size.x / 2, bounds.size.y / 2));
        m_verifyButtonText->setPosition(sf::Vector2f(
            m_verifyButton.getPosition().x + btnW / 2,
            m_verifyButton.getPosition().y + btnH / 2
        ));
    }
    
    // Configurar etiquetas de las cuadrículas
    m_cachedTargetLabel = std::make_unique<sf::Text>(m_font, "PATRON ORIGINAL", static_cast<unsigned int>(16 * scale));
    m_cachedTargetLabel->setFillColor(sf::Color(200, 200, 200));
    sf::FloatRect targetBounds = m_cachedTargetLabel->getLocalBounds();
    m_cachedTargetLabel->setPosition(sf::Vector2f(startX + gridSize / 2 - targetBounds.size.x / 2, startY - 25 * scale));
    
    m_cachedPlayerLabel = std::make_unique<sf::Text>(m_font, "TU REPLICA", static_cast<unsigned int>(16 * scale));
    m_cachedPlayerLabel->setFillColor(sf::Color(200, 200, 200));
    sf::FloatRect playerBounds = m_cachedPlayerLabel->getLocalBounds();
    m_cachedPlayerLabel->setPosition(sf::Vector2f(playerStartX + gridSize / 2 - playerBounds.size.x / 2, startY - 25 * scale));
    
    // Crear celdas del patrón objetivo
    m_targetCells.clear();
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            sf::RectangleShape cell(sf::Vector2f(blockSize - 4, blockSize - 4));
            cell.setPosition(sf::Vector2f(
                startX + 12 + j * blockSize + 2,
                startY + 12 + i * blockSize + 2
            ));
            if (i < (int)m_currentTargetPattern.size() && 
                j < (int)m_currentTargetPattern[i].size()) {
                cell.setFillColor(m_currentTargetPattern[i][j]);
            } else {
                cell.setFillColor(sf::Color(80, 80, 80));
            }
            cell.setOutlineThickness(1);
            cell.setOutlineColor(sf::Color::Black);
            m_targetCells.push_back(cell);
        }
    }
    
    // Crear o actualizar celdas del jugador
    if (m_playerCells.empty()) {
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                sf::RectangleShape cell(sf::Vector2f(blockSize - 4, blockSize - 4));
                cell.setPosition(sf::Vector2f(
                    playerStartX + 12 + j * blockSize + 2,
                    startY + 12 + i * blockSize + 2
                ));
                if (m_playerPattern[i][j] != sf::Color::Transparent) {
                    cell.setFillColor(m_playerPattern[i][j]);
                } else {
                    cell.setFillColor(sf::Color(50, 50, 55, 255));
                }
                cell.setOutlineThickness(2);
                cell.setOutlineColor(sf::Color::White);
                m_playerCells.push_back(cell);
            }
        }
    } else {
        int idx = 0;
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                if (idx < (int)m_playerCells.size()) {
                    m_playerCells[idx].setPosition(sf::Vector2f(
                        playerStartX + 12 + j * blockSize + 2,
                        startY + 12 + i * blockSize + 2
                    ));
                    m_playerCells[idx].setSize(sf::Vector2f(blockSize - 4, blockSize - 4));
                }
                idx++;
            }
        }
    }
    
    // Configurar bloques del inventario
    float invStartY = btnY + btnH + 25.f * scale;
    int cols = 6;
    float invWidth = totalWidth;
    float cellWidth = invWidth / cols;
    float invCellSize = blockSize * 0.85f;
    
    if (m_inventoryBlocks.empty()) {
        for (size_t idx = 0; idx < m_availableBlocks.size(); idx++) {
            int row = idx / cols;
            int col = idx % cols;
            
            float centerX = startX + col * cellWidth + cellWidth / 2;
            float centerY = invStartY + row * (invCellSize + 10) + invCellSize / 2;
            
            sf::RectangleShape block(sf::Vector2f(invCellSize, invCellSize));
            block.setPosition(sf::Vector2f(centerX - invCellSize / 2, centerY - invCellSize / 2));
            
            if (m_availableBlocks[idx].isPlaced) {
                block.setFillColor(sf::Color(80, 80, 80, 100));
                block.setOutlineColor(sf::Color(100, 100, 100));
            } else {
                block.setFillColor(m_availableBlocks[idx].color);
                block.setOutlineColor(sf::Color::White);
            }
            block.setOutlineThickness(2);
            m_inventoryBlocks.push_back(block);
            m_availableBlocks[idx].worldRect = block.getGlobalBounds();
        }
    } else {
        for (size_t idx = 0; idx < m_availableBlocks.size() && idx < m_inventoryBlocks.size(); idx++) {
            int row = idx / cols;
            int col = idx % cols;
            
            float centerX = startX + col * cellWidth + cellWidth / 2;
            float centerY = invStartY + row * (invCellSize + 10) + invCellSize / 2;
            
            m_inventoryBlocks[idx].setSize(sf::Vector2f(invCellSize, invCellSize));
            m_inventoryBlocks[idx].setPosition(sf::Vector2f(centerX - invCellSize / 2, centerY - invCellSize / 2));
            m_availableBlocks[idx].worldRect = m_inventoryBlocks[idx].getGlobalBounds();
        }
    }
    
    m_cachedScale = scale;
    m_cachedPlayerStartX = playerStartX;
    m_cachedGridSize = gridSize;
    m_cachedStartY = startY;
}

// Mostrar patrón temporalmente
void MiniGameTetris::showPatternTemporarily() {
    m_isPatternVisible = true;
    m_patternVisibleTimer = PATTERN_VISIBLE_TIME;
}

// Activar minijuego
void MiniGameTetris::activate() {
    m_isActive = true;
    reset();
}

// Desactivar minijuego
void MiniGameTetris::deactivate() {
    m_isActive = false;
}

// Reiniciar juego
void MiniGameTetris::reset() {
    m_gameWon = false;
    m_currentPatternIndex = 0;
    m_patternsCompleted = 0;
    m_blocksPlaced = 0;
    m_selectedBlockIndex = -1;
    m_isDragging = false;
    m_messageTimer = 0.f;
    m_waitingForVerification = false;
    m_roundCompleted = false;
    m_showVerification = false;
    
    // Limpiar vectores
    m_playerCells.clear();
    m_inventoryBlocks.clear();
    m_targetCells.clear();
    
    // Limpiar patrones del jugador
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            m_playerPattern[i][j] = sf::Color::Transparent;
        }
    }
    
    clearVerificationMarks();
    generateAllPatterns();
}

// Manejar eventos de entrada
void MiniGameTetris::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!m_isActive || m_gameWon) return;
    
    // Manejar clic del mouse
    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos = sf::Vector2f(mousePressed->position);
            
            // Verificar clic en botón de verificar
            if (m_verifyButton.getGlobalBounds().contains(mousePos)) {
                if (!m_waitingForVerification && !m_roundCompleted && isGridComplete()) {
                    verifyPattern();
                } else if (!isGridComplete()) {
                    showTemporaryMessage("¡Completa todos los espacios primero!", Colors::Orange, 1.5f);
                }
                return;
            }
            
            // Iniciar arrastre de bloque
            if (!m_waitingForVerification && !m_roundCompleted && !m_isDragging) {
                // Buscar bloque en inventario
                for (size_t i = 0; i < m_availableBlocks.size(); i++) {
                    if (!m_availableBlocks[i].isPlaced && 
                        m_availableBlocks[i].worldRect.contains(mousePos)) {
                        m_selectedBlockIndex = i;
                        m_isDragging = true;
                        m_dragOffset = mousePos - sf::Vector2f(
                            m_availableBlocks[i].worldRect.position.x,
                            m_availableBlocks[i].worldRect.position.y
                        );
                        break;
                    }
                }
                
                // Buscar bloque colocado para quitarlo
                if (m_selectedBlockIndex == -1) {
                    for (size_t i = 0; i < m_availableBlocks.size(); i++) {
                        if (m_availableBlocks[i].isPlaced) {
                            int gridRow = m_availableBlocks[i].gridPos.y;
                            int gridCol = m_availableBlocks[i].gridPos.x;
                            if (gridRow >= 0 && gridCol >= 0) {
                                int cellIndex = gridRow * GRID_SIZE + gridCol;
                                if (cellIndex < (int)m_playerCells.size()) {
                                    sf::FloatRect cellBounds = m_playerCells[cellIndex].getGlobalBounds();
                                    if (cellBounds.contains(mousePos)) {
                                        // Quitar bloque de la cuadrícula
                                        m_playerPattern[gridRow][gridCol] = sf::Color::Transparent;
                                        m_playerCells[cellIndex].setFillColor(sf::Color(50, 50, 55, 255));
                                        m_availableBlocks[i].isPlaced = false;
                                        m_availableBlocks[i].gridPos = sf::Vector2i(-1, -1);
                                        m_inventoryBlocks[i].setFillColor(m_availableBlocks[i].color);
                                        m_inventoryBlocks[i].setOutlineColor(sf::Color::White);
                                        m_blocksPlaced--;
                                        showTemporaryMessage("Bloque retirado", sf::Color(255, 165, 0), 0.5f);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Manejar liberación del mouse (soltar bloque)
    if (const auto* mouseReleased = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseReleased->button == sf::Mouse::Button::Left && m_isDragging && !m_waitingForVerification && !m_roundCompleted) {
            sf::Vector2f mousePos = sf::Vector2f(mouseReleased->position);
            sf::FloatRect playerGridBounds = m_playerGridBackground.getGlobalBounds();
            
            if (playerGridBounds.contains(mousePos) && m_selectedBlockIndex >= 0) {
                sf::Vector2i gridCell = getGridCellFromPosition(mousePos);
                if (gridCell.x >= 0 && gridCell.x < GRID_SIZE && 
                    gridCell.y >= 0 && gridCell.y < GRID_SIZE) {
                    
                    if (m_playerPattern[gridCell.y][gridCell.x] == sf::Color::Transparent) {
                        placeBlock(m_selectedBlockIndex, gridCell.y, gridCell.x);
                        showTemporaryMessage("Bloque colocado", sf::Color::Green, 0.5f);
                    } else {
                        showTemporaryMessage("¡Casilla ocupada!", Colors::Orange, 0.8f);
                    }
                }
            }
            m_isDragging = false;
            m_selectedBlockIndex = -1;
        }
    }
}

// Obtener celda de la cuadrícula desde posición del mouse
sf::Vector2i MiniGameTetris::getGridCellFromPosition(const sf::Vector2f& pos) {
    sf::FloatRect gridBounds = m_playerGridBackground.getGlobalBounds();
    if (gridBounds.size.x <= 20) return sf::Vector2i(-1, -1);
    
    float cellSize = (gridBounds.size.x - 24) / GRID_SIZE;
    
    float localX = pos.x - (gridBounds.position.x + 12);
    float localY = pos.y - (gridBounds.position.y + 12);
    
    if (localX < 0 || localY < 0) return sf::Vector2i(-1, -1);
    
    int col = static_cast<int>(localX / cellSize);
    int row = static_cast<int>(localY / cellSize);
    
    if (row >= GRID_SIZE || col >= GRID_SIZE) return sf::Vector2i(-1, -1);
    
    return sf::Vector2i(col, row);
}

// Verificar si una posición es válida para colocar bloque
bool MiniGameTetris::isValidPlacement(int row, int col, const sf::Color& color) {
    return m_playerPattern[row][col] == sf::Color::Transparent;
}

// Colocar bloque en la cuadrícula
void MiniGameTetris::placeBlock(int blockIndex, int row, int col) {
    // Si el bloque ya estaba colocado en otra posición, quitarlo primero
    if (m_availableBlocks[blockIndex].isPlaced) {
        int oldRow = m_availableBlocks[blockIndex].gridPos.y;
        int oldCol = m_availableBlocks[blockIndex].gridPos.x;
        if (oldRow >= 0 && oldCol >= 0) {
            m_playerPattern[oldRow][oldCol] = sf::Color::Transparent;
            int oldIndex = oldRow * GRID_SIZE + oldCol;
            if (oldIndex < (int)m_playerCells.size()) {
                m_playerCells[oldIndex].setFillColor(sf::Color(50, 50, 55, 255));
                m_playerCells[oldIndex].setOutlineThickness(2);
                m_playerCells[oldIndex].setOutlineColor(sf::Color::White);
            }
            m_blocksPlaced--;
        }
    }
    
    // Colocar bloque en nueva posición
    m_availableBlocks[blockIndex].isPlaced = true;
    m_availableBlocks[blockIndex].gridPos = sf::Vector2i(col, row);
    m_playerPattern[row][col] = m_availableBlocks[blockIndex].color;
    m_blocksPlaced++;
    
    int cellIndex = row * GRID_SIZE + col;
    if (cellIndex < (int)m_playerCells.size()) {
        m_playerCells[cellIndex].setFillColor(m_availableBlocks[blockIndex].color);
        m_playerCells[cellIndex].setOutlineThickness(2);
        m_playerCells[cellIndex].setOutlineColor(sf::Color::White);
    }
    
    // Ocultar bloque en inventario
    if (blockIndex < (int)m_inventoryBlocks.size()) {
        m_inventoryBlocks[blockIndex].setFillColor(sf::Color(80, 80, 80, 100));
        m_inventoryBlocks[blockIndex].setOutlineColor(sf::Color(100, 100, 100));
    }
}

// Verificar si el patrón del jugador es correcto
void MiniGameTetris::verifyPattern() {
    if (!isGridComplete()) {
        showTemporaryMessage("¡Completa todos los espacios primero!", Colors::Orange, 1.5f);
        return;
    }
    
    m_waitingForVerification = true;
    m_showVerification = true;
    
    bool allCorrect = true;
    
    // Comparar cada celda con el patrón objetivo
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            int cellIndex = i * GRID_SIZE + j;
            
            if (cellIndex >= (int)m_playerCells.size()) continue;
            
            if (m_playerPattern[i][j] == m_currentTargetPattern[i][j]) {
                m_playerCells[cellIndex].setOutlineThickness(5);
                m_playerCells[cellIndex].setOutlineColor(sf::Color::Green);
            } else {
                m_playerCells[cellIndex].setOutlineThickness(5);
                m_playerCells[cellIndex].setOutlineColor(sf::Color::Red);
                allCorrect = false;
            }
        }
    }
    
    // Procesar resultado
    if (allCorrect) {
        showTemporaryMessage("¡PATRON CORRECTO!", sf::Color::Green, 1.5f);
        m_currentPatternIndex++;
        m_patternsCompleted++;
        m_roundCompleted = true;
        m_roundCompleteTimer = 1.5f;
        
        if (m_vitalSigns) {
            m_vitalSigns->applyEffect(5);
        }
    } else {
        showTemporaryMessage("Patron incorrecto. Revisa los colores marcados", sf::Color::Red, 2.f);
        
        if (m_vitalSigns) {
            m_vitalSigns->applyEffect(-2);
        }
    }
    
    m_verificationTimer = 2.5f;
}

void MiniGameTetris::showVerificationResult() {
}

// Marcar celda como correcta
void MiniGameTetris::markCellAsCorrect(int index) {
    m_correctCells.push_back(index);
}

// Marcar celda como incorrecta
void MiniGameTetris::markCellAsWrong(int index) {
    m_wrongCells.push_back(index);
}

// Limpiar marcas de verificación
void MiniGameTetris::clearVerificationMarks() {
    m_correctCells.clear();
    m_wrongCells.clear();
    
    for (auto& cell : m_playerCells) {
        cell.setOutlineThickness(2);
        cell.setOutlineColor(sf::Color::White);
    }
}

// Avanzar a la siguiente ronda
void MiniGameTetris::nextRound() {
    // Limpiar completamente las celdas y bloques existentes
    m_playerCells.clear();
    m_inventoryBlocks.clear();
    m_targetCells.clear();
    
    // Limpiar patrones del jugador
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            m_playerPattern[i][j] = sf::Color::Transparent;
        }
    }
    
    m_blocksPlaced = 0;
    m_showVerification = false;
    m_waitingForVerification = false;
    clearVerificationMarks();
    
    // Cargar el siguiente patrón
    loadNextPattern();
}

// Verificar si la cuadrícula del jugador está completa
bool MiniGameTetris::isGridComplete() {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (m_playerPattern[i][j] == sf::Color::Transparent) {
                return false;
            }
        }
    }
    return true;
}

void MiniGameTetris::checkPatternComplete() {
}

// Actualizar lógica del juego
void MiniGameTetris::update(float dt) {
    if (!m_isActive || m_gameWon) return;
    
    updateMessage(dt);
    
    // Ocultar patrón después del tiempo límite
    if (m_isPatternVisible && !m_roundCompleted && !m_waitingForVerification) {
        m_patternVisibleTimer -= dt;
        if (m_patternVisibleTimer <= 0.f) {
            m_isPatternVisible = false;
            if (m_hintText) {
                m_hintText->setString("¡El patron ha desaparecido! Replica de memoria");
                m_hintText->setFillColor(sf::Color::Red);
            }
        }
    }
    
    // Esperar antes de cargar siguiente ronda
    if (m_roundCompleted) {
        m_roundCompleteTimer -= dt;
        if (m_roundCompleteTimer <= 0.f) {
            m_roundCompleted = false;
            m_waitingForVerification = false;
            m_showVerification = false;
            nextRound();
        }
    }
    
    // Limpiar marcas de verificación después del tiempo
    if (m_showVerification && m_verificationTimer > 0.f) {
        m_verificationTimer -= dt;
        if (m_verificationTimer <= 0.f) {
            for (auto& cell : m_playerCells) {
                cell.setOutlineThickness(2);
                cell.setOutlineColor(sf::Color::White);
            }
            m_showVerification = false;
            m_waitingForVerification = false;
        }
    }
}

// Dibujar el minijuego
void MiniGameTetris::draw(sf::RenderWindow& window) {
    if (!m_isActive) {
        return;
    }
    
    sf::Vector2u windowSize = window.getSize();
    float screenW = static_cast<float>(windowSize.x);
    float screenH = static_cast<float>(windowSize.y);
    
    // Recalcular layout si cambió el tamaño de la ventana
    if (windowSize != m_lastWindowSize) {
        m_lastWindowSize = windowSize;
        recalculateLayout(screenW, screenH);
        
        if (m_hasBackgroundTexture && m_backgroundSprite) {
            sf::Vector2u texSize = m_backgroundTexture.getSize();
            float scaleX = screenW / texSize.x;
            float scaleY = screenH / texSize.y;
            m_backgroundSprite->setScale(sf::Vector2f(scaleX, scaleY));
            m_backgroundSprite->setPosition(sf::Vector2f(0, 0));
        }
    }
    
    // Dibujar overlay semitransparente
    sf::RectangleShape overlay(sf::Vector2f(screenW, screenH));
    overlay.setFillColor(sf::Color(0, 0, 0, 90));
    window.draw(overlay);
    
    // Dibujar panel principal
    m_background.setSize(sf::Vector2f(screenW * 0.95f, screenH * 0.92f));
    m_background.setPosition(sf::Vector2f(screenW * 0.025f, screenH * 0.04f));
    m_background.setFillColor(sf::Color(0, 0, 0, 200));
    m_background.setOutlineThickness(3.f);
    m_background.setOutlineColor(sf::Color(150, 150, 150));
    window.draw(m_background);
    
    float scale = m_cachedScale;
    if (scale < 0.1f) scale = 1.0f;
    
    // Dibujar textos de título e instrucciones
    if (m_titleText) {
        m_titleText->setCharacterSize(static_cast<unsigned int>(32 * scale));
        sf::FloatRect bounds = m_titleText->getLocalBounds();
        m_titleText->setPosition(sf::Vector2f(screenW / 2 - bounds.size.x / 2, screenH * 0.03f));
        window.draw(*m_titleText);
    }
    
    if (m_instructionText) {
        m_instructionText->setCharacterSize(static_cast<unsigned int>(18 * scale));
        sf::FloatRect bounds = m_instructionText->getLocalBounds();
        m_instructionText->setPosition(sf::Vector2f(screenW / 2 - bounds.size.x / 2, screenH * 0.08f));
        window.draw(*m_instructionText);
    }
    
    if (m_hintText) {
        m_hintText->setCharacterSize(static_cast<unsigned int>(18 * scale));
        sf::FloatRect bounds = m_hintText->getLocalBounds();
        m_hintText->setPosition(sf::Vector2f(screenW / 2 - bounds.size.x / 2, screenH * 0.115f));
        window.draw(*m_hintText);
    }
    
    if (m_patternCounterText) {
        m_patternCounterText->setCharacterSize(static_cast<unsigned int>(24 * scale));
        m_patternCounterText->setPosition(sf::Vector2f(screenW - 120, screenH * 0.03f));
        window.draw(*m_patternCounterText);
    }
    
    if (m_closeText) {
        m_closeText->setCharacterSize(static_cast<unsigned int>(16 * scale));
        m_closeText->setPosition(sf::Vector2f(screenW - 150, screenH - 40));
        window.draw(*m_closeText);
    }
    
    // Dibujar etiquetas de las cuadrículas
    if (m_cachedTargetLabel) {
        window.draw(*m_cachedTargetLabel);
    }
    if (m_cachedPlayerLabel) {
        window.draw(*m_cachedPlayerLabel);
    }
    
    // Dibujar fondos de las cuadrículas
    window.draw(m_targetGridBackground);
    window.draw(m_playerGridBackground);
    
    // Dibujar celdas del patrón original (visibles u ocultas)
    if (m_isPatternVisible && !m_roundCompleted) {
        for (auto& cell : m_targetCells) {
            window.draw(cell);
        }
    } else if (!m_isPatternVisible && !m_roundCompleted) {
        for (auto& cell : m_targetCells) {
            sf::RectangleShape dark = cell;
            dark.setFillColor(sf::Color(30, 30, 30, 200));
            window.draw(dark);
        }
    }
    
    // Dibujar celdas del jugador
    for (auto& cell : m_playerCells) {
        window.draw(cell);
    }
    
    // Dibujar botón de verificar
    window.draw(m_verifyButton);
    if (m_verifyButtonText) {
        window.draw(*m_verifyButtonText);
    }
    
    // Dibujar bloques del inventario (no colocados)
    for (size_t i = 0; i < m_inventoryBlocks.size(); i++) {
        if (!m_availableBlocks[i].isPlaced) {
            window.draw(m_inventoryBlocks[i]);
        }
    }
    
    // Dibujar bloque arrastrado
    if (m_isDragging && m_selectedBlockIndex >= 0 && m_selectedBlockIndex < (int)m_inventoryBlocks.size()) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::RectangleShape dragged = m_inventoryBlocks[m_selectedBlockIndex];
        dragged.setPosition(sf::Vector2f(mousePos.x - m_dragOffset.x, mousePos.y - m_dragOffset.y));
        dragged.setOutlineColor(sf::Color::Yellow);
        dragged.setOutlineThickness(3);
        window.draw(dragged);
    }
    
    // Dibujar mensaje temporal
    if (m_messageText && !m_messageText->getString().isEmpty()) {
        m_messageText->setCharacterSize(static_cast<unsigned int>(28 * scale));
        sf::FloatRect bounds = m_messageText->getLocalBounds();
        m_messageText->setPosition(sf::Vector2f(screenW / 2 - bounds.size.x / 2, screenH * 0.18f));
        window.draw(*m_messageText);
    }
}

// Mostrar mensaje temporal
void MiniGameTetris::showTemporaryMessage(const std::string& msg, const sf::Color& color, float duration) {
    m_currentMessage = msg;
    m_messageColor = color;
    m_messageTimer = duration;
    if (m_messageText) {
        m_messageText->setString(msg);
        m_messageText->setFillColor(color);
    }
}

// Actualizar mensaje temporal
void MiniGameTetris::updateMessage(float dt) {
    if (m_messageTimer > 0.f) {
        m_messageTimer -= dt;
        if (m_messageTimer <= 0.f && m_messageText) {
            m_messageText->setString("");
        }
    }
}