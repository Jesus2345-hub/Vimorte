#include "Estructuras/LevelTree.hpp"
#include "Estados/State.hpp"
#include "Configuracion/Game.hpp"
#include "Estados/Niveles/Centinelas/Centinela2/Centinela2.hpp"
#include "Estados/MenuState.hpp"
#include "Estados/Niveles/Centinelas/Centinela1/CentinelaConductosState.hpp"
#include "Estados/Niveles/Centinelas/Centinela3/Centinela3State.hpp"
#include "Estados/Niveles/Nivel2/Nivel2State.hpp"  
#include "Estados/Niveles/Nivel4/Nivel4State.hpp"
#include "Estados/VideoFinalState.hpp"
#include "Estados/Niveles/Nivel5/Nivel5State.hpp"
#include "Estados/Niveles/Nivel3/Nivel3State.hpp"  
#include "Estados/Niveles/Nivel1/Nivel6State.hpp"
#include "Estados/Niveles/Nivel6/Nivel7State.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

LevelTree::LevelTree() : currentNode(nullptr), inCentinelaPath(false)
{
}

// Función helper para crear un nodo centinela 1 con sus finales
std::unique_ptr<LevelNode> crearCentinela1()
{
    auto centinela = std::make_unique<LevelNode>("centinela1", "Centinela 1", LevelType::CENTINELA, 0);
    centinela->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<CentinelaConductosState>(w, g);  
    };
    centinela->permiteRetrocesoDesdeCentinela = true;

    auto finalBueno = std::make_unique<LevelNode>("final_bueno_centinela1", "Final Bueno Centinela 1", LevelType::NORMAL, 0);
    finalBueno->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<VideoFinalState>(w, g, "assets/videos/Centinela1_BuenFinal", true);
    };

    auto finalMalo = std::make_unique<LevelNode>("final_malo_centinela1", "Final Malo Centinela 1", LevelType::NORMAL, 0);
    finalMalo->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<VideoFinalState>(w, g, "assets/videos/Centinela1_FinalMalo", false);
    };

    centinela->left = std::move(finalBueno);
    centinela->right = std::move(finalMalo);
    
    return centinela;
}

// Función helper para crear un nodo centinela 2 con sus finales
std::unique_ptr<LevelNode> crearCentinela2()
{
    auto centinela = std::make_unique<LevelNode>("centinela2", "Centinela 2 - Cocina", LevelType::CENTINELA, 0);
    centinela->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<Centinela2State>(w, g);
    };
    centinela->permiteRetrocesoDesdeCentinela = true;

    auto finalBueno = std::make_unique<LevelNode>("final_bueno_Centinela2", "Final Bueno Centinela 2", LevelType::NORMAL, 0);
    finalBueno->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<VideoFinalState>(w, g, "assets/videos/Centinela2_BuenFinal", true);
    };

    auto finalMalo = std::make_unique<LevelNode>("final_malo_Centinela2", "Final Malo Centinela 2", LevelType::NORMAL, 0);
    finalMalo->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<VideoFinalState>(w, g, "assets/videos/Centinela2_FinalMalo", false);
    };

    centinela->left = std::move(finalBueno);
    centinela->right = std::move(finalMalo);
    
    return centinela;
}

// Función helper para crear un nodo centinela 3 (Cámaras) con sus finales
std::unique_ptr<LevelNode> crearCentinela3Camaras()
{
    auto centinela = std::make_unique<LevelNode>("centinela3Camaras", "Centinela 3 - Camaras", LevelType::CENTINELA, 0);
    centinela->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<Centinela3State>(w, g);
    };
    centinela->permiteRetrocesoDesdeCentinela = true;

    // Nodo de FINAL BUENO
    auto finalBueno = std::make_unique<LevelNode>("final_bueno_centinela3", "Final Bueno Centinela 3", LevelType::NORMAL, 0);
    finalBueno->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        // Asegúrate de que esta ruta de carpeta sea la correcta
        return std::make_unique<VideoFinalState>(w, g, "assets/videos/Centinela3_BuenFinal", true);
    };

    // Nodo de FINAL MALO
    auto finalMalo = std::make_unique<LevelNode>("final_malo_centinela3", "Final Malo Centinela 3", LevelType::NORMAL, 0);
    finalMalo->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        // Asegúrate de que esta ruta de carpeta sea la correcta
        return std::make_unique<VideoFinalState>(w, g, "assets/videos/Centinela3_FinalMalo", true);
    };

    centinela->left = std::move(finalBueno);
    centinela->right = std::move(finalMalo);
    
    return centinela;
}



