#ifndef LEVELTREE_HPP
#define LEVELTREE_HPP

#include "Estructuras/LevelNode.hpp"
#include <memory>
#include <stack>
#include <string>
#include <vector>

class State;
class Game;

class LevelTree {
private:
    std::unique_ptr<LevelNode> root;
    LevelNode* currentNode;
    std::stack<LevelNode*> navigationHistory;
    std::vector<std::string> visitedNodes;
    bool inCentinelaPath;
      std::string nivelPadreDelCentinela;
public:
    LevelTree();
    ~LevelTree() = default;
    
    void buildTree();
    
    LevelNode* getCurrentNode() const { return currentNode; }
    LevelNode* getRoot() const { return root.get(); }
    
    bool goToNextLevel();
    bool goToCentinela();
    bool returnFromCentinela();
    bool jumpToNode(const std::string& nodeId);
    
    std::unique_ptr<State> createCurrentState(sf::RenderWindow* window, Game* game);
    
    std::string getCurrentPath() const;
    bool restorePath(const std::string& path);
    const std::vector<std::string>& getVisitedNodes() const { return visitedNodes; }
    
    std::string getCurrentNodeInfo() const;
    bool isInCentinela() const { return inCentinelaPath; }
    
    LevelNode* findNode(const std::string& id);
    // Añadir en la sección pública de LevelTree.hpp:
void resetToRoot();
    // Obtiene el identificador del nodo actual donde esta el jugador
    // Util para saber que final se alcanzo y mostrarlo en CaminoFinalState
    std::string obtenerIdNodoActual() const;
    
// Retorna el identificador del nivel desde el cual se entro al centinela
// Si no estamos en un centinela o es el final lineal, retorna cadena vacia
    std::string obtenerNivelPadreDelCentinela() const
    {
        return nivelPadreDelCentinela;
    }

private:
    LevelNode* findNodeRecursive(LevelNode* node, const std::string& id);
};

#endif