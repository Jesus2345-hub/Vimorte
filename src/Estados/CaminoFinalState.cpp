// Implementacion del estado que muestra el camino del jugador hasta un final
// Algoritmo principal: busqueda en profundidad con retroceso (DFS)
// Fundamento: Unidad V - Arboles, Tema 1: Arboles binarios

#include "Estados/CaminoFinalState.hpp"
#include "Estados/MenuState.hpp"
#include <iostream>
#include <cmath>

// Constructor: inicializa todos los elementos graficos y reconstruye el camino
// Recibe el identificador del final para saber que camino resaltar
CaminoFinalState::CaminoFinalState(sf::RenderWindow* window, Game* game,
                                   const std::string& identificadorFinal)
    : State(window, game), 
      m_identificadorFinal(identificadorFinal),
      m_textureCreada(false),
      m_necesitaRedibujar(true),
      m_scrollOffset(0.0f),
      m_maxScroll(0.0f),
      m_arbolTotalY(0.0f),
      m_arrastrandoScroll(false)
{
    // Cargar la fuente de texto desde el archivo de fuentes del menu
    if (!m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf"))
    {
        std::cerr << "Error al cargar la fuente en CaminoFinalState" << std::endl;
    }

    // Configurar la camara con resolucion fija de 1280x720
    m_camera = sf::View(sf::Vector2f(640.0f, 360.0f), 
                        sf::Vector2f(1280.0f, 720.0f));

    // Fondo oscuro que cubre toda la pantalla
    m_background.setSize(sf::Vector2f(1280.0f, 720.0f));
    m_background.setFillColor(sf::Color(10, 10, 20, 255));

    // Panel central donde se dibujara el arbol completo
    m_panel.setSize(sf::Vector2f(1200.0f, 580.0f));
    m_panel.setPosition(sf::Vector2f(40.0f, 70.0f));
    m_panel.setFillColor(sf::Color(20, 20, 40, 240));
    m_panel.setOutlineThickness(3.0f);
    m_panel.setOutlineColor(sf::Color(255, 215, 0));  // Borde dorado

    // Titulo principal que aparece en la parte superior
    m_title = std::make_unique<sf::Text>(m_font, "CAMINOS PARA LLEGAR HASTA ESE FINAL", 36);
    m_title->setFillColor(sf::Color(255, 215, 0));  // Color dorado
    m_title->setStyle(sf::Text::Bold);

    // Texto con las instrucciones para que el jugador sepa como salir
    m_instructionText = std::make_unique<sf::Text>(m_font,
                     "Scroll: Navegar por el arbol | ENTER o ESC: Volver al menu", 14);
    m_instructionText->setFillColor(sf::Color(200, 200, 200));

    // Configurar la barra de desplazamiento visual
    m_scrollBar.setSize(sf::Vector2f(12.0f, 580.0f));
    m_scrollBar.setPosition(sf::Vector2f(1248.0f, 70.0f));
    m_scrollBar.setFillColor(sf::Color(40, 40, 50, 200));
    m_scrollBar.setOutlineThickness(1.0f);
    m_scrollBar.setOutlineColor(sf::Color(100, 100, 100));

    m_scrollThumb.setSize(sf::Vector2f(10.0f, 60.0f));
    m_scrollThumb.setPosition(sf::Vector2f(1249.0f, 70.0f));
    m_scrollThumb.setFillColor(sf::Color(150, 150, 150, 200));
    m_scrollThumb.setOutlineThickness(1.0f);
    m_scrollThumb.setOutlineColor(sf::Color(200, 200, 200));

      // Obtener la raiz del arbol de niveles del juego
    m_raiz = game->getLevelTree().getRoot();

    // Obtener el nivel desde el cual se entro al centinela
    // Si el final es de un centinela, tendremos el identificador del nivel padre
    // Si esta vacio, significa que es el final lineal del nivel6
    m_nivelPadreCentinela = game->getLevelTree().obtenerNivelPadreDelCentinela();
    
    std::cout << "Nivel padre del centinela: " 
              << (m_nivelPadreCentinela.empty() ? "NINGUNO (final lineal)" : m_nivelPadreCentinela) 
              << std::endl;
       // Paso 1: Reconstruir el camino dependiendo de si es final de centinela o final lineal
    if (m_nivelPadreCentinela.empty())
    {
        // Es el final lineal del nivel6
        // Buscamos el camino desde la raiz hasta el final
        std::cout << "Reconstruyendo camino para final lineal..." << std::endl;
        reconstruirCaminoDesdeFinal(identificadorFinal);
    }
    else
    {
        // Es un final de centinela, sabemos exactamente desde que nivel se entro
        std::cout << "Reconstruyendo todos los caminos posibles hacia: " 
                  << identificadorFinal << std::endl;
        std::cout << "Nivel padre usado: " << m_nivelPadreCentinela << std::endl;
        
        // Paso A: Buscar el camino desde la raiz hasta el nivel padre del centinela
        // Este es el camino que el jugador realmente tomo
        std::vector<std::string> rutaHastaNivelPadre;
        bool encontroNivelPadre = buscarCaminoHastaNodo(
            m_raiz, 
            m_nivelPadreCentinela, 
            rutaHastaNivelPadre);
        
        if (encontroNivelPadre)
        {
            for (const std::string& idNodo : rutaHastaNivelPadre)
            {
                m_conjuntoHashCamino.insert(idNodo);
            }
        }
        
        // Paso B: Encontrar TODOS los centinelas con el mismo id en el arbol
        // y agregar sus caminos hacia el final especifico
        // Para cada centinela encontrado, iluminamos el camino desde su nivel padre
        
        // Primero, necesitamos saber el id del centinela
        // Lo obtenemos del nivel padre que el jugador uso
        LevelNode* nodoNivelPadre = game->getLevelTree().findNode(m_nivelPadreCentinela);
        
        if (nodoNivelPadre != nullptr && nodoNivelPadre->right != nullptr)
        {
            std::string idCentinela = nodoNivelPadre->right->id;
            std::cout << "Buscando todas las ocurrencias del centinela: " 
                      << idCentinela << std::endl;
            
            // Ahora buscamos TODOS los niveles que tienen este centinela como hijo derecho
            // Para cada uno, iluminamos el camino desde la raiz hasta ese nivel,
            // luego el centinela, luego el final
            std::vector<LevelNode*> todosLosNivelesPadre;
            encontrarTodosLosNivelesConCentinela(m_raiz, idCentinela, todosLosNivelesPadre);
            
            std::cout << "Se encontraron " << todosLosNivelesPadre.size() 
                      << " niveles que tienen el centinela " << idCentinela << std::endl;
            
            // Para cada nivel padre encontrado, iluminar el camino completo
            for (LevelNode* nivelPadre : todosLosNivelesPadre)
            {
                std::cout << "  Procesando nivel padre: " << nivelPadre->displayName << std::endl;
                
                // Buscar el camino desde la raiz hasta este nivel padre
                std::vector<std::string> rutaAlternativa;
                bool encontro = buscarCaminoHastaNodo(m_raiz, nivelPadre->id, rutaAlternativa);
                
                if (encontro)
                {
                    // Insertar todos los nodos de este camino alternativo
                    for (const std::string& idNodo : rutaAlternativa)
                    {
                        m_conjuntoHashCamino.insert(idNodo);
                    }
                    
                    // Insertar el centinela (hijo derecho de este nivel padre)
                    if (nivelPadre->right != nullptr)
                    {
                        m_conjuntoHashCamino.insert(nivelPadre->right->id);
                        
                        // Insertar el final especifico
                        if (nivelPadre->right->left != nullptr && 
                            nivelPadre->right->left->id == identificadorFinal)
                        {
                            m_conjuntoHashCamino.insert(nivelPadre->right->left->id);
                        }
                        else if (nivelPadre->right->right != nullptr && 
                                 nivelPadre->right->right->id == identificadorFinal)
                        {
                            m_conjuntoHashCamino.insert(nivelPadre->right->right->id);
                        }
                    }
                }
            }
        }
    }

    // Paso 2: Construir la representacion visual de todo el arbol
    // Los nodos que estan en el conjunto hash se pintaran de amarillo
    // Inicializamos m_arbolTotalY antes de construir
    m_arbolTotalY = 0.0f;
    construirArbolVisual(m_raiz, 600.0f, 130.0f, 500.0f, 150.0f, 0);

    // Calcular el desplazamiento maximo basado en la altura total del arbol
    m_maxScroll = std::max(0.0f, m_arbolTotalY - 350.0f);
    
    std::cout << "Arbol visual construido. Nodos creados: " 
              << m_nodosVisuales.size() << std::endl;
    std::cout << "Lineas creadas: " << m_lineas.size() << std::endl;
    std::cout << "Altura total del arbol: " << m_arbolTotalY << std::endl;
    std::cout << "Scroll maximo: " << m_maxScroll << std::endl;
}
// Metodo principal que encuentra el camino desde un final especifico hasta la raiz
// Utiliza el algoritmo de busqueda en profundidad (DFS) con retroceso
// Complejidad: O(h) donde h es la altura del arbol
// Al encontrar el camino, guarda los identificadores en la tabla hash
void CaminoFinalState::reconstruirCaminoDesdeFinal(
    const std::string& identificadorFinal)
{
    // Vector temporal que almacenara la secuencia de nodos desde la raiz
    // Se va llenando durante la busqueda recursiva
    std::vector<std::string> rutaTemporal;
    
    // Iniciar la busqueda recursiva desde la raiz del arbol
    // La funcion buscarCaminoHastaNodo modifica rutaTemporal
    bool caminoEncontrado = buscarCaminoHastaNodo(m_raiz, 
                                                  identificadorFinal, 
                                                  rutaTemporal);
    
    if (caminoEncontrado)
    {
        // Convertir el vector en una tabla hash para busquedas rapidas O(1)
        // Esto permite verificar si un nodo esta en el camino sin recorrer el vector
        // Fundamento: Unidad III - Dispersion, Tema 1: Tablas Hash
        for (const std::string& idNodo : rutaTemporal)
        {
            m_conjuntoHashCamino.insert(idNodo);
        }
        
        // Mostrar el camino encontrado en consola para depuracion
        std::cout << "Camino reconstruido exitosamente: ";
        for (size_t indice = 0; indice < rutaTemporal.size(); indice++)
        {
            std::cout << rutaTemporal[indice];
            if (indice < rutaTemporal.size() - 1)
            {
                std::cout << " -> ";
            }
        }
        std::cout << std::endl;
    }
    else
    {
        // Si no se encuentra el camino, mostrar error en consola
        std::cerr << "Error: no se pudo encontrar el camino para el final: " 
                  << identificadorFinal << std::endl;
    }
}

// Busqueda recursiva en profundidad que encuentra el camino desde la raiz
// hasta un nodo objetivo especifico
// Parametros:
//   nodoActual: nodo que se esta visitando en este momento de la recursion
//   identificadorObjetivo: el identificador del nodo final que buscamos
//   rutaEncontrada: vector que se va llenando con los identificadores visitados
// Retorna verdadero si el objetivo esta en el subarbol del nodo actual
// Retorna falso si no esta y elimina el nodo actual de la ruta (retroceso)
bool CaminoFinalState::buscarCaminoHastaNodo(
    LevelNode* nodoActual,
    const std::string& identificadorObjetivo,
    std::vector<std::string>& rutaEncontrada)
{
    // Caso base: si el nodo es nulo, no hay camino por esta rama
    if (nodoActual == nullptr)
    {
        return false;
    }

    // Agregar el nodo actual a la ruta que estamos construyendo
    // Este nodo es candidato a formar parte del camino
    rutaEncontrada.push_back(nodoActual->id);

    // Si encontramos el nodo objetivo, retornamos exito
    // La ruta ya contiene todos los nodos desde la raiz hasta aqui
    if (nodoActual->id == identificadorObjetivo)
    {
        return true;
    }

    // Buscar en el subarbol izquierdo recursivamente
    // Si encuentra el objetivo por la izquierda, retorna verdadero
    if (buscarCaminoHastaNodo(nodoActual->left.get(), 
                              identificadorObjetivo, 
                              rutaEncontrada))
    {
        return true;
    }

    // Buscar en el subarbol derecho recursivamente
    // Si encuentra el objetivo por la derecha, retorna verdadero
    if (buscarCaminoHastaNodo(nodoActual->right.get(), 
                              identificadorObjetivo, 
                              rutaEncontrada))
    {
        return true;
    }

    // Si el objetivo no esta en ninguno de los dos subarboles, retrocedemos
    // Eliminamos el nodo actual de la ruta porque no forma parte del camino
    // Este es el mecanismo de retroceso (backtracking) del algoritmo DFS
    rutaEncontrada.pop_back();
    return false;
}

// Construye la representacion visual del arbol de forma recursiva
// Dibuja cajas, textos y lineas de conexion para cada nodo
// Los nodos que estan en m_conjuntoHashCamino se resaltan en amarillo
void CaminoFinalState::construirArbolVisual(
    LevelNode* nodo, 
    float posicionX, 
    float posicionY,
    float espacioHorizontal, 
    float espacioVertical, 
    int nivelProfundidad)
{
    // No hacer nada si el nodo es nulo
    if (nodo == nullptr) 
    {
        return;
    }

    // Crear la representacion visual de este nodo
    NodoVisual nodoGrafico;
    nodoGrafico.punteroNodo = nodo;
    nodoGrafico.coordenadaX = posicionX;
    nodoGrafico.coordenadaY = posicionY;
    nodoGrafico.espacioXOriginal = espacioHorizontal;
    nodoGrafico.profundidad = nivelProfundidad;

    // Acortar el nombre si es muy largo para que quepa en la caja
    std::string nombreParaMostrar = nodo->displayName;
    if (nombreParaMostrar.length() > 14) 
    {
        nombreParaMostrar = nombreParaMostrar.substr(0, 13) + ".";
    }

    // Crear el texto con el nombre del nivel
    nodoGrafico.textoNombre = std::make_unique<sf::Text>(
        m_font, nombreParaMostrar, 12);

    // Configurar dimensiones de la caja contenedora
    float anchoCaja = 110.0f;
    float altoCaja = 35.0f;

    // Configurar la caja rectangular que contiene al nodo
    nodoGrafico.cajaContenedora.setSize(sf::Vector2f(anchoCaja, altoCaja));
    nodoGrafico.cajaContenedora.setOrigin(
        sf::Vector2f(anchoCaja / 2.0f, altoCaja / 2.0f));
    nodoGrafico.cajaContenedora.setPosition(
        sf::Vector2f(posicionX, posicionY));
    nodoGrafico.cajaContenedora.setOutlineThickness(2.0f);

    // Verificar si este nodo pertenece al camino que debe resaltarse
    // Usando la tabla hash para busqueda en tiempo constante O(1)
    // La funcion find retorna un iterador al final si no encuentra el elemento
    bool perteneceAlCamino = 
        m_conjuntoHashCamino.find(nodo->id) 
        != m_conjuntoHashCamino.end();

    if (perteneceAlCamino)
    {
        // Nodo que forma parte del camino del jugador
        // Se pinta con colores amarillos resaltados para destacarlo
        nodoGrafico.cajaContenedora.setFillColor(sf::Color(60, 60, 10, 230));
        nodoGrafico.cajaContenedora.setOutlineColor(sf::Color::Yellow);
        nodoGrafico.textoNombre->setFillColor(sf::Color::Yellow);
        nodoGrafico.textoNombre->setStyle(sf::Text::Bold);
    }
    else if (nodo->type == LevelType::CENTINELA)
    {
        // Nodo tipo centinela que no esta en el camino
        // Se pinta con colores naranjas para identificarlo
        nodoGrafico.cajaContenedora.setFillColor(sf::Color(80, 40, 0, 230));
        nodoGrafico.cajaContenedora.setOutlineColor(sf::Color(255, 150, 50));
        nodoGrafico.textoNombre->setFillColor(sf::Color::White);
    }
    else if (nodo->id.find("final_") != std::string::npos 
             || nodo->id.find("Final") != std::string::npos)
    {
        // Nodo de tipo final que no esta en el camino
        // Se pinta con colores verdes para identificarlo como final
        nodoGrafico.cajaContenedora.setFillColor(sf::Color(0, 50, 0, 230));
        nodoGrafico.cajaContenedora.setOutlineColor(sf::Color(100, 255, 100));
        nodoGrafico.textoNombre->setFillColor(sf::Color::White);
    }
    else
    {
        // Nodo normal que no esta en el camino
        // Se pinta con colores grises
        nodoGrafico.cajaContenedora.setFillColor(sf::Color(60, 60, 70, 230));
        nodoGrafico.cajaContenedora.setOutlineColor(sf::Color(180, 180, 180));
        nodoGrafico.textoNombre->setFillColor(sf::Color::White);
    }

    // Centrar el texto dentro de su caja contenedora
    nodoGrafico.textoNombre->setOutlineThickness(1.5f);
    nodoGrafico.textoNombre->setOutlineColor(sf::Color::Black);
    sf::FloatRect limitesTexto = nodoGrafico.textoNombre->getLocalBounds();
    nodoGrafico.textoNombre->setOrigin(
        sf::Vector2f(limitesTexto.size.x / 2.0f, limitesTexto.size.y / 2.0f));
    nodoGrafico.textoNombre->setPosition(sf::Vector2f(posicionX, posicionY));

    // Guardar este nodo visual en la coleccion
    m_nodosVisuales.push_back(std::move(nodoGrafico));

    // Actualizar la altura total del arbol para calcular el scroll maximo
    if (posicionY > m_arbolTotalY)
    {
        m_arbolTotalY = posicionY;
    }

    // Calcular posiciones para los hijos
    float posicionYHijos = posicionY + espacioVertical;
    float espacioHorizontalHijos = espacioHorizontal / 2.2f;

    // Procesar hijo izquierdo si existe
    if (nodo->left != nullptr)
    {
        float posicionXHijoIzquierdo = posicionX - espacioHorizontalHijos;

        // Crear linea de conexion entre padre e hijo izquierdo
        sf::VertexArray linea(sf::PrimitiveType::Lines, 2);
        linea[0].position = sf::Vector2f(posicionX, posicionY + altoCaja / 2.0f);
        linea[1].position = sf::Vector2f(posicionXHijoIzquierdo, 
                                         posicionYHijos - altoCaja / 2.0f);

        // Verificar si ambos nodos estan en el camino para colorear la linea
        bool padreEnCamino = m_conjuntoHashCamino.find(
            nodo->id) != m_conjuntoHashCamino.end();
        bool hijoIzquierdoEnCamino = m_conjuntoHashCamino.find(
            nodo->left->id) != m_conjuntoHashCamino.end();

        if (padreEnCamino && hijoIzquierdoEnCamino)
        {
            // Ambos nodos estan en el camino: linea amarilla resaltada
            linea[0].color = sf::Color::Yellow;
            linea[1].color = sf::Color::Yellow;
        }
        else
        {
            // Linea gris normal para conexiones fuera del camino
            linea[0].color = sf::Color(120, 120, 120);
            linea[1].color = sf::Color(120, 120, 120);
        }

        m_lineas.push_back(linea);

        // Llamada recursiva para construir el subarbol izquierdo
        construirArbolVisual(nodo->left.get(), 
                           posicionXHijoIzquierdo, 
                           posicionYHijos,
                           espacioHorizontalHijos, 
                           espacioVertical, 
                           nivelProfundidad + 1);
    }

    // Procesar hijo derecho si existe
    if (nodo->right != nullptr)
    {
        float posicionXHijoDerecho = posicionX + espacioHorizontalHijos;

        // Crear linea de conexion entre padre e hijo derecho
        sf::VertexArray linea(sf::PrimitiveType::Lines, 2);
        linea[0].position = sf::Vector2f(posicionX, posicionY + altoCaja / 2.0f);
        linea[1].position = sf::Vector2f(posicionXHijoDerecho, 
                                         posicionYHijos - altoCaja / 2.0f);

        // Verificar si ambos nodos estan en el camino para colorear la linea
        bool padreEnCamino = m_conjuntoHashCamino.find(
            nodo->id) != m_conjuntoHashCamino.end();
        bool hijoDerechoEnCamino = m_conjuntoHashCamino.find(
            nodo->right->id) != m_conjuntoHashCamino.end();

        if (padreEnCamino && hijoDerechoEnCamino)
        {
            // Ambos nodos estan en el camino: linea amarilla resaltada
            linea[0].color = sf::Color::Yellow;
            linea[1].color = sf::Color::Yellow;
        }
        else
        {
            // Linea naranja para ramas de centinela fuera del camino
            linea[0].color = sf::Color(200, 150, 50, 150);
            linea[1].color = sf::Color(200, 150, 50, 150);
        }

        m_lineas.push_back(linea);

        // Llamada recursiva para construir el subarbol derecho
        construirArbolVisual(nodo->right.get(), 
                           posicionXHijoDerecho, 
                           posicionYHijos,
                           espacioHorizontalHijos, 
                           espacioVertical, 
                           nivelProfundidad + 1);
    }
}

// Maneja los eventos de teclado para salir del estado
// Tambien maneja el scroll de la rueda del raton como en AdminMenuState
void CaminoFinalState::handleEvent(const sf::Event& event)
{
    // Obtener la posicion del raton relativa a la camara
    sf::Vector2f mousePos = window->mapPixelToCoords(
        sf::Mouse::getPosition(*window), m_camera);

    // Verificar si se uso la rueda del raton para hacer scroll
    if (const auto* scroll = event.getIf<sf::Event::MouseWheelScrolled>())
    {
        // Actualizar el desplazamiento con la rueda del raton
        // El valor delta indica la direccion y cantidad del scroll
        m_scrollOffset = std::clamp(
            m_scrollOffset - scroll->delta * 40.0f, 
            0.0f, 
            m_maxScroll);
        // Marcar que necesitamos redibujar la textura
        m_necesitaRedibujar = true;
    }

    // Verificar si se presiono un boton del raton
    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouse->button == sf::Mouse::Button::Left)
        {
            // Verificar si el click fue en la barra de scroll
            sf::FloatRect barBounds = m_scrollBar.getGlobalBounds();
            if (barBounds.contains(mousePos))
            {
                // Iniciar arrastre de la barra de scroll
                m_arrastrandoScroll = true;
                // Mover el thumb a la posicion del click
                float porcentaje = (mousePos.y - 70.0f) / 580.0f;
                m_scrollOffset = std::clamp(
                    porcentaje * m_maxScroll, 0.0f, m_maxScroll);
                m_necesitaRedibujar = true;
            }
        }
    }

    // Verificar si se solto un boton del raton
    if (const auto* mouseReleased = event.getIf<sf::Event::MouseButtonReleased>())
    {
        if (mouseReleased->button == sf::Mouse::Button::Left)
        {
            // Dejar de arrastrar la barra de scroll
            m_arrastrandoScroll = false;
        }
    }

    // Verificar si se presiono una tecla
    if (const auto* teclaPresionada = event.getIf<sf::Event::KeyPressed>())
    {
        // Salir del estado al presionar Escape o Enter
        // Volvemos al menu principal llamando a returnToMenu
        if (teclaPresionada->code == sf::Keyboard::Key::Escape 
            || teclaPresionada->code == sf::Keyboard::Key::Enter)
        {
            // Volver al menu principal directamente
            // Esto reemplaza toda la pila de estados por el MenuState
            game->returnToMenu();
        }
    }
}

