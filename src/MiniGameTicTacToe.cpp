#include "MinigameTicTacToe.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

MinigameTicTacToe::MinigameTicTacToe()
    : m_isActive(false), m_gameWon(false), m_gameLost(false),
      m_playerTurn(true), m_board(9, 0), m_messageTimer(0.f)
{
    m_background.setFillColor(sf::Color(0, 0, 0, 200));
    m_background.setOutlineThickness(3.f);
    m_background.setOutlineColor(sf::Color(100, 100, 100));

    // Cargar texturas
    if (!m_bgTexture.loadFromFile("assets/images/niveles/nivel6/tictactoe_bg.png"))
    {
        std::cerr << "Error cargando tictactoe_bg.png" << std::endl;
    }
    if (!m_tableroTexture.loadFromFile("assets/images/niveles/nivel6/tablero.png"))
    {
        std::cerr << "Error cargando tablero.png" << std::endl;
    }
    if (!m_xTexture.loadFromFile("assets/images/niveles/nivel6/x.png"))
    {
        std::cerr << "Error cargando x.png" << std::endl;
    }
    if (!m_oTexture.loadFromFile("assets/images/niveles/nivel6/o.png"))
    {
        std::cerr << "Error cargando o.png" << std::endl;
    }
    if (!m_winLineTexture.loadFromFile("assets/images/niveles/nivel6/win_line.png"))
    {
        std::cerr << "Error cargando win_line.png" << std::endl;
    }
}

void MinigameTicTacToe::setPosition(const sf::Vector2f &pos)
{
    m_position = pos;
    m_background.setPosition(pos);
}

void MinigameTicTacToe::setSize(const sf::Vector2f &size)
{
    m_size = size;
    m_background.setSize(size);

    // Fondo
    if (m_bgTexture.getSize().x > 0)
    {
        m_bgSprite = std::make_unique<sf::Sprite>(m_bgTexture);

        // Escala fija (ajustable)
        float bgScale = 1.2f; // ← CAMBIA ESTE VALOR para tamaño del bg
        m_bgSprite->setScale(sf::Vector2f(bgScale, bgScale));

        sf::FloatRect bounds = m_bgSprite->getLocalBounds();
        m_bgSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));

        // POSICIÓN DEL BACKGROUND
        m_bgSprite->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f + 0.f, // ← + derecha, - izquierda
            m_position.y + m_size.y / 2.f - 20.f // ← - arriba, + abajo
            ));
    }

    // Tablero
    if (m_tableroTexture.getSize().x > 0)
    {
        m_tableroSprite = std::make_unique<sf::Sprite>(m_tableroTexture);

        float tableroScale = 1.5f; // ← CAMBIA ESTE VALOR
        m_tableroSprite->setScale(sf::Vector2f(tableroScale, tableroScale));

        sf::FloatRect bounds = m_tableroSprite->getLocalBounds();
        m_tableroSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));

        // POSICIÓN DEL TABLERO
        m_tableroSprite->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f + 0.f, // ← + derecha, - izquierda
            m_position.y + m_size.y / 2.f - 20.f // ← - arriba, + abajo
            ));
    }

    // Línea de victoria
    if (m_winLineTexture.getSize().x > 0)
    {
        m_winLineSprite = std::make_unique<sf::Sprite>(m_winLineTexture);
        sf::FloatRect bounds = m_winLineSprite->getLocalBounds();
        m_winLineSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_winLineSprite->setScale(sf::Vector2f(0.5f, 0.5f));
    }

    // Textos
    if (m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf"))
    {
        m_titleText = std::make_unique<sf::Text>(m_font);
        m_titleText->setString("TIC TAC TOE");
        m_titleText->setCharacterSize(28);
        m_titleText->setFillColor(sf::Color::Yellow);
        sf::FloatRect tb = m_titleText->getLocalBounds();
        m_titleText->setOrigin(sf::Vector2f(tb.size.x / 2.f, 0.f));
        m_titleText->setPosition(sf::Vector2f(m_position.x + m_size.x / 2.f, m_position.y + 10.f));

        m_turnText = std::make_unique<sf::Text>(m_font);
        m_turnText->setCharacterSize(20);
        m_turnText->setFillColor(sf::Color::White);

        m_closeText = std::make_unique<sf::Text>(m_font);
        m_closeText->setString("ESC para salir");
        m_closeText->setCharacterSize(14);
        m_closeText->setFillColor(sf::Color(150, 150, 150));
        m_closeText->setPosition(sf::Vector2f(m_position.x + m_size.x - 120.f, m_position.y + m_size.y - 30.f));

        m_resultText = std::make_unique<sf::Text>(m_font);
        m_resultText->setCharacterSize(36);
        m_resultText->setStyle(sf::Text::Bold);
    }
}

void MinigameTicTacToe::initBoard()
{
    m_board.assign(9, 0);
    m_boardSprites.clear();
    m_playerTurn = true;
    m_gameWon = false;
    m_gameLost = false;

    if (m_turnText)
        m_turnText->setString("Tu turno (X)");
    if (m_titleText)
    {
        m_titleText->setString("TIC TAC TOE");
        m_titleText->setFillColor(sf::Color::Yellow);
    }
}

