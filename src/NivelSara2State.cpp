#include "NivelSara2State.hpp"
#include "PauseState.hpp"
#include "MuerteCentinelaState.hpp"
#include "CoordenadasDebug.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>


// CONSTRUCTOR

NivelSara2State::NivelSara2State(sf::RenderWindow *window, Game *game)
    : State(window, game),
      m_background(nullptr),
      m_casoResuelto(false),
      m_nivelCompletado(false),
      m_setActualCaso(-1),
      m_textoInteraccion(nullptr),
      m_mostrarPuertaSalida(true),
      m_cercaPuertaSalida(false),
      m_escapeConsumed(false),
      m_mostrarTutorial(false),
      m_mostrarTutorialPorTecla(false),
      m_msjActual(),
      m_fontLoaded(false),
      m_tiempoFlotante(0.0f) ,
      m_mensajeFlotante(nullptr)
{
    m_cercaBloqueInteractivo = false;
    m_mensajeEmergenteActivo = false;
    m_bloqueActualIndex = -1;
    m_msjActual.texto = "";
    m_msjActual.tiempoRestante = 0.0f;
    m_msjActual.color = sf::Color::Yellow;

    m_player.loadAssets();
    m_player.setPosition(315.f, 435.f);
    m_player.setSpeed(300.0f);

    // Verificar si es la primera vez para mostrar el tutorial
    if (game->tienePartidaActiva())
    {
        const auto &items = game->getSaveManager().getCurrentProgress().itemsRecolectados;
        auto it = std::find(items.begin(), items.end(), "TutorialSara2Visto");

        if (it == items.end())
        {
            m_mostrarTutorial = true;
            game->getSaveManager().addItemRecolectado("TutorialSara2Visto");
            std::cout << "Primer ingreso a NivelSara2: Mostrando tutorial" << std::endl;
        }
    }

    if (m_backgroundTexture.loadFromFile("assets/images/niveles/nivel_sara2/background.png"))
    {
        m_background = std::make_unique<sf::Sprite>(m_backgroundTexture);
        sf::Vector2u textureSize = m_backgroundTexture.getSize();
        m_worldSize = sf::Vector2f(static_cast<float>(textureSize.x),
                                   static_cast<float>(textureSize.y));
        std::cout << "NivelSara2 cargado. Tamaño: " << m_worldSize.x << "x" << m_worldSize.y << std::endl;
    }
    else
    {
        std::cerr << "Error: No se pudo cargar background.jpg para NivelSara2" << std::endl;
        m_worldSize = sf::Vector2f(1754.f, 1587.f);
    }

    sf::Vector2u windowSize = window->getSize();

    // Cámara con tamaño
    float fixedWidth = 1280.f;
    float fixedHeight = 720.f;
    m_camera = sf::View(
        sf::Vector2f(m_worldSize.x / 2.f, m_worldSize.y / 2.f),
        sf::Vector2f(fixedWidth, fixedHeight));
    m_lastWindowSize = windowSize;

    // Áreas de interacción
    m_puertaSalidaArea = sf::FloatRect(sf::Vector2f(1550.f, 1350.f), sf::Vector2f(120.f, 180.f));

    configurarColisiones();
    configurarMinijuegoCriminal();
    configurarBloquesInteractivos();
    
    // Carga de fuente
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    if (!m_fontLoaded)
    {
        std::cerr << "ERROR en NivelSara2State: No se pudo cargar la fuente" << std::endl;
    }

    if (m_fontLoaded)
    {
        m_textoInteraccion = std::make_unique<sf::Text>(m_font);
        m_textoInteraccion->setString("Interacción");
        m_textoInteraccion->setCharacterSize(20);
        m_textoInteraccion->setFillColor(sf::Color::White);
        m_textoInteraccion->setPosition(sf::Vector2f(windowSize.x / 2.f, windowSize.y - 70.f));

        sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
        m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));

        m_textoMensaje = std::make_unique<sf::Text>(m_font);
        m_textoMensaje->setCharacterSize(28);  
        m_textoMensaje->setFillColor(sf::Color::Yellow);
        m_textoMensaje->setOutlineColor(sf::Color::Black); 
        m_textoMensaje->setOutlineThickness(2.0f);
        m_textoMensaje->setStyle(sf::Text::Bold);
        
        
        // INICIALIZAR MENSAJE FLOTANTE
        
        m_mensajeFlotante = std::make_unique<sf::Text>(m_font);
        m_mensajeFlotante->setCharacterSize(24);
        m_mensajeFlotante->setFillColor(sf::Color::Red);
        m_mensajeFlotante->setOutlineColor(sf::Color::Black);
        m_mensajeFlotante->setOutlineThickness(2.5f);
        m_mensajeFlotante->setStyle(sf::Text::Bold);
    }
    else
    {
        m_textoInteraccion = nullptr;
        m_textoMensaje = nullptr;
    }

    // Guardado automático
    if (game->tienePartidaActiva())
    {
        game->getSaveManager().setNivelActual(5, 1);
        game->guardarPartidaActual();
        std::cout << "Partida guardada automáticamente en NivelSara2" << std::endl;
    }

    std::cout << "NivelSara2State inicializado correctamente" << std::endl;
    game->setIsInLevel(true);
    CoordenadasDebug::getInstance().setVisible(true);
    
    m_criminalMinigame.setOnCompleteCallback([this](bool exito) {
        if (exito && !m_criminalGameCompleted) 
        {
            m_criminalGameCompleted = true;
            std::cout << "CASO RESUELTO! Set actual: " << m_setActualCaso << std::endl;
            mostrarMensaje("CASO RESUELTO. Has encontrado todas las pistas y al culpable.\nEntregale las cosas a Andrea", 5.0f);
        }
    });

    m_criminalMinigame.setDebugMode(true);
    
  
}


// MOSTRAR MENSAJE FLOTANTE

void NivelSara2State::mostrarMensajeFlotante(const std::string& texto, float duracion, sf::Color color)
{
    if (!m_fontLoaded) return;
    
    m_mensajeFlotante->setString(texto);
    m_mensajeFlotante->setFillColor(color);
    
    // Centrar el texto
    sf::FloatRect bounds = m_mensajeFlotante->getLocalBounds();
    m_mensajeFlotante->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
    
    m_tiempoFlotante = duracion;
    m_clockFlotante.restart();
    
    std::cout << "MENSAJE FLOTANTE: " << texto << std::endl;
}


// REAJUSTAR MINIJUEGO (MANTENIENDO ESTADO)