// Actualiza el estado, principalmente el arrastre de la barra de scroll
void CaminoFinalState::update(float dt)
{
    // Si se esta arrastrando la barra de scroll, actualizar posicion
    if (m_arrastrandoScroll)
    {
        sf::Vector2f mousePos = window->mapPixelToCoords(
            sf::Mouse::getPosition(*window), m_camera);
        float porcentaje = (mousePos.y - 70.0f) / 580.0f;
        float nuevoOffset = std::clamp(
            porcentaje * m_maxScroll, 0.0f, m_maxScroll);
        
        if (nuevoOffset != m_scrollOffset)
        {
            m_scrollOffset = nuevoOffset;
            m_necesitaRedibujar = true;
        }
    }
}

// Dibuja todo el arbol con el camino resaltado en amarillo
// Usa una RenderTexture para optimizar el rendimiento
// Reconstruye la textura cuando cambia el scroll
void CaminoFinalState::draw()
{
    if (!window)
    {
        return;
    }

    // Crear la textura del arbol solo la primera vez que se dibuja
    // o cuando el scroll cambia y necesita redibujarse
    if (!m_textureCreada || m_necesitaRedibujar)
    {
        sf::Vector2u dimensionesPanel(1200, 580);
        
        // Si es la primera vez, crear la textura
        if (!m_textureCreada)
        {
            if (m_arbolTexture.resize(dimensionesPanel))
            {
                m_arbolTexture.setSmooth(true);
                m_arbolSprite = std::make_unique<sf::Sprite>(
                    m_arbolTexture.getTexture());
                m_arbolSprite->setPosition(sf::Vector2f(40.0f, 70.0f));
                m_textureCreada = true;
            }
        }

        // Dibujar el arbol completo en la textura
        m_arbolTexture.clear(sf::Color(20, 20, 40, 240));

        // Configurar la vista interna de la textura
        sf::View vistaTextura(sf::Vector2f(600.0f, 290.0f), 
                             sf::Vector2f(1200.0f, 580.0f));
        m_arbolTexture.setView(vistaTextura);

        // Calcular factores de desplazamiento igual que en AdminMenuState
        float factorExpansion = 1.0f + (m_scrollOffset / 55.0f);
        float desplazamientoX = m_scrollOffset * 7.8f;
        float centroX = 600.0f;

        // Dibujar todas las lineas de conexion entre nodos
        for (auto& linea : m_lineas)
        {
            // Crear una copia de la linea para modificarla con el scroll
            sf::VertexArray lineaDibujo = linea;
            for (size_t i = 0; i < lineaDibujo.getVertexCount(); i++)
            {
                float xOriginal = lineaDibujo[i].position.x;
                lineaDibujo[i].position.x = centroX + 
                    (xOriginal - centroX) * factorExpansion + desplazamientoX;
                lineaDibujo[i].position.y -= m_scrollOffset;
            }
            m_arbolTexture.draw(lineaDibujo);

            // Dibujar la linea una segunda vez desplazada un pixel
            // Esto simula un mayor grosor visual en las lineas
            sf::VertexArray lineaGruesa = lineaDibujo;
            for (size_t i = 0; i < lineaGruesa.getVertexCount(); i++)
            {
                lineaGruesa[i].position.y += 1.0f;
            }
            m_arbolTexture.draw(lineaGruesa);
        }

        // Dibujar todos los nodos visuales del arbol
        for (auto& nodoGrafico : m_nodosVisuales)
        {
            // Calcular posicion con el desplazamiento de scroll
            float nodoY = nodoGrafico.coordenadaY - m_scrollOffset;
            float xCentrado = centroX + 
                (nodoGrafico.coordenadaX - centroX) * factorExpansion + 
                desplazamientoX;

            // Crear una copia de la caja para modificar su posicion
            sf::RectangleShape caja = nodoGrafico.cajaContenedora;
            caja.setPosition(sf::Vector2f(xCentrado, nodoY));

            // Dibujar la caja contenedora del nodo
            m_arbolTexture.draw(caja);

            // Dibujar el texto con el nombre del nivel
            if (nodoGrafico.textoNombre != nullptr)
            {
                sf::Text texto = *nodoGrafico.textoNombre;
                texto.setPosition(sf::Vector2f(xCentrado, nodoY));
                m_arbolTexture.draw(texto);
            }
        }

        // Finalizar el dibujado en la textura
        m_arbolTexture.display();
        
        // Ya no necesita redibujarse hasta que cambie el scroll
        m_necesitaRedibujar = false;
    }

    // Configurar la vista de la ventana principal
    window->setView(m_camera);

    // Dibujar el fondo oscuro
    window->draw(m_background);

    // Dibujar el panel central con borde dorado
    window->draw(m_panel);

    // Dibujar el sprite que contiene el arbol renderizado
    if (m_textureCreada)
    {
        window->draw(*m_arbolSprite);
    }

    // Dibujar la barra de desplazamiento si hay contenido para desplazar
    if (m_maxScroll > 0.0f)
    {
        // Calcular la altura del thumb proporcional al contenido visible
        float thumbHeight = std::max(30.0f, 580.0f * (580.0f / (m_arbolTotalY + 100.0f)));
        float thumbY = 70.0f + (m_scrollOffset / m_maxScroll) * (580.0f - thumbHeight);

        m_scrollThumb.setSize(sf::Vector2f(10.0f, thumbHeight));
        m_scrollThumb.setPosition(sf::Vector2f(1249.0f, thumbY));

        window->draw(m_scrollBar);
        window->draw(m_scrollThumb);
    }

    // Dibujar el titulo centrado en la parte superior
    if (m_title != nullptr)
    {
        sf::FloatRect limitesTitulo = m_title->getLocalBounds();
        m_title->setOrigin(
            sf::Vector2f(limitesTitulo.size.x / 2.0f, 0.0f));
        m_title->setPosition(sf::Vector2f(640.0f, 15.0f));
        window->draw(*m_title);
    }

    // Dibujar las instrucciones centradas en la parte inferior
    if (m_instructionText != nullptr)
    {
        sf::FloatRect limitesInstrucciones = 
            m_instructionText->getLocalBounds();
        m_instructionText->setOrigin(
            sf::Vector2f(limitesInstrucciones.size.x / 2.0f, 0.0f));
        m_instructionText->setPosition(sf::Vector2f(640.0f, 690.0f));
        window->draw(*m_instructionText);
    }
}

// Busca recursivamente todos los nodos que tienen un centinela especifico como hijo derecho
// Parametros:
//   nodoActual: nodo que se esta visitando
//   idCentinela: identificador del centinela que buscamos
//   resultados: vector donde se guardan los nodos nivel padre encontrados
void CaminoFinalState::encontrarTodosLosNivelesConCentinela(
    LevelNode* nodoActual,
    const std::string& idCentinela,
    std::vector<LevelNode*>& resultados)
{
    if (nodoActual == nullptr)
    {
        return;
    }
    
    // Verificar si este nodo tiene un hijo derecho que sea el centinela buscado
    if (nodoActual->right != nullptr && nodoActual->right->id == idCentinela)
    {
        resultados.push_back(nodoActual);
    }
    
    // Buscar recursivamente en los subarboles
    encontrarTodosLosNivelesConCentinela(nodoActual->left.get(), idCentinela, resultados);
    encontrarTodosLosNivelesConCentinela(nodoActual->right.get(), idCentinela, resultados);
}