// Final feliz para nivel 6
std::unique_ptr<LevelNode> crearFinalFelizNivel6()
{
    auto finalFeliz = std::make_unique<LevelNode>("final_feliz_nivel6", "Final Feliz - Escape", LevelType::NORMAL, 0);
    finalFeliz->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<VideoFinalState>(w, g, "assets/videos/Final_Lineal", true);
    };
    return finalFeliz;
}

void LevelTree::buildTree()
{
    // ============ NIVEL 1 (viejo nivel6) - ROOT ============
    auto nivel1 = std::make_unique<LevelNode>("nivel1", "La Busqueda de los Dientes", LevelType::NORMAL, 1);
    nivel1->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<Nivel6State>(w, g);
    };

    // ============ NIVEL 2 ============
    auto nivel2 = std::make_unique<LevelNode>("nivel2", "Casino Vimorte", LevelType::NORMAL, 2);
    nivel2->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<Nivel2State>(w, g); 
    };

    // ============ NIVEL 3 ============
    auto nivel3 = std::make_unique<LevelNode>("nivel3", "Bomba", LevelType::NORMAL, 3);
    nivel3->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<Nivel3State>(w, g);
    };

    // ============ NIVEL 4 ============
    auto nivel4 = std::make_unique<LevelNode>("nivel4", "Estabilizar Signos Vitales", LevelType::NORMAL, 4);
    nivel4->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<Nivel4State>(w, g);
    };

    // ============ NIVEL 5 ============
    auto nivel5 = std::make_unique<LevelNode>("nivel5", "Nivel 5", LevelType::NORMAL, 5);
    nivel5->stateFactory = [](sf::RenderWindow* w, Game* g) -> std::unique_ptr<State> {
        return std::make_unique<Nivel5State>(w, g);
    };

    // ============ NIVEL 6 ============
    auto nivel6 = std::make_unique<LevelNode>("nivel6", "Cancha de Vimorte", LevelType::NORMAL, 6);
    nivel6->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<Nivel7State>(w, g);
    };

    // ==================== CONSTRUCCIÓN DEL ÁRBOL ====================
    // nivel1: izquierda = nivel2, derecha = centinela1
    root = std::move(nivel1);
    root->left = std::move(nivel2);
    root->right = crearCentinela1();
    
    // nivel2: izquierda = nivel3, derecha = centinela3Camaras
    root->left->left = std::move(nivel3);
    root->left->right = crearCentinela3Camaras();
    
    // nivel3: izquierda = nivel4, derecha = centinela3Camaras
    root->left->left->left = std::move(nivel4);
    root->left->left->right = crearCentinela3Camaras();
    
    // nivel4: izquierda = nivel5, derecha = centinela2
    root->left->left->left->left = std::move(nivel5);
    root->left->left->left->right = crearCentinela2();
    
    // nivel5: izquierda = nivel6, derecha = centinela2
    root->left->left->left->left->left = std::move(nivel6);
    root->left->left->left->left->right = crearCentinela2();
    
    // nivel6: izquierda = final feliz, derecha = nada
    root->left->left->left->left->left->left = crearFinalFelizNivel6();
    root->left->left->left->left->left->right = nullptr;  // sin centinela

    currentNode = root.get();
    visitedNodes.push_back(currentNode->id);

    std::cout << "Arbol de niveles construido. Raiz: " << root->displayName << std::endl;
    std::cout << "Centinelas: C1(nivel1), C2(nivel4,nivel5), C3Camaras(nivel2,nivel3)" << std::endl;
}


bool LevelTree::goToNextLevel()
{
    if (!currentNode)
        return false;
    if (currentNode->hasCentinela() && !currentNode->right->permiteRetrocesoDesdeCentinela)
    {
        std::cout << "Debes completar el centinela antes de continuar" << std::endl;
        return false;
    }
    if (currentNode->hasNextLevel())
    {
        navigationHistory.push(currentNode);
        currentNode = currentNode->left.get();
        visitedNodes.push_back(currentNode->id);
        inCentinelaPath = false;
        std::cout << "Avanzando a: " << currentNode->displayName << std::endl;
        return true;
    }
    std::cout << "Has completado todos los niveles!" << std::endl;
    return false;
}

