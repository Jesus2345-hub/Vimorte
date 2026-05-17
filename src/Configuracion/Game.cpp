#include "Configuracion/Game.hpp"
#include "Configuracion/Config.hpp"
#include "Estados/MenuState.hpp"
#include "Estados/State.hpp"
#include "Estados/Niveles/Nivel2/Nivel2State.hpp"  
#include "Administradores/Lobby.hpp"
#include "Estados/AdminMenuState.hpp"
#include <optional>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

Game::Game()
{
    // Cargar configuración de pantalla ANTES de crear la ventana
    Config::cargar();

    if (Config::isPantallaCompleta())
    {
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        window = std::make_unique<sf::RenderWindow>(
            sf::VideoMode({desktop.size.x, desktop.size.y}),
            "Vimorte",
            sf::State::Fullscreen);
    }
    else
    {
        window = std::make_unique<sf::RenderWindow>(
            sf::VideoMode({1280, 720}),
            "Vimorte",
            sf::Style::Titlebar | sf::Style::Close);
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        window->setPosition(sf::Vector2i(
            (desktop.size.x - 1280) / 2,
            (desktop.size.y - 720) / 2));
    }

    sf::Vector2u winSize = window->getSize();
    window->setView(sf::View(sf::FloatRect(
        sf::Vector2f(0.f, 0.f),
        sf::Vector2f(static_cast<float>(winSize.x), static_cast<float>(winSize.y)))));

    // Cargar configuración de audio guardada
    cargarConfiguracionAudio();

    // Construir árbol de niveles
    levelTree.buildTree();

    // Crear menú principal
    states.push(std::make_unique<MenuState>(window.get(), this));
}

Game::~Game()
{
    guardarConfiguracionAudio();
    detenerMusica();

    // Limpiar estados antes de destruir
    while (!states.empty())
    {
        states.pop();
    }
}

void Game::cargarConfiguracionAudio()
{
    std::ifstream file("saves/config.csv");
    if (!file.is_open())
        return;

    std::string linea;
    std::getline(file, linea);

    if (std::getline(file, linea))
    {
        std::stringstream ss(linea);
        std::string token;

        std::getline(ss, token, ',');
        volGeneral = std::stof(token);
        std::getline(ss, token, ',');
        volMusica = std::stof(token);
        std::getline(ss, token, ',');
        volEfectos = std::stof(token);
    }
}

void Game::guardarConfiguracionAudio()
{
#ifdef _WIN32
    system("mkdir saves 2> nul");
#else
    system("mkdir -p saves");
#endif

    std::ofstream file("saves/config.csv");
    if (!file.is_open())
        return;

    file << "vol_general,vol_musica,vol_efectos\n";
    file << volGeneral << "," << volMusica << "," << volEfectos << "\n";
}

void Game::cambiarMusica(const std::string &rutaMusica)
{
    if (m_currentMusicPath == rutaMusica && m_currentMusic.getStatus() == sf::Music::Status::Playing)
    {
        return;
    }

    m_currentMusic.stop();

    if (m_currentMusic.openFromFile(rutaMusica))
    {
        m_currentMusicPath = rutaMusica;
        m_currentMusic.setLooping(true);
        m_currentMusic.setVolume(getRealMusica());
        m_currentMusic.play();
    }
    else
    {
        std::cerr << " Error: No se pudo cargar la música: " << rutaMusica << std::endl;
    }
}

void Game::detenerMusica()
{
    m_currentMusic.stop();
    m_currentMusicPath = "";
}

void Game::setVolGeneral(float v)
{
    if (v < 0.f)
        v = 0.f;
    if (v > 100.f)
        v = 100.f;
    volGeneral = v;
    actualizarVolumenMusica();
    guardarConfiguracionAudio();
}

void Game::setVolMusica(float v)
{
    if (v < 0.f)
        v = 0.f;
    if (v > 100.f)
        v = 100.f;
    volMusica = v;
    actualizarVolumenMusica();
    guardarConfiguracionAudio();
}