void MinigameTicTacToe::activate()
{
    m_isActive = true;
    reset();
}

void MinigameTicTacToe::deactivate()
{
    m_isActive = false;
    m_boardSprites.clear();
}

void MinigameTicTacToe::reset()
{
    initBoard();
}

sf::Vector2f MinigameTicTacToe::getCellPosition(int cellIndex)
{
    int row = cellIndex / 3;
    int col = cellIndex % 3;

    float cellSize = 100.f;
    float startX = m_position.x + m_size.x / 2.f - cellSize * 1.5f;
    float startY = m_position.y + m_size.y / 2.f - cellSize * 1.5f - 20.f;

    return sf::Vector2f(startX + col * cellSize + cellSize / 2.f,
                        startY + row * cellSize + cellSize / 2.f);
}

int MinigameTicTacToe::getCellFromMouse(const sf::Vector2f &mousePos)
{
    float cellSize = 100.f;
    float startX = m_position.x + m_size.x / 2.f - cellSize * 1.5f;
    float startY = m_position.y + m_size.y / 2.f - cellSize * 1.5f - 20.f;

    for (int i = 0; i < 9; i++)
    {
        int row = i / 3;
        int col = i % 3;
        sf::FloatRect cell(sf::Vector2f(startX + col * cellSize, startY + row * cellSize),
                           sf::Vector2f(cellSize, cellSize));
        if (cell.contains(mousePos))
            return i;
    }
    return -1;
}

void MinigameTicTacToe::placeMark(int cellX, int cellY)
{
    int index = cellY * 3 + cellX;
    if (index < 0 || index >= 9 || m_board[index] != 0)
        return;

    int mark = m_playerTurn ? 1 : 2;
    m_board[index] = mark;

    // Crear sprite para la ficha
    auto sprite = std::make_unique<sf::Sprite>(m_playerTurn ? m_xTexture : m_oTexture);
    float fichaScale = 0.5f;
    sprite->setScale(sf::Vector2f(fichaScale, fichaScale));
    sf::FloatRect bounds = sprite->getLocalBounds();
    sprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));

    sf::Vector2f pos = getCellPosition(index);
    sprite->setPosition(pos);
    m_boardSprites.push_back(std::move(sprite));

    // Verificar ganador
    int winner = checkWinner();
    if (winner != 0)
    {
        if (winner == 1)
        {
            m_gameWon = true;
            if (m_titleText)
            {
                m_titleText->setString("¡GANASTE!");
                m_titleText->setFillColor(sf::Color::Green);
            }
            showWinLine(winner);
        }
        else
        {
            m_gameLost = true;
            if (m_titleText)
            {
                m_titleText->setString("PERDISTE...");
                m_titleText->setFillColor(sf::Color::Red);
            }
            showWinLine(winner);
        }
        return;
    }

    // Verificar empate
    bool empate = true;
    for (int v : m_board)
    {
        if (v == 0)
        {
            empate = false;
            break;
        }
    }
    if (empate)
    {
        // Mostrar mensaje de empate brevemente y reiniciar
        if (m_titleText)
        {
            m_titleText->setString("EMPATE - Reiniciando...");
            m_titleText->setFillColor(sf::Color(255, 165, 0));
        }
        // Reiniciar después de 1 segundo
        m_messageTimer = 1.0f;
        m_playerTurn = false; // Bloquear clicks durante el reinicio
        return;
    }

    m_playerTurn = !m_playerTurn;
    if (m_turnText)
    {
        m_turnText->setString(m_playerTurn ? "Tu turno (X)" : "Turno del joven (O)");
    }

    // Movimiento de la IA
    if (!m_playerTurn && !m_gameWon && !m_gameLost)
    {
        aiMove();
    }
}

void MinigameTicTacToe::aiMove()
{
    // IA simple: busca ganar o bloquear
    // 1. Intentar ganar
    for (int i = 0; i < 9; i++)
    {
        if (m_board[i] == 0)
        {
            m_board[i] = 2;
            if (checkWinner() == 2)
            {
                // Colocar ficha ganadora
                auto sprite = std::make_unique<sf::Sprite>(m_oTexture);
                float fichaScale = 0.5f;
                sprite->setScale(sf::Vector2f(fichaScale, fichaScale));
                sf::FloatRect bounds = sprite->getLocalBounds();
                sprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
                sf::Vector2f pos = getCellPosition(i);
                sprite->setPosition(pos);
                m_boardSprites.push_back(std::move(sprite));

                m_gameLost = true;
                if (m_titleText)
                {
                    m_titleText->setString("PERDISTE...");
                    m_titleText->setFillColor(sf::Color::Red);
                }
                showWinLine(2);
                return;
            }
            m_board[i] = 0;
        }
    }

    // 2. Bloquear al jugador
    for (int i = 0; i < 9; i++)
    {
        if (m_board[i] == 0)
        {
            m_board[i] = 1;
            if (checkWinner() == 1)
            {
                m_board[i] = 2;
                auto sprite = std::make_unique<sf::Sprite>(m_oTexture);
                sprite->setScale(sf::Vector2f(0.35f, 0.35f));
                sf::FloatRect bounds = sprite->getLocalBounds();
                sprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
                sf::Vector2f pos = getCellPosition(i);
                sprite->setPosition(pos);
                m_boardSprites.push_back(std::move(sprite));

                m_playerTurn = true;
                if (m_turnText)
                    m_turnText->setString("Tu turno (X)");
                return;
            }
            m_board[i] = 0;
        }
    }

    // 3. Centro o esquina aleatoria
    std::vector<int> preferencias = {4, 0, 2, 6, 8, 1, 3, 5, 7};
    for (int idx : preferencias)
    {
        if (m_board[idx] == 0)
        {
            m_board[idx] = 2;
            auto sprite = std::make_unique<sf::Sprite>(m_oTexture);
            sprite->setScale(sf::Vector2f(0.35f, 0.35f));
            sf::FloatRect bounds = sprite->getLocalBounds();
            sprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
            sf::Vector2f pos = getCellPosition(idx);
            sprite->setPosition(pos);
            m_boardSprites.push_back(std::move(sprite));

            m_playerTurn = true;
            if (m_turnText)
                m_turnText->setString("Tu turno (X)");
            return;
        }
    }
}