void NivelSara2State::reajustarMinijuegoCriminalManteniendoEstado()
{
    if (!m_criminalMinigame.isActive()) return;
    
    sf::Vector2u windowSize = window->getSize();
    float minijuegoW = windowSize.x * 0.85f;
    float minijuegoH = windowSize.y * 0.85f;
    float minijuegoX = (windowSize.x - minijuegoW) / 2.f;
    float minijuegoY = (windowSize.y - minijuegoH) / 2.f;
    
    m_criminalMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
    m_criminalMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
    
    m_criminalMinigame.cargarFondoOnly("assets/images/niveles/nivel_sara2/criminalCase.png");
}


// CONFIGURAR BLOQUES INTERACTIVOS

void NivelSara2State::configurarBloquesInteractivos()
{
    m_bloquesInteractivos.clear();
    
    m_bloquesInteractivos.push_back({
        sf::FloatRect(sf::Vector2f(280.f, 904.f), sf::Vector2f(100.f, 100.f)),
        "Andrea esta desesperada.\nSus joyas mas preciosas fueron robadas...\nAndrea : Ayudame a encontrar al culpable.\n\n[Presiona R para entregar los objetos si has resuelto el caso]"
    });
}


// MANEJAR EVENTOS

void NivelSara2State::handleEvent(const sf::Event &event)
{
    // Prioridad: si hay mensaje emergente
    if (m_mensajeEmergenteActivo) {
        if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            
            // ESC: Cerrar mensaje
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                m_mensajeEmergenteActivo = false;
                m_bloqueActualIndex = -1;
                return;
            }
            
            // Tecla R: entregar objetos
            if (keyPressed->code == sf::Keyboard::Key::R && !m_nivelCompletado) {
    
    if (m_criminalGameCompleted) {
        
        Inventory* inv = m_player.getInventory();
        bool tieneTodosLosObjetos = true;
        
        // Usamos m_setActualCaso para saber qué caso se completó
        int casoCompletado = m_setActualCaso;
        
        if (casoCompletado >= 0 && casoCompletado < (int)m_todosLosObjetos.size()) {
            // Verificar que el jugador tenga TODOS los objetos de ESE caso
            for (const auto& objRequerido : m_todosLosObjetos[casoCompletado]) {
                bool encontrado = false;
                if (inv) {
                    for (int i = 0; i < 20; i++) {
                        Item* item = inv->getItem(i);
                        if (item && item->name == objRequerido.nombre) {
                            encontrado = true;
                            break;
                        }
                    }
                }
                if (!encontrado) {
                    tieneTodosLosObjetos = false;
                    break;  
                }
            }
        } else 
        {
            for (const auto& objRequerido : m_objetosCriminal) {
                bool encontrado = false;
                if (inv) {
                    for (int i = 0; i < 20; i++) {
                        Item* item = inv->getItem(i);
                        if (item && item->name == objRequerido.nombre) {
                            encontrado = true;
                            break;
                        }
                    }
                }
                if (!encontrado) {
                    tieneTodosLosObjetos = false;
                    break;
                }
            }
        }
        
        if (tieneTodosLosObjetos) 
        {
            // Éxito: el jugador tiene los objetos correctos
            m_casoResuelto = true;
            m_nivelCompletado = true;
            
            // LIMPIAR INVENTARIO - los objetos han sido entregados
            Inventory* inv = m_player.getInventory();
            if (inv) {
                // Limpiar solo los objetos del caso actual
                int casoActual = m_setActualCaso;
                if (casoActual >= 0 && casoActual < (int)m_todosLosObjetos.size()) {
                    for (const auto& objRequerido : m_todosLosObjetos[casoActual]) {
                        for (int i = 0; i < 20; i++) {
                            Item* item = inv->getItem(i);
                            if (item && item->name == objRequerido.nombre) {
                                inv->removeItem(i);
                                std::cout << "Objeto entregado y eliminado: " << objRequerido.nombre << std::endl;
                                break;
                            }
                        }
                    }
                }
            }
            
            m_bloquesInteractivos[m_bloqueActualIndex].mensaje = 
                "GRACIAS! Has recuperado todas mis joyas.\n"
                "Eres un heroe...\n\n"
                "Ahora dirigete al ASCENSOR\n"
                "y presiona E para avanzar al siguiente nivel.";
            
            std::cout << "OBJETOS ENTREGADOS CORRECTAMENTE! Nivel completado." << std::endl;
        } else {
            // El jugador NO tiene todos los objetos - solo mostrar mensaje flotante
            m_mensajeEmergenteActivo = false;
            mostrarMensajeFlotante("Aun no has encontrado todas las joyas. Sigue investigando\n en la escena del crimen.", 3.0f, sf::Color::Yellow);
        }
           
        } else {
            // Caso no completado
            m_mensajeEmergenteActivo = false;
            mostrarMensajeFlotante("Aun no has resuelto el caso.\nInvestiga la escena del crimen y encuentra\ntodas las pistas y al culpable.", 3.0f, sf::Color::Yellow);
            return;
        }
    }
        }
        return;
    }
    
    // Teclas globales (solo si NO hay mensaje activo)
    if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->code == sf::Keyboard::Key::Escape)
        {
            if (m_mostrarTutorial || m_mostrarTutorialPorTecla)
            {
                m_mostrarTutorial = false;
                m_mostrarTutorialPorTecla = false;
                return;
            }
        }

        if (keyPressed->code == sf::Keyboard::Key::M)
        {
            std::cout << "M presionada - Activando tutorial" << std::endl;
            if (game->tienePartidaActiva())
            {
                const auto &items = game->getSaveManager().getCurrentProgress().itemsRecolectados;
                auto it = std::find(items.begin(), items.end(), "TutorialSara2Visto");
                if (it != items.end())
                {
                    m_mostrarTutorialPorTecla = true;
                }
                else
                {
                    m_mostrarTutorial = true;
                }
            }
            else
            {
                m_mostrarTutorialPorTecla = true;
            }
        }
    }

    Inventory *inv = m_player.getInventory();
    if (inv)
    {
        inv->handleEvent(event, *window);
    }
    
    // Manejar eventos del minijuego criminal
    if (m_criminalMinigame.isActive()) {
        m_criminalMinigame.handleEvent(event, *window);
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                m_criminalMinigame.deactivate();
                return;
            }
        }
        return;
    }
}


// CONFIGURAR MINIJUEGO CRIMINAL

