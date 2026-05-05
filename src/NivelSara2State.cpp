#include "NivelSara2State.hpp"
#include "PauseState.hpp"
#include "MuerteCentinelaState.hpp"
#include "CoordenadasDebug.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

NivelSara2State::NivelSara2State(sf::RenderWindow *window, Game *game)
    : State(window, game),
      m_background(nullptr),
      m_textoInteraccion(nullptr),
      m_mostrarPuertaSalida(true),
      m_cercaPuertaSalida(false),
      m_escapeConsumed(false),
      m_mostrarTutorial(false),
      m_mostrarTutorialPorTecla(false),
      m_msjActual(),
      m_fontLoaded(false)
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

    // ========== ÁREAS DE INTERACCIÓN ==========
    m_puertaSalidaArea = sf::FloatRect(sf::Vector2f(1550.f, 1350.f), sf::Vector2f(120.f, 180.f));

    configurarColisiones();
    configurarMinijuegoCriminal();
    configurarBloquesInteractivos();
    // ========== CARGA DE FUENTE ==========
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
    }
    else
    {
        m_textoInteraccion = nullptr;
        m_textoMensaje = nullptr;
    }

    // ========== GUARDADO AUTOMÁTICO ==========
    if (game->tienePartidaActiva())
    {
        game->getSaveManager().setNivelActual(5, 1);
        game->guardarPartidaActual();
        std::cout << "Partida guardada automáticamente en NivelSara2" << std::endl;
    }

    std::cout << "NivelSara2State inicializado correctamente" << std::endl;
    game->setIsInLevel(true);
    // ===== ACTIVAR DEBUG DE COORDENADAS SIEMPRE =====
    CoordenadasDebug::getInstance().setVisible(true);

    // ===== IMPORTANTE: Inicializar el minijuego con la imagen de fondo =====
    m_criminalMinigame.init("assets/images/niveles/nivel_sara2/criminalCase.png", 
                            m_objetosCriminal, 
                            m_sospechososCriminal);
    
    m_criminalMinigame.setDebugMode(true);
    
    m_criminalMinigame.setOnCompleteCallback([this](bool exito) {
        if (exito && !m_criminalGameCompleted) {
            m_criminalGameCompleted = true;
            mostrarMensaje("CASO RESUELTO. Has encontrado todas las pistas y al culpable.", 5.0f);
        }
    });
}
void NivelSara2State::configurarBloquesInteractivos()
{
    m_bloquesInteractivos.clear();
    
    m_bloquesInteractivos.push_back({
        sf::FloatRect(sf::Vector2f(280.f, 944.f), sf::Vector2f(100.f, 100.f)),
        "Andrea Tiene horas buscando sus joyas\n mas preciosas.\nHa perdido toda esperanza....\n encuentralos y debajo de los asiento"
    });
    // Agrega mas si es necesario
}
void NivelSara2State::handleEvent(const sf::Event &event)
{
    
    // ===== PRIORIDAD: SI HAY MENSAJE EMERGENTE, ESCAPE LO CIERRA =====
    if (m_mensajeEmergenteActivo) {
        if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                m_mensajeEmergenteActivo = false;
                m_bloqueActualIndex = -1;
                std::cout << "Mensaje emergente cerrado con Escape" << std::endl;
                return;  // No procesar más eventos
            }
        }
        return;  // No procesar nada más mientras el mensaje está activo
    }
    
    // ===== TECLAS GLOBALES (solo si NO hay mensaje activo) =====
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
            // Si no hay mensaje, aquí NO abrimos pausa - eso se maneja en update()
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
void NivelSara2State::configurarMinijuegoCriminal()
{
    m_criminalArea = sf::FloatRect(sf::Vector2f(800.f, 600.f), sf::Vector2f(150.f, 150.f));
    m_cercaCriminalArea = false;
    m_criminalGameCompleted = false;
    
    // ===== LIMPIAR POOLS =====
    m_criminalMinigame.limpiarPools();
    
    // ===== BLOQUE 1 (Caso de la playa) =====
    std::vector<ObjetoBuscar> objetosBloque1;
    objetosBloque1.emplace_back("Collar", sf::FloatRect(sf::Vector2f(560.f, 426.f), sf::Vector2f(36.f, 40.f)), "Un collar de perlas abandonado en la arena");
    objetosBloque1.emplace_back("Carta Mojada", sf::FloatRect(sf::Vector2f(191.f, 438.f), sf::Vector2f(75.f, 38.f)), "Una carta informacion crucial de Andrea");
    objetosBloque1.emplace_back("Reloj Arena", sf::FloatRect(sf::Vector2f(665.f, 357.f), sf::Vector2f(25.f, 70.f)), "Un reloj de regalo");
    objetosBloque1.emplace_back("Medalla", sf::FloatRect(sf::Vector2f(116.f, 164.f), sf::Vector2f(20.f, 23.f)), "Una medalla vieja");
    objetosBloque1.emplace_back("Botella", sf::FloatRect(sf::Vector2f(104.f, 359.f), sf::Vector2f(25.f, 24.f)), "Una botella con las penas de Andrea");
    objetosBloque1.emplace_back("Diario", sf::FloatRect(sf::Vector2f(524.f, 496.f), sf::Vector2f(35.f, 30.f)), "El diario personal");
    objetosBloque1.emplace_back("Anillo", sf::FloatRect(sf::Vector2f(504.f, 414.f), sf::Vector2f(21.f, 25.f)), "Un anillo de compromiso");
    objetosBloque1.emplace_back("Foto", sf::FloatRect(sf::Vector2f(744.f, 240.f), sf::Vector2f(34.f, 54.f)), "Una foto, no sabemos por que");
    objetosBloque1.emplace_back("Cuchillo", sf::FloatRect(sf::Vector2f(348.f, 370.f), sf::Vector2f(36.f, 25.f)), "Quiere defenderse");
    objetosBloque1.emplace_back("Bolso", sf::FloatRect(sf::Vector2f(408.f, 473.f), sf::Vector2f(109.f, 80.f)), "Un bolso que ya le toca cambio");
    objetosBloque1.emplace_back("Trapo Viejo", sf::FloatRect(sf::Vector2f(766.f, 455.f), sf::Vector2f(26.f, 15.f)), "Un Trapo sucio... no limpia su bolso");
    
    std::vector<Sospechoso> sospechososBloque1;
    sospechososBloque1.emplace_back("Capitan Rodrigo", sf::FloatRect(sf::Vector2f(300.f, 650.f), sf::Vector2f(80.f, 100.f)), "El capitan del barco. Tenia acceso a todo, pero parece honesto.", false);
    sospechososBloque1.emplace_back("Isabella la Adivina", sf::FloatRect(sf::Vector2f(750.f, 620.f), sf::Vector2f(70.f, 90.f)), "Siempre supo que algo pasaria. ES LA CULPABLE", true);
    sospechososBloque1.emplace_back("Don Julio el Pescador", sf::FloatRect(sf::Vector2f(550.f, 680.f), sf::Vector2f(80.f, 90.f)), "Vio todo desde su bote, pero jura que no fue el.", false);
    
    // Diálogos con fondos personalizados - Usar push_back con std::move
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
        "Recuerdo que justo antes de que pasara,\nvi a alguien con una medalla peculiar.\nEra una MEDALLA ANTIGUA.\nSolo una persona en este pueblo\n tiene una igual.",
        "assets/images/niveles/nivel_sara2/anonimo.jpg");
    dialogosBloque1.push_back(std::move(dialogoTestigo));
    
    // ===== BLOQUE 2 (Caso del cofre) =====
    std::vector<ObjetoBuscar> objetosBloque2;
    objetosBloque2.emplace_back("Camaleon", sf::FloatRect(sf::Vector2f(235.f, 400.f), sf::Vector2f(50.f, 17.f)), "Un camaleon que cambia de color, podria ser una pista");
    objetosBloque2.emplace_back("Cofre", sf::FloatRect(sf::Vector2f(753.f, 409.f), sf::Vector2f(40.f, 40.f)), "Un cofre antiguo cerrado con candado");
    objetosBloque2.emplace_back("Mapa", sf::FloatRect(sf::Vector2f(591.f, 471.f), sf::Vector2f(131.f, 59.f)), "Un mapa del tesoro con marcas sospechosas");
    objetosBloque2.emplace_back("Reloj Antiguo", sf::FloatRect(sf::Vector2f(604.f, 390.f), sf::Vector2f(24.f, 34.f)), "Un reloj de bolsillo que marca una hora especifica");
    objetosBloque2.emplace_back("Microscopio", sf::FloatRect(sf::Vector2f(172.f, 197.f), sf::Vector2f(5.f, 53.f)), "Un microscopio para examinar evidencias");
    objetosBloque2.emplace_back("Catalejo", sf::FloatRect(sf::Vector2f(362.f, 485.f), sf::Vector2f(33.f, 9.f)), "Un catalejo usado para vigilar desde lejos");
    objetosBloque2.emplace_back("Cortina", sf::FloatRect(sf::Vector2f(380.f, 200.f), sf::Vector2f(34.f, 85.f)), "Una cortina rasgada, alguien la uso para esconderse");
    objetosBloque2.emplace_back("Juego de Llaves", sf::FloatRect(sf::Vector2f(287.f, 507.f), sf::Vector2f(32.f, 42.f)), "Un conjunto de cinco llaves, una abre el cofre");
    objetosBloque2.emplace_back("Periodico", sf::FloatRect(sf::Vector2f(619.f, 161.f), sf::Vector2f(43.f, 49.f)), "Un periodico viejo con noticias sobre robos");
    objetosBloque2.emplace_back("Red", sf::FloatRect(sf::Vector2f(694.f, 95.f), sf::Vector2f(69.f, 98.f)), "Una red de pescar, tal vez usada para atrapar pruebas");
    
    std::vector<Sospechoso> sospechososBloque2;
    sospechososBloque2.emplace_back("Sebastian el Guardabosques", sf::FloatRect(sf::Vector2f(300.f, 600.f), sf::Vector2f(80.f, 100.f)), "Conoce cada rincon del bosque, sabe esconder cosas.", false);
    sospechososBloque2.emplace_back("Valentina la Arqueologa", sf::FloatRect(sf::Vector2f(550.f, 620.f), sf::Vector2f(75.f, 90.f)), "Experta en tesoros antiguos. ES LA CULPABLE, queria el cofre para ella sola.", true);
    sospechososBloque2.emplace_back("Don Mateo el Herrero", sf::FloatRect(sf::Vector2f(750.f, 650.f), sf::Vector2f(85.f, 85.f)), "Fabrica candados, sabia como abrir el cofre sin forzarlo.", false);
    
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


        // ===== BLOQUE 3 (Caso del naufragio misterioso) =====
    std::vector<ObjetoBuscar> objetosBloque3;
    objetosBloque3.emplace_back("Calaveras", sf::FloatRect(sf::Vector2f(697.f, 262.f), sf::Vector2f(45.f, 48.f)), "Una calavera tallada en madera, simbolo de una antigua tripulacion");
    objetosBloque3.emplace_back("Canoa", sf::FloatRect(sf::Vector2f(508.f, 254.f), sf::Vector2f(75.f, 28.f)), "Una canoa varada en la orilla, con rasguños recientes");
    objetosBloque3.emplace_back("Rueda", sf::FloatRect(sf::Vector2f(219.f, 184.f), sf::Vector2f(67.f, 67.f)), "Una rueda de timon rota, parte del naufragio");
    objetosBloque3.emplace_back("Botella de Vino", sf::FloatRect(sf::Vector2f(324.f, 312.f), sf::Vector2f(34.f, 51.f)), "Una botella de vino añejo, aun con liquido adentro");
    objetosBloque3.emplace_back("Cangrejo", sf::FloatRect(sf::Vector2f(711.f, 446.f), sf::Vector2f(44.f, 24.f)), "Un cangrejo que parecia cuidar algo entre las rocas");
    objetosBloque3.emplace_back("Muñeco", sf::FloatRect(sf::Vector2f(370.f, 421.f), sf::Vector2f(24.f, 44.f)), "Un muñeco de trapo con una nota escondida");
    objetosBloque3.emplace_back("Maletin Oculto", sf::FloatRect(sf::Vector2f(345.f, 321.f), sf::Vector2f(55.f, 19.f)), "Un maletin enterrado en la arena, cerrado con candado");
    objetosBloque3.emplace_back("Dado", sf::FloatRect(sf::Vector2f(658.f, 529.f), sf::Vector2f(14.f, 19.f)), "Un dado de marfil, siempre muestra el numero seis");
    objetosBloque3.emplace_back("Comida para Gato", sf::FloatRect(sf::Vector2f(379.f, 533.f), sf::Vector2f(34.f, 29.f)), "Latas de comida para gato, muy fuera de lugar aqui");
    objetosBloque3.emplace_back("Caja de Madera", sf::FloatRect(sf::Vector2f(280.f, 345.f), sf::Vector2f(46.f, 19.f)), "Una caja de madera flotante, contiene documentos");

    std::vector<Sospechoso> sospechososBloque3;
    sospechososBloque3.emplace_back("El Viejo Marino", sf::FloatRect(sf::Vector2f(300.f, 650.f), sf::Vector2f(80.f, 100.f)), "Vive en la costa desde hace decadas. Conoce cada naufragio.", false);
    sospechososBloque3.emplace_back("Carmen la Buzo", sf::FloatRect(sf::Vector2f(550.f, 620.f), sf::Vector2f(75.f, 90.f)), "Bucea en busca de tesoros. ES LA CULPABLE, robo las pertenencias de los desaparecidos.", true);
    sospechososBloque3.emplace_back("Misterioso Comerciante", sf::FloatRect(sf::Vector2f(750.f, 650.f), sf::Vector2f(85.f, 85.f)), "Siempre aparece justo despues de los naufragios. Nadie sabe de donde viene.", false);

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

    // ===== AGREGAR SETS A LOS POOLS  =====
    m_criminalMinigame.agregarSetObjetos(std::move(objetosBloque1));
    m_criminalMinigame.agregarSetSospechosos(std::move(sospechososBloque1));
    m_criminalMinigame.agregarSetDialogos(dialogosBloque1);
    
    m_criminalMinigame.agregarSetObjetos(std::move(objetosBloque2));
    m_criminalMinigame.agregarSetSospechosos(std::move(sospechososBloque2));
    m_criminalMinigame.agregarSetDialogos(dialogosBloque2);

    m_criminalMinigame.agregarSetObjetos(std::move(objetosBloque3));
    m_criminalMinigame.agregarSetSospechosos(std::move(sospechososBloque3));
    m_criminalMinigame.agregarSetDialogos(dialogosBloque3);
    
    // Guardar copias para reinit
    m_objetosCriminal = objetosBloque1;  
    m_sospechososCriminal = sospechososBloque1;
    
    // Configurar el minijuego
    m_criminalMinigame.setInventory(m_player.getInventory());
    m_criminalMinigame.setBaseSize(sf::Vector2f(800.f, 600.f));
    
    // Calcular tamaño basado en la ventana
    sf::Vector2u windowSize = window->getSize();
    float minijuegoW = windowSize.x * 0.85f;
    float minijuegoH = windowSize.y * 0.85f;
    float minijuegoX = (static_cast<float>(windowSize.x) - minijuegoW) / 2.f;
    float minijuegoY = (static_cast<float>(windowSize.y) - minijuegoH) / 2.f;
    
    m_criminalMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
    m_criminalMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
    
    // Inicializar con el fondo y datos
    m_criminalMinigame.init("assets/images/niveles/nivel_sara2/criminalCase.png", 
                            m_objetosCriminal, 
                            m_sospechososCriminal);
    
    m_criminalMinigame.setDebugMode(true);
    
    m_criminalMinigame.setOnCompleteCallback([this](bool exito) {
        if (exito && !m_criminalGameCompleted) {
            m_criminalGameCompleted = true;
            mostrarMensaje("CASO RESUELTO. Has encontrado todas las pistas y al culpable.\nEntregale las cosas a Andrea", 5.0f);
        }
    });
}

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
    
    // Reinicializar con los objetos guardados
    m_criminalMinigame.init("assets/images/niveles/nivel_sara2/criminalCase.png", 
                            m_objetosCriminal, m_sospechososCriminal);
    m_criminalMinigame.setDebugMode(true);
}
void NivelSara2State::verificarEntradaCentinela()
{
    LevelNode *currentNode = game->getLevelTree().getCurrentNode();
    if (currentNode && currentNode->hasCentinela())
    {
        // Lógica para centinela
    }
}

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
                std::cout << "Saliendo de NivelSara2..." << std::endl;
                game->avanzarNivel();
            }
        }
        else
        {
            ePresionado = false;
        }
    }
}

