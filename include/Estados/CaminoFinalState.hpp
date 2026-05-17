// Cabecera para el estado que muestra el camino recorrido hasta un final
// Este estado es solo de visualizacion, sin interaccion mas que para salir
// Algoritmo principal: busqueda en profundidad desde la raiz hasta el final

#pragma once

#include "State.hpp"
#include "../Configuracion/Game.hpp"
#include "../Estructuras/LevelTree.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>
#include <unordered_set>

class CaminoFinalState : public State 
{
public:
    // Constructor: recibe la ventana, el juego y el identificador del final alcanzado
    // El identificadorFinal es el id del nodo final donde llego el jugador
    CaminoFinalState(sf::RenderWindow* window, Game* game, 
                     const std::string& identificadorFinal);
    
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw() override;

private:
    // Metodo principal que reconstruye el camino desde el final hacia la raiz
    // Utiliza busqueda en profundidad para encontrar la ruta completa
    // Guarda los identificadores del camino en el conjuntoHashCamino
    void reconstruirCaminoDesdeFinal(const std::string& identificadorFinal);
    
    // Busca recursivamente en el arbol el camino desde la raiz hasta un nodo objetivo
    // Parametros:
    //   nodoActual: nodo que se esta visitando en la recursion
    //   identificadorObjetivo: el nodo final que estamos buscando
    //   rutaEncontrada: vector que se va llenando con los nodos del camino
    // Retorna verdadero si el objetivo esta en el subarbol del nodo actual
    bool buscarCaminoHastaNodo(LevelNode* nodoActual, 
                               const std::string& identificadorObjetivo,
                               std::vector<std::string>& rutaEncontrada);
    
    // Construye la representacion visual del arbol con todos los nodos y conexiones
    // Se llama recursivamente para recorrer todo el arbol
    // Los nodos que estan en conjuntoHashCamino se pintan de amarillo
    void construirArbolVisual(LevelNode* nodo, float posicionX, float posicionY,
                             float espacioHorizontal, float espacioVertical, 
                             int nivelProfundidad);
    
    // Recursos graficos
    sf::Font m_font;
    sf::View m_camera;
    sf::RectangleShape m_background;
    sf::RectangleShape m_panel;
    std::unique_ptr<sf::Text> m_title;
    std::unique_ptr<sf::Text> m_instructionText;
    
    // Estructura que almacena la representacion visual de cada nodo del arbol
    struct NodoVisual 
    {
        LevelNode* punteroNodo;                // Apuntador al nodo original del arbol
        float coordenadaX;                     // Posicion horizontal en pantalla
        float coordenadaY;                     // Posicion vertical en pantalla
        float espacioXOriginal;                // Espacio horizontal original para calculos de zoom
        int profundidad;                       // Nivel de profundidad en el arbol
        std::unique_ptr<sf::Text> textoNombre; // Texto con el nombre del nivel
        sf::RectangleShape cajaContenedora;    // Rectangulo que encierra el texto
    };
    
    // Coleccion de todos los nodos visuales creados
    std::vector<NodoVisual> m_nodosVisuales;
    // Lineas que conectan los nodos del arbol
    std::vector<sf::VertexArray> m_lineas;
    
    // Tabla hash que contiene los identificadores de los nodos que forman el camino
    // Permite verificar en tiempo constante O(1) si un nodo debe resaltarse
    // Fundamento: Unidad III - Dispersion, Tablas Hash
    std::unordered_set<std::string> m_conjuntoHashCamino;
    
    // Variables para el desplazamiento con scroll como en AdminMenuState
    float m_scrollOffset;
    float m_maxScroll;
    float m_arbolTotalY;
    bool m_arrastrandoScroll;
    
    // Barra de desplazamiento visual
    sf::RectangleShape m_scrollBar;
    sf::RectangleShape m_scrollThumb;
    
    // Textura donde se renderiza todo el arbol para optimizar el dibujado
    sf::RenderTexture m_arbolTexture;
    std::unique_ptr<sf::Sprite> m_arbolSprite;
    bool m_textureCreada;
    bool m_necesitaRedibujar;  // Indica si hay que redibujar la textura por cambio de scroll
    
    // Referencia a la raiz del arbol de niveles
    LevelNode* m_raiz;
    // Identificador del final que activo este estado
    std::string m_identificadorFinal;

        // Identificador del nivel desde el cual se entro al centinela
    // Si esta vacio, significa que es el final lineal del nivel6
    std::string m_nivelPadreCentinela;
};