void NivelSara2State::configurarMinijuegoCriminal()
{
    m_criminalArea = sf::FloatRect(sf::Vector2f(800.f, 600.f), sf::Vector2f(150.f, 150.f));
    m_cercaCriminalArea = false;
    m_criminalGameCompleted = false;
    
    m_criminalMinigame.limpiarPools();
    
    m_todosLosObjetos.clear();
    m_todosLosSospechosos.clear();
    m_todosLosDialogos.clear();
    
    // Bloque 1
    std::vector<ObjetoBuscar> objetosBloque1;
    objetosBloque1.emplace_back("Collar", sf::FloatRect(sf::Vector2f(560.f, 426.f), sf::Vector2f(36.f, 40.f))," ");
    objetosBloque1.emplace_back("Carta Mojada", sf::FloatRect(sf::Vector2f(191.f, 438.f), sf::Vector2f(75.f, 38.f))," ");
    objetosBloque1.emplace_back("Reloj Arena", sf::FloatRect(sf::Vector2f(665.f, 357.f), sf::Vector2f(25.f, 70.f))," ");
    objetosBloque1.emplace_back("Medalla", sf::FloatRect(sf::Vector2f(116.f, 164.f), sf::Vector2f(20.f, 23.f))," ");
    objetosBloque1.emplace_back("Botella", sf::FloatRect(sf::Vector2f(104.f, 359.f), sf::Vector2f(25.f, 24.f))," ");
    objetosBloque1.emplace_back("Diario", sf::FloatRect(sf::Vector2f(524.f, 496.f), sf::Vector2f(35.f, 30.f))," ");
    objetosBloque1.emplace_back("Anillo", sf::FloatRect(sf::Vector2f(504.f, 414.f), sf::Vector2f(21.f, 25.f))," ");
    objetosBloque1.emplace_back("Foto", sf::FloatRect(sf::Vector2f(744.f, 240.f), sf::Vector2f(34.f, 54.f))," ");
    objetosBloque1.emplace_back("Cuchillo", sf::FloatRect(sf::Vector2f(348.f, 370.f), sf::Vector2f(36.f, 25.f))," ");
    objetosBloque1.emplace_back("Bolso", sf::FloatRect(sf::Vector2f(408.f, 473.f), sf::Vector2f(109.f, 80.f))," ");
    objetosBloque1.emplace_back("Trapo Viejo", sf::FloatRect(sf::Vector2f(766.f, 455.f), sf::Vector2f(26.f, 15.f))," ");
    
    std::vector<Sospechoso> sospechososBloque1;
    sospechososBloque1.emplace_back("Capitan Rodrigo", sf::FloatRect(sf::Vector2f(300.f, 650.f), sf::Vector2f(80.f, 100.f)), "El capitan del barco. Tenia acceso a todo,\npero parece honesto.", false);
    sospechososBloque1.emplace_back("Isabella la Adivina", sf::FloatRect(sf::Vector2f(750.f, 620.f), sf::Vector2f(70.f, 90.f)), "Siempre supo que algo pasaria", true);
    sospechososBloque1.emplace_back("Don Julio el Pescador", sf::FloatRect(sf::Vector2f(550.f, 680.f), sf::Vector2f(80.f, 90.f)), "Vio todo desde su bote,\npero jura que no fue el.", false);
    
    std::vector<DialogoNarrativo> dialogosBloque1;
    DialogoNarrativo dialogoDonJulio("Don Julio el Pescador", 
        "Vi todo desde mi bote esa noche.\nEl barco estaba tranquilo,\n pero vi a alguien moviendose\nsigilosamente cerca\nde las pertenencias de Andrea.",
        "assets/images/niveles/nivel_sara2/JulioPescador.png");
    dialogosBloque1.push_back(std::move(dialogoDonJulio));
    
    DialogoNarrativo dialogoCapitan("Capitan Rodrigo", 
        "Como capitan, conozco a toda mi tripulacion.\nTodos son honorables,\nexcepto que recuerdo que Isabella\nestuvo despierta toda la noche,\nalgo inusual en ella.",
        "assets/images/niveles/nivel_sara2/capitanRodrigo.png");
    dialogosBloque1.push_back(std::move(dialogoCapitan));
    
    DialogoNarrativo dialogoIsabella("Isabella la Adivina", 
        "Mis visiones me advirtieron del robo,\n pero no pude evitarlo.\nVi una figura encapuchada.\nNo me atrevo a decir quien era\n por miedo a represalias.",
        "assets/images/niveles/nivel_sara2/isabellaAdivina.png");
    dialogosBloque1.push_back(std::move(dialogoIsabella));
    
    DialogoNarrativo dialogoTestigo("Testigo", 
        "Recuerdo que justo antes de que pasara,\nvi a alguien con una medalla peculiar.\nEra una MEDALLA ANTIGUA.\nSolo una persona en este pueblo\ntiene una igual.",
        "assets/images/niveles/nivel_sara2/anonimo.jpg");
    dialogosBloque1.push_back(std::move(dialogoTestigo));
    
    // Bloque 2
    std::vector<ObjetoBuscar> objetosBloque2;
    objetosBloque2.emplace_back("Camaleon", sf::FloatRect(sf::Vector2f(235.f, 400.f), sf::Vector2f(50.f, 17.f))," ");
    objetosBloque2.emplace_back("Cofre", sf::FloatRect(sf::Vector2f(753.f, 409.f), sf::Vector2f(40.f, 40.f))," ");
    objetosBloque2.emplace_back("Mapa", sf::FloatRect(sf::Vector2f(591.f, 471.f), sf::Vector2f(131.f, 59.f))," ");
    objetosBloque2.emplace_back("Reloj Antiguo", sf::FloatRect(sf::Vector2f(604.f, 390.f), sf::Vector2f(24.f, 34.f))," ");
    objetosBloque2.emplace_back("Microscopio", sf::FloatRect(sf::Vector2f(161.f, 197.f), sf::Vector2f(15.f, 53.f))," ");
    objetosBloque2.emplace_back("Catalejo", sf::FloatRect(sf::Vector2f(362.f, 485.f), sf::Vector2f(33.f, 15.f))," ");
    objetosBloque2.emplace_back("Cortina", sf::FloatRect(sf::Vector2f(380.f, 200.f), sf::Vector2f(34.f, 85.f))," ");
    objetosBloque2.emplace_back("Juego de Llaves", sf::FloatRect(sf::Vector2f(287.f, 507.f), sf::Vector2f(32.f, 42.f))," ");
    objetosBloque2.emplace_back("Periodico", sf::FloatRect(sf::Vector2f(619.f, 161.f), sf::Vector2f(43.f, 49.f))," ");
    objetosBloque2.emplace_back("Red", sf::FloatRect(sf::Vector2f(694.f, 95.f), sf::Vector2f(69.f, 98.f))," ");
    
    std::vector<Sospechoso> sospechososBloque2;
    sospechososBloque2.emplace_back("Sebastian el Guardabosques", sf::FloatRect(sf::Vector2f(300.f, 600.f), sf::Vector2f(80.f, 100.f)), "Conoce cada rincon del bosque,\nsabe esconder cosas.", false);
    sospechososBloque2.emplace_back("Valentina la Arqueologa", sf::FloatRect(sf::Vector2f(550.f, 620.f), sf::Vector2f(75.f, 90.f)), "Experta en tesoros antiguos,\nqueria el cofre para ella sola.", true);
    sospechososBloque2.emplace_back("Don Mateo el Herrero", sf::FloatRect(sf::Vector2f(750.f, 650.f), sf::Vector2f(85.f, 85.f)), "Fabrica candados\nsabia como abrir el cofre sin forzarlo.", false);
    
    std::vector<DialogoNarrativo> dialogosBloque2;
    DialogoNarrativo dialogoSebastian("Sebastian el Guardabosques", 
        "He patrullado este bosque por treinta anios.\nVi a alguien merodeando\ncerca del cofre la noche anterior al robo.",
        "assets/images/niveles/nivel_sara2/sebastian.png");
    dialogosBloque2.push_back(std::move(dialogoSebastian));
    
    DialogoNarrativo dialogoMateo("Don Mateo el Herrero", 
        "Alguien me pidio hacer una copia de\n una llave antigua. El diseno \nera identico al del \ncandado del cofre.",
        "assets/images/niveles/nivel_sara2/mateo.png");
    dialogosBloque2.push_back(std::move(dialogoMateo));
    
    DialogoNarrativo dialogoValentina("Valentina la Arqueologa", 
        "Yo solo queria estudiar\n las piezas del cofre para mi investigacion.\nSe como abrirlo, pero jamas\n robaria algo tan valioso.",
        "assets/images/niveles/nivel_sara2/valentina.png");
    dialogosBloque2.push_back(std::move(dialogoValentina));
    
    DialogoNarrativo dialogoAnonimo("Testigo Anonimo", 
        "Escuche una discusion acalorada cerca\n del campamento. Alguien gritaba \n'el tesoro me pertenece por derecho'.",
        "assets/images/niveles/nivel_sara2/anonimo.jpg");
    dialogosBloque2.push_back(std::move(dialogoAnonimo));

    // Bloque 3
    std::vector<ObjetoBuscar> objetosBloque3;
    objetosBloque3.emplace_back("Calaveras", sf::FloatRect(sf::Vector2f(697.f, 262.f), sf::Vector2f(45.f, 48.f))," ");
    objetosBloque3.emplace_back("Canoa", sf::FloatRect(sf::Vector2f(508.f, 254.f), sf::Vector2f(75.f, 35.f))," ");
    objetosBloque3.emplace_back("Rueda", sf::FloatRect(sf::Vector2f(219.f, 184.f), sf::Vector2f(67.f, 67.f))," ");
    objetosBloque3.emplace_back("Botella de Vino", sf::FloatRect(sf::Vector2f(324.f, 312.f), sf::Vector2f(34.f, 51.f))," ");
    objetosBloque3.emplace_back("Cangrejo", sf::FloatRect(sf::Vector2f(711.f, 446.f), sf::Vector2f(44.f, 24.f))," ");
    objetosBloque3.emplace_back("Muñeco", sf::FloatRect(sf::Vector2f(370.f, 421.f), sf::Vector2f(24.f, 44.f))," ");
    objetosBloque3.emplace_back("Maletin Oculto", sf::FloatRect(sf::Vector2f(345.f, 321.f), sf::Vector2f(55.f, 19.f))," ");
    objetosBloque3.emplace_back("Dado", sf::FloatRect(sf::Vector2f(658.f, 529.f), sf::Vector2f(14.f, 19.f))," ");
    objetosBloque3.emplace_back("Comida para Gato", sf::FloatRect(sf::Vector2f(379.f, 533.f), sf::Vector2f(34.f, 29.f))," ");
    objetosBloque3.emplace_back("Caja de Madera", sf::FloatRect(sf::Vector2f(280.f, 345.f), sf::Vector2f(46.f, 19.f))," ");

    std::vector<Sospechoso> sospechososBloque3;
    sospechososBloque3.emplace_back("El Viejo Marino", sf::FloatRect(sf::Vector2f(300.f, 650.f), sf::Vector2f(80.f, 100.f)), "Vive en la costa desde hace decadas.\nConoce cada naufragio.", false);
    sospechososBloque3.emplace_back("Carmen la Buzo", sf::FloatRect(sf::Vector2f(550.f, 620.f), sf::Vector2f(75.f, 90.f)), "Bucea en busca de tesoros", true);
    sospechososBloque3.emplace_back("Misterioso\nComerciante", sf::FloatRect(sf::Vector2f(750.f, 650.f), sf::Vector2f(85.f, 85.f)), "Siempre aparece justo\ndespues de los naufragios.\nNadie sabe de donde viene.", false);

    std::vector<DialogoNarrativo> dialogosBloque3;
    DialogoNarrativo dialogoMarino("El Viejo Marino", 
        "Yo conozco cada barco que ha naufragado\n en estas costas. Este ultimo... \nfue sabotaje. Vi a alguien nadando hacia la costa\ncon un maletin la noche del accidente.",
        "assets/images/niveles/nivel_sara2/viejoMarino.png");
    dialogosBloque3.push_back(std::move(dialogoMarino));

    DialogoNarrativo dialogoComerciante("Misterioso Comerciante", 
        "Yo solo compro lo que encuentro en la playa.\nPero esa noche... vi a Carmen buceando\nmucho antes de que llegaran los rescatistas.\nQueria algo especifico del barco.",
        "assets/images/niveles/nivel_sara2/comerciante.png");
    dialogosBloque3.push_back(std::move(dialogoComerciante));

    DialogoNarrativo dialogoBuzo("Carmen la Buzo", 
        "Es cierto que buceo en busca de tesoros,\npero jamas robaria nada.\nYo fui quien encontre el maletin...\npero alguien me lo robo de mi escondite.",
        "assets/images/niveles/nivel_sara2/carmenBuzo.png");
    dialogosBloque3.push_back(std::move(dialogoBuzo));

    DialogoNarrativo dialogoTestigo2("Pescador Anonimo", 
        "Recuerdo que la noche del naufragio\n vi una figura cargando algo pesado.\nTenian una MEDALLA DE BUCEO colgando.\nSolo los buzos profesionales tienen eso.",
        "assets/images/niveles/nivel_sara2/anonimo.jpg");
    dialogosBloque3.push_back(std::move(dialogoTestigo2));

    // Guardar en vectores miembro
    m_todosLosObjetos.push_back(objetosBloque1);
    m_todosLosObjetos.push_back(objetosBloque2);
    m_todosLosObjetos.push_back(objetosBloque3);
    
    m_todosLosSospechosos.push_back(sospechososBloque1);
    m_todosLosSospechosos.push_back(sospechososBloque2);
    m_todosLosSospechosos.push_back(sospechososBloque3);
    
    m_todosLosDialogos.push_back(dialogosBloque1);
    m_todosLosDialogos.push_back(dialogosBloque2);
    m_todosLosDialogos.push_back(dialogosBloque3);
    
    // Agregar sets a los pools
    for (const auto& objSet : m_todosLosObjetos) {
        m_criminalMinigame.agregarSetObjetos(objSet);
    }
    for (const auto& sosSet : m_todosLosSospechosos) {
        m_criminalMinigame.agregarSetSospechosos(sosSet);
    }
    for (const auto& diaSet : m_todosLosDialogos) {
        m_criminalMinigame.agregarSetDialogos(diaSet);
    }
    
    m_objetosCriminal = objetosBloque1;  
    m_sospechososCriminal = sospechososBloque1;
    
    m_criminalMinigame.setInventory(m_player.getInventory());
    m_criminalMinigame.setBaseSize(sf::Vector2f(800.f, 600.f));
    
    sf::Vector2u windowSize = window->getSize();
    float minijuegoW = windowSize.x * 0.85f;
    float minijuegoH = windowSize.y * 0.85f;
    float minijuegoX = (static_cast<float>(windowSize.x) - minijuegoW) / 2.f;
    float minijuegoY = (static_cast<float>(windowSize.y) - minijuegoH) / 2.f;
    
    m_criminalMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
    m_criminalMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
    
    m_criminalMinigame.init("assets/images/niveles/nivel_sara2/criminalCase.png", 
                            m_objetosCriminal, 
                            m_sospechososCriminal);
    
    m_criminalMinigame.setDebugMode(true);
    
    m_criminalMinigame.setOnCompleteCallback([this](bool exito) {
    if (exito && !m_criminalGameCompleted) 
    {
        m_criminalGameCompleted = true;
        std::cout << "CASO RESUELTO! Set actual: " << m_setActualCaso << std::endl;
        mostrarMensaje("CASO RESUELTO. Has encontrado todas las pistas y al culpable.\nEntregale las cosas a Andrea", 5.0f);
    }
    });
    
    m_criminalMinigame.cargarFondoOnly("assets/images/niveles/nivel_sara2/criminalCase.png");
    m_criminalMinigame.generarNuevoCaso(); 
    m_setActualCaso = 0;  
}