void NivelSara2State::update(float dt)
{
     static sf::Vector2u lastWindowSize = window->getSize();
sf::Vector2u currentWindowSize = window->getSize();

if (currentWindowSize != lastWindowSize) {
    lastWindowSize = currentWindowSize;
    
    // Calcular posición en PÍXELES de la ventana
    float minijuegoW = currentWindowSize.x * 0.85f;
    float minijuegoH = currentWindowSize.y * 0.85f;
    float minijuegoX = (currentWindowSize.x - minijuegoW) / 2.f;
    float minijuegoY = (currentWindowSize.y - minijuegoH) / 2.f;
    
    m_criminalMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
    m_criminalMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
    
    // Forzar actualización del fondo
    m_criminalMinigame.init("assets/images/niveles/nivel_sara2/criminalCase.png",
                            m_objetosCriminal, 
                            m_sospechososCriminal);
}

    // ===== SI HAY MENSAJE EMERGENTE, NO ACTUALIZAR MOVIMIENTO =====
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

    // ========== DETECCIÓN DE BLOQUES INTERACTIVOS ==========
    m_cercaBloqueInteractivo = false;
    int bloqueIndex = -1;

    for (size_t i = 0; i < m_bloquesInteractivos.size(); i++) {
        if (m_player.getHurtbox().findIntersection(m_bloquesInteractivos[i].area).has_value()) {
            m_cercaBloqueInteractivo = true;
            bloqueIndex = i;
            break;
        }
    }
    // ========== DETECCIÓN DE ÁREA DEL MINIJUEGO CRIMINAL ==========
    m_cercaCriminalArea = m_player.getHurtbox().findIntersection(m_criminalArea).has_value();

    static bool cCriminalPresionado = false;
    if (m_cercaCriminalArea && !m_criminalGameCompleted && !m_criminalMinigame.isActive() 
        && !m_mensajeEmergenteActivo) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
            if (!cCriminalPresionado) {
                cCriminalPresionado = true;
                
                // RESET COMPLETO antes de activar
                m_criminalMinigame.resetCompletamente();  // Usar el nuevo método
                
                sf::Vector2u winSize = window->getSize();
                float minijuegoW = winSize.x * 0.85f;
                float minijuegoH = winSize.y * 0.85f;
                float minijuegoX = (winSize.x - minijuegoW) / 2.f;
                float minijuegoY = (winSize.y - minijuegoH) / 2.f;
                
                m_criminalMinigame.setPosition(sf::Vector2f(minijuegoX, minijuegoY));
                m_criminalMinigame.setSize(sf::Vector2f(minijuegoW, minijuegoH));
                
                m_criminalMinigame.activate();
                std::cout << "Minijuego activado - Estado completamente reiniciado" << std::endl;
            }
        } else {
            cCriminalPresionado = false;
        }
    }
    // Manejar minijuego criminal activo
    if (m_criminalMinigame.isActive()) {
        m_criminalMinigame.update(dt);
        return;  // No mover al jugador mientras el minijuego está activo
    }
    // Detectar tecla R para mostrar mensaje (solo si NO hay mensaje activo)
    static bool rPresionado = false;
    if (m_cercaBloqueInteractivo && bloqueIndex != -1 && !m_mensajeEmergenteActivo) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
            if (!rPresionado) {
                rPresionado = true;
                m_mensajeEmergenteActivo = true;
                m_bloqueActualIndex = bloqueIndex;
                std::cout << "Mostrando mensaje del bloque " << bloqueIndex << std::endl;
            }
        } else {
            rPresionado = false;
        }
    } else {
        rPresionado = false;
    }

    // ========== MOVIMIENTO (solo si no hay mensaje activo) ==========
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

    // ========== COLISIONES ==========
    for (const auto &obj : m_mapaFisico)
    {
        if (m_player.getHurtbox().findIntersection(obj.getBounds()).has_value())
        {
            m_player.setPosition(posAnterior.x, posAnterior.y);
            break;
        }
    }
    // ===== ACTUALIZAR COORDENADAS DEBUG =====
    CoordenadasDebug::getInstance().actualizar(window, m_camera, m_player.getPosition());
    
    // ========== CÁMARA ==========
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

    // ========== PAUSA (solo si NO hay mensaje activo) ==========
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
void NivelSara2State::draw()
{
    if (!window)
        return;

    // Declarar winW y winH al inicio (para que estén disponibles en toda la función)
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

    // ===== DIBUJAR COORDENADAS DEBUG =====
    CoordenadasDebug::getInstance().dibujar(*window);

    // ===== MENSAJE EMERGENTE (encima de todo) =====
    if (m_mensajeEmergenteActivo && m_bloqueActualIndex >= 0 && m_bloqueActualIndex < (int)m_bloquesInteractivos.size())
    {
        // Fondo semitransparente
        sf::RectangleShape overlay(sf::Vector2f(window->getSize().x, window->getSize().y));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window->draw(overlay);
        
        if (m_fontLoaded)
        {
            // Cuadro de diálogo
            sf::RectangleShape dialogBox(sf::Vector2f(600.f, 300.f));
            dialogBox.setFillColor(sf::Color(30, 30, 30, 240));
            dialogBox.setOutlineThickness(3.f);
            dialogBox.setOutlineColor(sf::Color::White);
            dialogBox.setOrigin(sf::Vector2f(300.f, 150.f));
            dialogBox.setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y / 2.f));
            window->draw(dialogBox);
            
            // Texto del mensaje
            sf::Text mensajeText(m_font);
            mensajeText.setString(m_bloquesInteractivos[m_bloqueActualIndex].mensaje);
            mensajeText.setCharacterSize(20);
            mensajeText.setFillColor(sf::Color::White);
            mensajeText.setOrigin(sf::Vector2f(
                mensajeText.getLocalBounds().size.x / 2.f,
                mensajeText.getLocalBounds().size.y / 2.f
            ));
            mensajeText.setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y / 2.f - 30.f));
            window->draw(mensajeText);
            
            // Texto de instrucción
            sf::Text instruccionText(m_font);
            instruccionText.setString("[ESC] Cerrar");
            instruccionText.setCharacterSize(16);
            instruccionText.setFillColor(sf::Color(200, 200, 200));
            instruccionText.setOrigin(sf::Vector2f(
                instruccionText.getLocalBounds().size.x / 2.f,
                instruccionText.getLocalBounds().size.y / 2.f
            ));
            instruccionText.setPosition(sf::Vector2f(window->getSize().x / 2.f, window->getSize().y / 2.f + 80.f));
            window->draw(instruccionText);
        }
    }
    
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
        
        // Texto para el minijuego criminal
        if (m_cercaCriminalArea && !m_criminalGameCompleted && !m_criminalMinigame.isActive()) {
            m_textoInteraccion->setString("Presiona R para investigar el crimen en la playa");
            sf::FloatRect textBounds = m_textoInteraccion->getLocalBounds();
            m_textoInteraccion->setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
            m_textoInteraccion->setPosition(sf::Vector2f(winW / 2.f, winH - 130.f));
            window->draw(*m_textoInteraccion);
        }
    }

    if (m_textoMensaje && m_msjActual.tiempoRestante > 0.0f && !m_textoMensaje->getString().isEmpty())
    {
        sf::Vector2u winSize = window->getSize();
        m_textoMensaje->setPosition(sf::Vector2f(winSize.x / 2.f, winSize.y / 3.f));
        window->draw(*m_textoMensaje);
    }

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
    
    // Dibujar minijuego criminal si está activo
    if (m_criminalMinigame.isActive()) {
        m_criminalMinigame.draw(*window);
    }
}

void NivelSara2State::configurarColisiones()
{
    m_mapaFisico.clear();

    // Límites del mapa basados en el tamaño del mundo (background)
    float mapWidth = m_worldSize.x;
    float mapHeight = m_worldSize.y;

    // Pared superior (borde de arriba)
    m_mapaFisico.emplace_back(0.f, 0.f, mapWidth, 250.f);
    
    // Pared inferior (borde de abajo)
    m_mapaFisico.emplace_back(0.f, mapHeight - 30.f, mapWidth, 30.f);
    
    // Pared izquierda (borde izquierdo)
    m_mapaFisico.emplace_back(0.f, 0.f, 30.f, mapHeight);
    
    // Pared derecha (borde derecho)
    m_mapaFisico.emplace_back(mapWidth - 30.f, 0.f, 30.f, mapHeight);

    std::cout << "Colisiones configuradas" << std::endl;
}

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