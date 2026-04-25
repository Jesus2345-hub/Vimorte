#include "Game.hpp"
#include "MenuState.hpp"
#include "State.hpp"
#include "Nivel1State.hpp"
#include "Lobby.hpp"
#include "AdminMenuState.hpp"
#include <optional>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

Game::Game()
{
    window = std::make_unique<sf::RenderWindow>(sf::VideoMode({1280, 720}), "Vimorte", sf::Style::Titlebar | sf::Style::Close);
    window->setFramerateLimit(60);

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

        std::cout << "✅ Configuración de audio cargada" << std::endl;
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

    std::cout << "✅ Configuración de audio guardada" << std::endl;
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
        std::cout << "🎵 Música cambiada: " << rutaMusica << std::endl;
    }
    else
    {
        std::cerr << "❌ Error: No se pudo cargar la música: " << rutaMusica << std::endl;
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
        std::cout << "💾 Partida guardada en slot " << saveManager.getCurrentSlotId() << std::endl;
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
                std::cout << "💾 Checkpoint guardado antes del centinela" << std::endl;
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

        std::cout << "📂 Cargando partida del slot " << slotId << " - "
                  << levelTree.getCurrentNodeInfo() << std::endl;

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

            // ===== NUEVO: Alternar entre ventana normal y maximizada con F11 =====
            if (const auto *keyEvent = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyEvent->code == sf::Keyboard::Key::F11)
                {
                    // Alternar entre normal y maximizado
                    static bool isMaximized = false;
                    isMaximized = !isMaximized;

                    if (isMaximized)
                    {
                        // Obtener resolución del monitor actual
                        sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
                        window->setSize(sf::Vector2u(desktopMode.size.x, desktopMode.size.y));
                        window->setPosition(sf::Vector2i(0, 0));
                    }
                    else
                    {
                        window->setSize(sf::Vector2u(1280, 720));
                        // Centrar la ventana
                        sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
                        window->setPosition(sf::Vector2i(
                            (desktopMode.size.x - 1280) / 2,
                            (desktopMode.size.y - 720) / 2));
                    }
                }
            }

            if (!states.empty())
            {
                states.top()->handleEvent(*event);
            }
        }

        if (!states.empty())
        {
            states.top()->update(deltaTime);
        }

        window->clear(sf::Color::Black);

        if (!states.empty())
        {
            // Dibujar estados en orden inverso (primero el fondo)
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
            // Dibujar desde el fondo hasta arriba
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
    // Vaciar completamente la pila
    while (!states.empty())
    {
        states.pop();
    }
    states.push(std::move(state));
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
    std::cout << "🏠 Volviendo al menú principal..." << std::endl;

    // Detener música
    detenerMusica();

    // IMPORTANTE: Vaciar la pila de estados COMPLETAMENTE
    while (!states.empty())
    {
        states.pop();
    }

    // Limpiar el árbol de niveles pero NO reconstruirlo completamente
    // Simplemente reiniciamos el puntero actual a la raíz
    // Esto evita problemas con los stateFactories
    levelTree.resetToRoot(); // Necesitamos añadir este método a LevelTree

    // Limpiar el slot activo para que no haya conflictos
    // Nota: No borramos el saveManager, solo desactivamos la partida activa
    // El saveManager tiene su propio currentSlotId que podemos mantener

    // Crear NUEVO menú
    auto menuState = std::make_unique<MenuState>(window.get(), this);
    states.push(std::move(menuState));

    std::cout << "✅ Menú principal cargado correctamente" << std::endl;
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
            std::cout << "🔄 Reintentando desde el checkpoint antes del centinela" << std::endl;
        }
    }
    else
    {
        std::cout << "💀 Has fallado. Las consecuencias son permanentes." << std::endl;
        returnToMenu();
    }
}