// REAJUSTAR MINIJUEGO CRIMINAL

void NivelSara2State::reajustarMinijuegoCriminal()
{
    if (!m_criminalMinigame.isActive()) return;
    
    sf::Vector2u windowSize = window->getSize();
    float minijuegoW = windowSize.x * 0.85f;
    float minijuegoH = windowSize.y * 0.85f;
    float minijuegoX = (windowSize.x - minijuegoW) / 2.f;
    float minijuegoY = (windowSize.y - minijuegoH) / 2.f;
    
    m_criminalMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
    m_criminalMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
    
    m_criminalMinigame.init("assets/images/niveles/nivel_sara2/criminalCase.png", 
                            m_objetosCriminal, m_sospechososCriminal);
    m_criminalMinigame.setDebugMode(true);
}


// VERIFICAR ENTRADA CENTINELA
void NivelSara2State::verificarEntradaCentinela()
{
    LevelNode *currentNode = game->getLevelTree().getCurrentNode();
    if (currentNode && currentNode->hasCentinela())
    {
        // Lógica para centinela
    }
}


// VERIFICAR SALIDA DEL NIVEL

void NivelSara2State::verificarSalidaNivel()
{
    m_cercaPuertaSalida = m_player.getHurtbox().findIntersection(m_puertaSalidaArea).has_value();

    static bool ePresionado = false;
    if (m_cercaPuertaSalida)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
        {
            if (!ePresionado)
            {
                ePresionado = true;
                
                if (m_nivelCompletado)
                {
                    std::cout << "Nivel completado! Saliendo de NivelSara2..." << std::endl;
                    game->avanzarNivel();
                }
                else
                {
                    mostrarMensajeFlotante("Debes resolver el caso criminal y\nentregar los objetos a Andrea primero.\nHabla con Andrea presionando R", 4.0f, sf::Color::Yellow);
                }
            }
        }
        else
        {
            ePresionado = false;
        }
    }
}


