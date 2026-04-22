#ifndef LEVELNODE_HPP
#define LEVELNODE_HPP

#include <string>
#include <memory>
#include <functional>
#include <SFML/Graphics.hpp>

// Forward declarations
class State;
class Game;

// Tipo de nivel
enum class LevelType {
    NORMAL,     // Nivel lineal (hijo izquierdo)
    CENTINELA   // Subárbol centinela (hijo derecho)
};

class LevelNode {
public:
    std::string id;                 // Identificador único (ej: "nivel1", "centinela1")
    std::string displayName;        // Nombre para mostrar
    LevelType type;
    int levelNumber;                // Número de nivel (1-10) o 0 para centinelas
    
    // Hijos en el árbol binario
    std::unique_ptr<LevelNode> left;   // Camino lineal (siguiente nivel)
    std::unique_ptr<LevelNode> right;  // Camino centinela (subárbol)
    
    // Factoría para crear el estado correspondiente
    std::function<std::unique_ptr<State>(sf::RenderWindow*, Game*)> stateFactory;
    
    // Metadatos para el guardado
    bool requiereCentinelaCompletado = false;
    bool permiteRetrocesoDesdeCentinela = true;
    
    LevelNode(const std::string& id_, const std::string& displayName_, 
              LevelType type_ = LevelType::NORMAL, int levelNum_ = 0);
    
    bool isLeaf() const { return !left && !right; }
    bool hasCentinela() const { return right != nullptr; }
    bool hasNextLevel() const { return left != nullptr; }
};

#endif