int MinigameTicTacToe::checkWinner()
{
    // Filas, columnas, diagonales
    int lines[8][3] = {
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, // Filas
        {0, 3, 6},
        {1, 4, 7},
        {2, 5, 8}, // Columnas
        {0, 4, 8},
        {2, 4, 6} // Diagonales
    };

    for (auto &line : lines)
    {
        if (m_board[line[0]] != 0 &&
            m_board[line[0]] == m_board[line[1]] &&
            m_board[line[1]] == m_board[line[2]])
        {
            return m_board[line[0]];
        }
    }
    return 0;
}

void MinigameTicTacToe::showWinLine(int winner)
{
    if (!m_winLineSprite)
        return;

    // La imagen de línea es VERTICAL, calculamos posición y rotación
    int lines[8][3] = {
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};

    for (auto &line : lines)
    {
        if (m_board[line[0]] == winner &&
            m_board[line[0]] == m_board[line[1]] &&
            m_board[line[1]] == m_board[line[2]])
        {

            // Posición central de la línea ganadora
            sf::Vector2f start = getCellPosition(line[0]);
            sf::Vector2f end = getCellPosition(line[2]);
            sf::Vector2f center = sf::Vector2f((start.x + end.x) / 2.f, (start.y + end.y) / 2.f);

            m_winLineSprite->setPosition(center);

            // Calcular ángulo
            float dx = end.x - start.x;
            float dy = end.y - start.y;
            float angle = std::atan2(dy, dx) * 180.f / 3.14159f + 90.f;
            m_winLineSprite->setRotation(sf::degrees(angle));

            break;
        }
    }
}

void MinigameTicTacToe::handleEvent(const sf::Event &event, const sf::RenderWindow &window)
{
    if (!m_isActive || m_gameWon || m_gameLost)
        return;
    if (!m_playerTurn)
        return;

    if (event.is<sf::Event::MouseButtonPressed>())
    {
        const auto &mouseEvent = event.getIf<sf::Event::MouseButtonPressed>();
        if (mouseEvent->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mousePos = window.mapPixelToCoords(
                sf::Mouse::getPosition(window), window.getDefaultView());

            int cell = getCellFromMouse(mousePos);
            if (cell >= 0 && m_board[cell] == 0)
            {
                int cellX = cell % 3;
                int cellY = cell / 3;
                placeMark(cellX, cellY);
            }
        }
    }
}

void MinigameTicTacToe::update(float dt)
{
    if (!m_isActive)
        return;

    // Reinicio por empate
    if (m_messageTimer > 0.f && !m_gameWon && !m_gameLost)
    {
        m_messageTimer -= dt;
        if (m_messageTimer <= 0.f)
        {
            // Reiniciar el tablero
            initBoard();
            std::cout << "Empate - Tablero reiniciado" << std::endl;
        }
    }
}

void MinigameTicTacToe::draw(sf::RenderWindow &window)
{
    if (!m_isActive)
        return;

    window.draw(m_background);
    if (m_bgSprite)
        window.draw(*m_bgSprite);
    if (m_tableroSprite)
        window.draw(*m_tableroSprite);

    // Dibujar fichas
    for (auto &sprite : m_boardSprites)
    {
        window.draw(*sprite);
    }

    // Línea de victoria
    if ((m_gameWon || m_gameLost) && m_winLineSprite)
    {
        window.draw(*m_winLineSprite);
    }

    if (m_titleText)
        window.draw(*m_titleText);
    if (m_turnText)
    {
        sf::FloatRect tb = m_turnText->getLocalBounds();
        m_turnText->setOrigin(sf::Vector2f(tb.size.x / 2.f, 0.f));
        m_turnText->setPosition(sf::Vector2f(m_position.x + m_size.x / 2.f, m_position.y + 45.f));
        window.draw(*m_turnText);
    }
    if (m_closeText)
        window.draw(*m_closeText);
}