// UPDATE

void NivelSara2State::update(float dt)
{
    // Actualizar temporizador del mensaje flotante
    if (m_tiempoFlotante > 0.0f) {
        m_tiempoFlotante -= dt;
    }
    
    static sf::Vector2u lastWindowSize = window->getSize();
    sf::Vector2u currentWindowSize = window->getSize();

    if (currentWindowSize != lastWindowSize) {
        lastWindowSize = currentWindowSize;
        
        float minijuegoW = currentWindowSize.x * 0.85f;
        float minijuegoH = currentWindowSize.y * 0.85f;
        float minijuegoX = (currentWindowSize.x - minijuegoW) / 2.f;
        float minijuegoY = (currentWindowSize.y - minijuegoH) / 2.f;
        
        m_criminalMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
        m_criminalMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
        
        m_criminalMinigame.cargarFondoOnly("assets/images/niveles/nivel_sara2/criminalCase.png");
        
        if (m_criminalMinigame.isActive()) 
        {
            std::cout << "Minijuego activo - reescalado aplicado" << std::endl;
        }
    }
    
    // Si hay mensaje emergente, no actualizar movimiento
    if (m_mensajeEmergenteActivo)
    {
        return;
    }
    
    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f)
    {
        m_msjActual.tiempoRestante -= dt;
        if (m_msjActual.tiempoRestante <= 0.0f)
        {
            m_textoMensaje->setString("");
        }
    }

    sf::Vector2f posAnterior = m_player.getPosition();

    // Detección de bloques interactivos
    m_cercaBloqueInteractivo = false;
    int bloqueIndex = -1;

    for (size_t i = 0; i < m_bloquesInteractivos.size(); i++) {
        if (m_player.getHurtbox().findIntersection(m_bloquesInteractivos[i].area).has_value()) {
            m_cercaBloqueInteractivo = true;
            bloqueIndex = i;
            break;
        }
    }
    
    // Detección de área del minijuego criminal
    m_cercaCriminalArea = m_player.getHurtbox().findIntersection(m_criminalArea).has_value();

    static bool cCriminalPresionado = false;
    if (m_cercaCriminalArea && !m_criminalGameCompleted && !m_criminalMinigame.isActive() 
    && !m_mensajeEmergenteActivo) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
            if (!cCriminalPresionado) {
                cCriminalPresionado = true;
                
                m_criminalMinigame.limpiarPools();
                
                for (const auto& objSet : m_todosLosObjetos) {
                    m_criminalMinigame.agregarSetObjetos(objSet);
                }
                for (const auto& sosSet : m_todosLosSospechosos) {
                    m_criminalMinigame.agregarSetSospechosos(sosSet);
                }
                for (const auto& diaSet : m_todosLosDialogos) {
                    m_criminalMinigame.agregarSetDialogos(diaSet);
                }
                
                m_criminalMinigame.generarNuevoCaso();
                m_setActualCaso = m_criminalMinigame.getSetActualObjetos();

                sf::Vector2u winSize = window->getSize();
                float minijuegoW = winSize.x * 0.85f;
                float minijuegoH = winSize.y * 0.85f;
                float minijuegoX = (winSize.x - minijuegoW) / 2.f;
                float minijuegoY = (winSize.y - minijuegoH) / 2.f;
                
                m_criminalMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
                m_criminalMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
                m_criminalMinigame.cargarFondoOnly("assets/images/niveles/nivel_sara2/criminalCase.png");
                
                m_criminalMinigame.activate();
                std::cout << "Minijuego activado con nuevo caso" << std::endl;
            }
        } else {
            cCriminalPresionado = false;
        }
    }
    
    // Manejar minijuego criminal activo
    if (m_criminalMinigame.isActive()) {
        m_criminalMinigame.update(dt);
        return;
    }
    
    // Interacción con Andrea (abrir mensaje)
    static bool rPresionado = false;
    if (m_cercaBloqueInteractivo && bloqueIndex != -1 && !m_mensajeEmergenteActivo && !m_nivelCompletado) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
            if (!rPresionado) {
                rPresionado = true;
                m_mensajeEmergenteActivo = true;
                m_bloqueActualIndex = bloqueIndex;
                std::cout << "Abriendo diálogo - Bloque index: " << bloqueIndex << std::endl;
            }
        } else {
            rPresionado = false;
        }
    } else {
        rPresionado = false;
    }

    // Movimiento
    Inventory *inv = m_player.getInventory();
    if (!inv || !inv->isOpen())
    {
        sf::Vector2f movimiento(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            movimiento.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            movimiento.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            movimiento.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            movimiento.x += 1.f;

        if (movimiento.x != 0.f || movimiento.y != 0.f)
        {
            float length = std::sqrt(movimiento.x * movimiento.x + movimiento.y * movimiento.y);
            movimiento /= length;
        }

        m_player.move(movimiento, dt);
    }

    m_player.update(dt);

    // Colisiones
    for (const auto &obj : m_mapaFisico)
    {
        if (m_player.getHurtbox().findIntersection(obj.getBounds()).has_value())
        {
            m_player.setPosition(posAnterior.x, posAnterior.y);
            break;
        }
    }
    
    CoordenadasDebug::getInstance().actualizar(window, m_camera, m_player.getPosition());
    
    // Cámara
    sf::Vector2f playerPos = m_player.getPosition();
    sf::Vector2f cameraPos = playerPos;

    float halfWidth = 1280.f / 2.f;
    float halfHeight = 720.f / 2.f;

    if (halfWidth * 2.f >= m_worldSize.x)
    {
        cameraPos.x = m_worldSize.x / 2.f;
    }
    else
    {
        if (cameraPos.x < halfWidth)
            cameraPos.x = halfWidth;
        if (cameraPos.x > m_worldSize.x - halfWidth)
            cameraPos.x = m_worldSize.x - halfWidth;
    }

    if (halfHeight * 2.f >= m_worldSize.y)
    {
        cameraPos.y = m_worldSize.y / 2.f;
    }
    else
    {
        if (cameraPos.y < halfHeight)
            cameraPos.y = halfHeight;
        if (cameraPos.y > m_worldSize.y - halfHeight)
            cameraPos.y = m_worldSize.y - halfHeight;
    }

    m_camera.setCenter(cameraPos);

    verificarSalidaNivel();
    verificarEntradaCentinela();

    // Pausa
    if (!m_mostrarTutorial && !m_mostrarTutorialPorTecla && !m_mensajeEmergenteActivo)
    {
        static bool escapeProcesado_ = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
        {
            if (!escapeProcesado_)
            {
                escapeProcesado_ = true;
                game->pushState(std::make_unique<PauseState>(window, game));
            }
        }
        else
        {
            escapeProcesado_ = false;
        }
    }

    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
    {
        m_escapeConsumed = false;
    }
}