void Game::setVolEfectos(float v)
{
    if (v < 0.f)
        v = 0.f;
    if (v > 100.f)
        v = 100.f;
    volEfectos = v;
    guardarConfiguracionAudio();
}

void Game::actualizarVolumenMusica()
{
    m_currentMusic.setVolume(getRealMusica());
}

void Game::guardarPartidaActual()
{
    if (tienePartidaActiva())
    {
        saveManager.getCurrentProgress().rutaActual = levelTree.getCurrentPath();
        saveManager.guardarProgresoActual();
    }
}

void Game::avanzarNivel()
{
    if (levelTree.goToNextLevel())
    {
        auto newState = levelTree.createCurrentState(window.get(), this);
        if (newState)
        {
            changeState(std::move(newState));

            if (tienePartidaActiva())
            {
                LevelNode *current = levelTree.getCurrentNode();
                saveManager.setNivelActual(current->levelNumber, current->levelNumber);
                saveManager.getCurrentProgress().rutaActual = levelTree.getCurrentPath();
                guardarPartidaActual();
            }
        }
    }
}

void Game::entrarCentinela()
{
    if (levelTree.goToCentinela())
    {
        if (tienePartidaActiva())
        {
            GameProgressData &progress = saveManager.getCurrentProgress();
            if (progress.modoElegido == GameProgressData::ModoJuego::CAMINO_AGRADABLE)
            {
                progress.tieneCheckpointCentinela = true;
                progress.checkpointRutaArbol = levelTree.getCurrentPath();
                progress.checkpointCentinelaId = levelTree.getCurrentNode()->id;
                guardarPartidaActual();
            }
        }

        auto newState = levelTree.createCurrentState(window.get(), this);
        if (newState)
        {
            changeState(std::move(newState));

            if (tienePartidaActiva())
            {
                saveManager.getCurrentProgress().rutaActual = levelTree.getCurrentPath();
                guardarPartidaActual();
            }
        }
    }
}

void Game::volverDeCentinela()
{
    if (levelTree.returnFromCentinela())
    {
        auto newState = levelTree.createCurrentState(window.get(), this);
        if (newState)
        {
            changeState(std::move(newState));

            if (tienePartidaActiva())
            {
                saveManager.getCurrentProgress().rutaActual = levelTree.getCurrentPath();
                guardarPartidaActual();
            }
        }
    }
}

void Game::cargarPartidaYContinuar(int slotId)
{
    if (saveManager.cargarPartida(slotId))
    {
        std::string rutaGuardada = saveManager.getCurrentProgress().rutaActual;
        if (!rutaGuardada.empty() && rutaGuardada != "principal")
        {
            levelTree.restorePath(rutaGuardada);
        }
        detenerMusica();

        auto newState = levelTree.createCurrentState(window.get(), this);
        if (newState)
        {
            changeState(std::move(newState));
        }
        else
        {
            changeState(std::make_unique<LobbyState>(window.get(), this));
        }
    }
}