// Reemplazar el método goToCentinela en LevelTree.cpp
bool LevelTree::goToCentinela()
{
    if (!currentNode)
        return false;

    if (currentNode->hasCentinela())
    {
        navigationHistory.push(currentNode);
        currentNode = currentNode->right.get();
        visitedNodes.push_back(currentNode->id);
        inCentinelaPath = true;
        std::cout << " Entrando a centinela: " << currentNode->displayName << std::endl;
        return true;
    }

    std::cout << " Este nivel no tiene centinela" << std::endl;
    return false;
}

bool LevelTree::returnFromCentinela()
{
    if (!inCentinelaPath)
    {
        std::cout << "No estas en un centinela" << std::endl;
        return false;
    }
    if (!navigationHistory.empty())
    {
        currentNode = navigationHistory.top();
        navigationHistory.pop();
        visitedNodes.push_back(currentNode->id);
        inCentinelaPath = false;
        std::cout << "Volviendo a: " << currentNode->displayName << std::endl;
        return true;
    }
    return false;
}

bool LevelTree::jumpToNode(const std::string &nodeId)
{
    LevelNode *target = findNode(nodeId);
    if (target)
    {
        while (!navigationHistory.empty())
            navigationHistory.pop();
        visitedNodes.clear();
        currentNode = target;
        visitedNodes.push_back(currentNode->id);
        inCentinelaPath = (currentNode->type == LevelType::CENTINELA);
        std::cout << "[ADMIN] Saltando a: " << currentNode->displayName << std::endl;
        return true;
    }
    std::cout << "Nodo no encontrado: " << nodeId << std::endl;
    return false;
}

std::unique_ptr<State> LevelTree::createCurrentState(sf::RenderWindow *window, Game *game)
{
    if (currentNode && currentNode->stateFactory)
    {
        return currentNode->stateFactory(window, game);
    }
    return nullptr;
}

std::string LevelTree::getCurrentPath() const
{
    std::stringstream ss;
    for (size_t i = 0; i < visitedNodes.size(); ++i)
    {
        if (i > 0)
            ss << "->";
        ss << visitedNodes[i];
    }
    return ss.str();
}

bool LevelTree::restorePath(const std::string &path)
{
    std::vector<std::string> ids;
    std::stringstream ss(path);
    std::string token;
    while (std::getline(ss, token, '-'))
    {
        if (token.size() >= 2 && token[0] == '>')
            token = token.substr(1);
        if (!token.empty() && token != ">")
        {
            token.erase(std::remove_if(token.begin(), token.end(), ::isspace), token.end());
            ids.push_back(token);
        }
    }
    if (ids.empty())
        return false;
    LevelNode *target = findNode(ids.back());
    if (!target)
        return false;
    currentNode = target;
    visitedNodes = ids;
    inCentinelaPath = (currentNode->type == LevelType::CENTINELA);
    std::cout << "Ruta restaurada hasta: " << currentNode->displayName << std::endl;
    return true;
}

std::string LevelTree::getCurrentNodeInfo() const
{
    if (!currentNode)
        return "Sin nivel";
    return currentNode->displayName + " [" + currentNode->id + "]";
}

LevelNode *LevelTree::findNode(const std::string &id)
{
    return findNodeRecursive(root.get(), id);
}

LevelNode *LevelTree::findNodeRecursive(LevelNode *node, const std::string &id)
{
    if (!node)
        return nullptr;
    if (node->id == id)
        return node;
    LevelNode *found = findNodeRecursive(node->left.get(), id);
    if (found)
        return found;
    return findNodeRecursive(node->right.get(), id);
}

void LevelTree::resetToRoot()
{
    currentNode = root.get();
    while (!navigationHistory.empty())
        navigationHistory.pop();
    visitedNodes.clear();
    if (currentNode)
        visitedNodes.push_back(currentNode->id);
    inCentinelaPath = false;
    std::cout << " Árbol reiniciado a la raíz: " << currentNode->displayName << std::endl;
}