// DRAW

void NivelSara2State::draw()
{
    if (!window)
        return;

    float winW = static_cast<float>(window->getSize().x);
    float winH = static_cast<float>(window->getSize().y);

    window->setView(m_camera);

    if (m_background)
    {
        window->draw(*m_background);
    }
    else
    {
        sf::RectangleShape fallback(m_worldSize);
        fallback.setFillColor(sf::Color(50, 30, 30));
        window->draw(fallback);
    }

    m_player.draw(*window);
    m_player.drawHurtbox(*window);

    // Debug: colisiones
    for (const auto &obj : m_mapaFisico)
    {
        sf::RectangleShape colision;
        colision.setPosition(sf::Vector2f(obj.getBounds().position.x, obj.getBounds().position.y));
        colision.setSize(sf::Vector2f(obj.getBounds().size.x, obj.getBounds().size.y));
        colision.setFillColor(sf::Color(255, 0, 0, 100));
        colision.setOutlineThickness(2.f);
        colision.setOutlineColor(sf::Color::Red);
        window->draw(colision);
    }
    
    // Dibujar bloques interactivos (modo debug)
    for (const auto &bloque : m_bloquesInteractivos)
    {
        sf::RectangleShape bloqueDebug(sf::Vector2f(bloque.area.size.x, bloque.area.size.y));
        bloqueDebug.setPosition(sf::Vector2f(bloque.area.position.x, bloque.area.position.y));
        bloqueDebug.setFillColor(sf::Color(0, 255, 255, 100));
        bloqueDebug.setOutlineThickness(2.f);
        bloqueDebug.setOutlineColor(sf::Color::Cyan);
        window->draw(bloqueDebug);
    }
    
    if (m_mostrarPuertaSalida)
    {
        sf::RectangleShape salidaDebug(sf::Vector2f(m_puertaSalidaArea.size.x, m_puertaSalidaArea.size.y));
        salidaDebug.setPosition(sf::Vector2f(m_puertaSalidaArea.position.x, m_puertaSalidaArea.position.y));
        salidaDebug.setFillColor(sf::Color(0, 255, 0, 50));
        salidaDebug.setOutlineThickness(3.f);
        salidaDebug.setOutlineColor(sf::Color::Green);
        window->draw(salidaDebug);
    }

    window->setView(window->getDefaultView());

    CoordenadasDebug::getInstance().dibujar(*window);

    // Mensaje emergente (diálogo de Andrea)
    if (m_mensajeEmergenteActivo && m_bloqueActualIndex >= 0 && m_bloqueActualIndex < (int)m_bloquesInteractivos.size())
    {
        sf::Vector2u winSize = window->getSize();
        float winW2 = static_cast<float>(winSize.x);
        float winH2 = static_cast<float>(winSize.y);
        
        sf::RectangleShape overlay(sf::Vector2f(winW2, winH2));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window->draw(overlay);
        
        if (m_fontLoaded)
        {
            float dialogWidth = 800.f;
            float dialogHeight = 520.f;
            float dialogX = winW2 / 2.f - dialogWidth / 2.f;
            float dialogY = winH2 / 2.f - dialogHeight / 2.f;
            
            sf::RectangleShape shadow(sf::Vector2f(dialogWidth + 8.f, dialogHeight + 8.f));
            shadow.setFillColor(sf::Color(0, 0, 0, 150));
            shadow.setPosition(sf::Vector2f(dialogX + 4.f, dialogY + 4.f));
            window->draw(shadow);
            
            sf::RectangleShape dialogBox(sf::Vector2f(dialogWidth, dialogHeight));
            dialogBox.setFillColor(sf::Color(25, 25, 35, 245));
            dialogBox.setOutlineThickness(3.f);
            
            if (m_nivelCompletado)
                dialogBox.setOutlineColor(sf::Color(100, 255, 100, 255));
            else if (m_criminalGameCompleted)
                dialogBox.setOutlineColor(sf::Color(255, 215, 0, 255));
            else
                dialogBox.setOutlineColor(sf::Color(150, 150, 200, 255));
                
            dialogBox.setPosition(sf::Vector2f(dialogX, dialogY));
            window->draw(dialogBox);
            
            sf::RectangleShape topBar(sf::Vector2f(dialogWidth, 50.f));
            
            if (m_nivelCompletado)
                topBar.setFillColor(sf::Color(30, 80, 30, 220));
            else if (m_criminalGameCompleted)
                topBar.setFillColor(sf::Color(80, 70, 30, 220));
            else
                topBar.setFillColor(sf::Color(50, 40, 60, 220));
                
            topBar.setPosition(sf::Vector2f(dialogX, dialogY));
            window->draw(topBar);
            
            sf::RectangleShape accentLine(sf::Vector2f(dialogWidth - 40.f, 2.f));
            
            if (m_nivelCompletado)
                accentLine.setFillColor(sf::Color(100, 255, 100, 255));
            else if (m_criminalGameCompleted)
                accentLine.setFillColor(sf::Color(255, 215, 0, 255));
            else
                accentLine.setFillColor(sf::Color(200, 150, 100, 255));
                
            accentLine.setPosition(sf::Vector2f(dialogX + 20.f, dialogY + 48.f));
            window->draw(accentLine);
            
            sf::Text tituloText(m_font);
            
            if (m_nivelCompletado)
                tituloText.setString("0 ANDREA o");
            else if (m_criminalGameCompleted)
                tituloText.setString("+ ANDREA +");
            else
                tituloText.setString("- ANDREA -");
                
            tituloText.setCharacterSize(24);
            tituloText.setStyle(sf::Text::Bold);
            
            if (m_nivelCompletado)
                tituloText.setFillColor(sf::Color(100, 255, 100, 255));
            else if (m_criminalGameCompleted)
                tituloText.setFillColor(sf::Color(255, 215, 0, 255));
            else
                tituloText.setFillColor(sf::Color(255, 220, 150, 255));
                
            sf::FloatRect tituloBounds = tituloText.getLocalBounds();
            tituloText.setOrigin(sf::Vector2f(tituloBounds.size.x / 2.f, tituloBounds.size.y / 2.f));
            tituloText.setPosition(sf::Vector2f(winW2 / 2.f, dialogY + 25.f));
            window->draw(tituloText);
            
            sf::Text mensajeText(m_font);

            std::string mensajeCompleto;
            if (m_bloqueActualIndex >= 0 && m_bloqueActualIndex < (int)m_bloquesInteractivos.size()) {
                mensajeCompleto = m_bloquesInteractivos[m_bloqueActualIndex].mensaje;
            }

            mensajeText.setString(mensajeCompleto);
            mensajeText.setCharacterSize(12);

            if (m_nivelCompletado)
                mensajeText.setFillColor(sf::Color(150, 255, 150, 255));
            else
                mensajeText.setFillColor(sf::Color(240, 240, 255, 255));

            mensajeText.setOutlineColor(sf::Color(0, 0, 0, 150));
            mensajeText.setOutlineThickness(1.5f);

            mensajeText.setOrigin(sf::Vector2f(0.f, 0.f));

            float textStartX = dialogX + 30.f;
            float textStartY = dialogY + 80.f;

            mensajeText.setPosition(sf::Vector2f(textStartX, textStartY));
            window->draw(mensajeText);
            
            sf::RectangleShape instruccionPanel(sf::Vector2f(dialogWidth - 40.f, 65.f));
            instruccionPanel.setFillColor(sf::Color(15, 15, 25, 200));
            instruccionPanel.setOutlineThickness(1.f);
            instruccionPanel.setOutlineColor(sf::Color(100, 100, 120, 100));
            instruccionPanel.setPosition(sf::Vector2f(dialogX + 20.f, dialogY + dialogHeight - 85.f));
            window->draw(instruccionPanel);
            
            sf::Text instruccionText(m_font);
            instruccionText.setCharacterSize(16);
            instruccionText.setOutlineThickness(0.5f);
            instruccionText.setOutlineColor(sf::Color::Black);
            
            if (m_nivelCompletado)
            {
                instruccionText.setString("[ ESC ] Cerrar    →    Ve al ascensor y presiona [ E ]");
                instruccionText.setFillColor(sf::Color(100, 255, 100, 255));
            }
            else if (m_criminalGameCompleted)
            {
                instruccionText.setString("[ R ] Entregar objetos a Andrea     |     [ ESC ] Cerrar");
                instruccionText.setFillColor(sf::Color(255, 215, 0, 255));
            }
            else
            {
                instruccionText.setString("[ ESC ] Cerrar     |     Resuelve el caso criminal primero");
                instruccionText.setFillColor(sf::Color(200, 150, 100, 255));
            }
            
            sf::FloatRect instrBounds = instruccionText.getLocalBounds();
            instruccionText.setOrigin(sf::Vector2f(instrBounds.size.x / 2.f, instrBounds.size.y / 2.f));
            instruccionText.setPosition(sf::Vector2f(winW2 / 2.f, dialogY + dialogHeight - 52.f));
            window->draw(instruccionText);
            
            sf::CircleShape cornerTL(6.f, 4);
            cornerTL.setFillColor(sf::Color(200, 150, 100, 180));
            cornerTL.setPosition(sf::Vector2f(dialogX + 3.f, dialogY + 3.f));
            window->draw(cornerTL);
            
            sf::CircleShape cornerTR(6.f, 4);
            cornerTR.setFillColor(sf::Color(200, 150, 100, 180));
            cornerTR.setPosition(sf::Vector2f(dialogX + dialogWidth - 9.f, dialogY + 3.f));
            window->draw(cornerTR);
            
            sf::CircleShape cornerBL(6.f, 4);
            cornerBL.setFillColor(sf::Color(200, 150, 100, 180));
            cornerBL.setPosition(sf::Vector2f(dialogX + 3.f, dialogY + dialogHeight - 9.f));
            window->draw(cornerBL);
            
            sf::CircleShape cornerBR(6.f, 4);
            cornerBR.setFillColor(sf::Color(200, 150, 100, 180));
            cornerBR.setPosition(sf::Vector2f(dialogX + dialogWidth - 9.f, dialogY + dialogHeight - 9.f));
            window->draw(cornerBR);
        }
    }
    
    // Textos de interacción
    if (m_fontLoaded && m_textoInteraccion)
    {
        if (m_cercaPuertaSalida)
        {
            m_textoInteraccion->setString("Presiona E para avanzar al siguiente nivel");
            sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
            m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            m_textoInteraccion->setPosition(sf::Vector2f(winW / 2.f, winH - 70.f));
            window->draw(*m_textoInteraccion);
        }
        if (m_cercaBloqueInteractivo && !m_mensajeEmergenteActivo)
        {
            m_textoInteraccion->setString("Presiona R. Andrea quiere decirte algo");
            sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
            m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            m_textoInteraccion->setPosition(sf::Vector2f(winW / 2.f, winH - 100.f));
            window->draw(*m_textoInteraccion);
        }
        
        if (m_cercaCriminalArea && !m_criminalGameCompleted && !m_criminalMinigame.isActive()) {
            m_textoInteraccion->setString("Presiona R para investigar el crimen en la playa");
            sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
            m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            m_textoInteraccion->setPosition(sf::Vector2f(winW / 2.f, winH - 130.f));
            window->draw(*m_textoInteraccion);
        }
    }

    // Mensaje temporal flotante (el que estaba antes)
    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f && !m_textoMensaje->getString().isEmpty() && !m_mensajeEmergenteActivo)
    {
        sf::Vector2u winSize = window->getSize();
        
        float posX = static_cast<float>(winSize.x) * 0.5f;   
        float posY = static_cast<float>(winSize.y) * 0.85f;  
        
        m_textoMensaje->setCharacterSize(14);  
        m_textoMensaje->setOutlineThickness(1.5f);
        
        sf::FloatRect bounds = m_textoMensaje->getLocalBounds();
        m_textoMensaje->setOrigin(sf::Vector2f(0.f, 0.f));
        m_textoMensaje->setPosition(sf::Vector2f(posX - bounds.size.x / 2.f, posY));
        window->draw(*m_textoMensaje);
    }

    
    // MENSAJE FLOTANTE CENTRADO (PARA ERRORES Y AVISOS)
    
    if (m_tiempoFlotante > 0.0f && !m_mensajeFlotante->getString().isEmpty() && !m_mensajeEmergenteActivo)
    {
        sf::Vector2u winSize = window->getSize();
        float centerX = winSize.x / 2.f;
        float centerY = winSize.y / 2.f;
        
        sf::FloatRect textBounds = m_mensajeFlotante->getLocalBounds();
        
        sf::RectangleShape fondoRect(sf::Vector2f(textBounds.size.x + 40, textBounds.size.y + 30));
        fondoRect.setFillColor(sf::Color(0, 0, 0, 220));
        fondoRect.setOutlineColor(m_mensajeFlotante->getFillColor());
        fondoRect.setOutlineThickness(2.f);
        fondoRect.setOrigin(sf::Vector2f(fondoRect.getSize().x / 2.f, fondoRect.getSize().y / 2.f));
        fondoRect.setPosition(sf::Vector2f(centerX, centerY));
        window->draw(fondoRect);
        
        m_mensajeFlotante->setPosition(sf::Vector2f(centerX, centerY));
        window->draw(*m_mensajeFlotante);
    }

    // Tutorial
    if (m_mostrarTutorial || m_mostrarTutorialPorTecla)
    {
        sf::RectangleShape overlay(sf::Vector2f(window->getSize().x, window->getSize().y));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window->draw(overlay);

        if (m_fontLoaded)
        {
            sf::Text tutorialText(m_font);
            tutorialText.setString(
                "BIENVENIDA AL NIVEL 5\n\n"
                "Has llegado lejos... pero esto no ha terminado.\n\n"
                "Hay un gran caso que resolver.. estás a cargo de esta mision...\n recorre el mapa y descubrelo\n"
                "\n\n"
                "[ESC] Cerrar | [M] Ayuda");
            tutorialText.setCharacterSize(20);
            tutorialText.setFillColor(sf::Color::White);
            sf::FloatRect textBounds = tutorialText.getLocalBounds();
            tutorialText.setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            tutorialText.setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y / 2.f));
            window->draw(tutorialText);
        }
    }

    Inventory *inv = m_player.getInventory();
    if (inv)
    {
        inv->draw(*window);
    }
    
    if (m_criminalMinigame.isActive()) {
        m_criminalMinigame.draw(*window);
    }
}