void Game::run()
{
    sf::Clock clock;
    while (window->isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        if (tienePartidaActiva())
        {
            saveManager.addTiempoJugado(deltaTime);
        }

        while (const std::optional<sf::Event> event = window->pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                if (tienePartidaActiva())
                {
                    guardarPartidaActual();
                }
                window->close();
            }

            // ===== Alternar entre ventana normal y maximizada con F11 =====
            if (const auto *keyEvent = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyEvent->code == sf::Keyboard::Key::F11)
                {
                    if (tienePartidaActiva())
                    {
                        guardarPartidaActual();
                    }

                    Config::alternarPantalla(window.get());

                    sf::Vector2u newSize = window->getSize();
                    window->setView(sf::View(sf::FloatRect(
                        sf::Vector2f(0.f, 0.f),
                        sf::Vector2f(static_cast<float>(newSize.x), static_cast<float>(newSize.y)))));

                    // Notificar al MenuState que redimensione (si es el estado actual)
                    if (!states.empty())
                    {
                        if (auto *menuState = dynamic_cast<MenuState *>(states.top().get()))
                        {
                            menuState->redimensionar(static_cast<float>(newSize.x), static_cast<float>(newSize.y));
                        }
                    }
                }
            }

            if (!states.empty())
            {
                states.top()->handleEvent(*event);
            }
        }
        
        // Update
        if (!states.empty())
        {
            states.top()->update(deltaTime);
        }
        // Procesar cambio de estado pendiente (fuera del update)
        if (m_pendingStateChange)
        {
            while (!states.empty())
                states.pop();
            states.push(std::move(m_pendingState));
            m_pendingStateChange = false;
        }

        // Draw
        window->clear(sf::Color::Black);

        if (!states.empty())
        {
            std::vector<State *> paraDibujar;
            std::stack<std::unique_ptr<State>> temp;

            while (!states.empty())
            {
                paraDibujar.push_back(states.top().get());
                temp.push(std::move(states.top()));
                states.pop();
            }
            while (!temp.empty())
            {
                states.push(std::move(temp.top()));
                temp.pop();
            }

            for (int i = paraDibujar.size() - 1; i >= 0; --i)
            {
                window->setView(window->getDefaultView());
                paraDibujar[i]->draw();
            }
        }
        window->display();
    }
}

void Game::changeState(std::unique_ptr<State> state)
{
    // En lugar de cambiar inmediatamente, guardar para el siguiente frame
    m_pendingState = std::move(state);
    m_pendingStateChange = true;
}

void Game::pushState(std::unique_ptr<State> state)
{
    states.push(std::move(state));
}

void Game::popState()
{
    if (!states.empty())
    {
        states.pop();
    }
}

void Game::returnToMenu()
{
    detenerMusica();
    levelTree.resetToRoot();
    m_isInLevel = false;

    // Programar cambio de estado para el siguiente frame
    m_pendingState = std::make_unique<MenuState>(window.get(), this);
    m_pendingStateChange = true;
}

void Game::setPantallaCompleta(bool fullscreen)
{
    Config::setPantallaCompleta(fullscreen);
    Config::alternarPantalla(window.get());
}

void Game::aplicarConfiguracionPantalla()
{
    Config::alternarPantalla(window.get());
}

void Game::reintentarCentinela()
{
    GameProgressData &progress = saveManager.getCurrentProgress();

    if (progress.modoElegido == GameProgressData::ModoJuego::CAMINO_AGRADABLE &&
        progress.tieneCheckpointCentinela)
    {
        levelTree.restorePath(progress.checkpointRutaArbol);

        progress.tieneCheckpointCentinela = false;
        progress.checkpointRutaArbol = "";
        progress.checkpointCentinelaId = "";

        auto newState = levelTree.createCurrentState(window.get(), this);
        if (newState)
        {
            changeState(std::move(newState));
        }
    }
    else
    {
        returnToMenu();
    }
}
void Game::completarNivelActual(int siguienteNivelId) {
    if (tienePartidaActiva()) {
        saveManager.setNivelActual(siguienteNivelId, siguienteNivelId);
        guardarPartidaActual();
    }
    
    // Intentar avanzar normalmente
    if (!levelTree.goToNextLevel()) {
        // Si falla, forzar salto al siguiente nivel
        std::string siguienteNivel = "nivel" + std::to_string(siguienteNivelId);
        levelTree.jumpToNode(siguienteNivel);
    }
    
    auto newState = levelTree.createCurrentState(window.get(), this);
    if (newState) {
        changeState(std::move(newState));
    }
}

void Game::irACentinela() {
    LevelTree& arbol = getLevelTree();
    if (arbol.goToCentinela()) {
        std::unique_ptr<State> nuevoEstado = arbol.createCurrentState(window.get(), this);
        if (nuevoEstado) {
            changeState(std::move(nuevoEstado));
        }
    }
}