// CONFIGURAR COLISIONES

void NivelSara2State::configurarColisiones()
{
    m_mapaFisico.clear();

    float mapWidth = m_worldSize.x;
    float mapHeight = m_worldSize.y;

    m_mapaFisico.emplace_back(0.f, 0.f, mapWidth, 250.f);
    m_mapaFisico.emplace_back(0.f, mapHeight - 30.f, mapWidth, 30.f);
    m_mapaFisico.emplace_back(0.f, 0.f, 30.f, mapHeight);
    m_mapaFisico.emplace_back(mapWidth - 30.f, 0.f, 30.f, mapHeight);

    std::cout << "Colisiones configuradas" << std::endl;
}


// JUGADOR HA MUERTO

void NivelSara2State::jugadorHaMuerto()
{
    LevelNode *currentNode = game->getLevelTree().getCurrentNode();
    if (currentNode && currentNode->type == LevelType::CENTINELA)
    {
        GameProgressData &progress = game->getSaveManager().getCurrentProgress();
        game->getSaveManager().addMuerte();
        if (progress.modoElegido == GameProgressData::ModoJuego::CAMINO_AGRADABLE)
        {
            std::cout << "Muerte en centinela (modo agradable)" << std::endl;
        }
        else
        {
            std::cout << "Muerte en centinela (modo consecuencias)" << std::endl;
        }
    }
    else
    {
        game->pushState(std::make_unique<PauseState>(window, game));
    }
}


// MOSTRAR MENSAJE (SISTEMA ORIGINAL)

void NivelSara2State::mostrarMensaje(const std::string &texto, float duracion, sf::Color color)
{
    if (!m_textoMensaje)
        return;

    m_msjActual.texto = texto;
    m_msjActual.tiempoRestante = duracion;
    m_msjActual.color = color;

    m_textoMensaje->setString(texto);
    m_textoMensaje->setFillColor(color);

    sf::FloatRect bounds = m_textoMensaje->getLocalBounds();
    m_textoMensaje->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));

    std::cout << "MENSAJE: " << texto << std::endl;
}