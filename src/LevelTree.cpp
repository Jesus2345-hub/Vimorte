#include "LevelTree.hpp"
#include "State.hpp"
#include "Game.hpp"
#include "Nivel1State.hpp"
#include "Centinela2.hpp"
#include "MenuState.hpp"
#include "CentinelaConductosState.hpp"
#include "Nivel2State.hpp"  
#include "Nivel4State.hpp"
#include "VideoFinalState.hpp"
#include "Nivel5State.hpp"
#include "Nivel2State.hpp"
#include "Nivel3State.hpp"  
#include "Nivel6State.hpp"
#include "Nivel7State.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

LevelTree::LevelTree() : currentNode(nullptr), inCentinelaPath(false)
{
}

void LevelTree::buildTree()
{
    auto nivel1 = std::make_unique<LevelNode>("nivel1", "Despertar", LevelType::NORMAL, 1);
    nivel1->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<Nivel1State>(w, g);
    };

    auto nivel2 = std::make_unique<LevelNode>("nivel2", "Casino Vimorte", LevelType::NORMAL, 2);
    nivel2->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<Nivel2State>(w, g); 
    };

    auto nivel3 = std::make_unique<LevelNode>("nivel3", "Bomba", LevelType::NORMAL, 3);
    nivel3->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<Nivel3State>(w, g);
    };

    auto nivel4 = std::make_unique<LevelNode>("nivel4", "Estabilizar Signos Vitales", LevelType::NORMAL, 4);
    nivel4->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<Nivel4State>(w, g);
    };

    auto nivel5 = std::make_unique<LevelNode>("nivel5", "Criminal Case", LevelType::NORMAL, 5);
    nivel5->stateFactory = [](sf::RenderWindow* w, Game* g) -> std::unique_ptr<State> {
        return std::make_unique<Nivel5State>(w, g);
    };

    auto nivel6 = std::make_unique<LevelNode>("nivel6", "La Busqueda de los Dientes", LevelType::NORMAL, 6);
    nivel6->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<Nivel6State>(w, g);
    };

    auto nivel7 = std::make_unique<LevelNode>("nivel7", "Cancha de Vimorte", LevelType::NORMAL, 7);
    nivel7->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<Nivel7State>(w, g);
    };

   

    auto centinela1 = std::make_unique<LevelNode>("centinela1", "Centinela 1", LevelType::CENTINELA, 0);
    centinela1->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<Nivel1State>(w, g);
    };
    centinela1->permiteRetrocesoDesdeCentinela = true;

    auto centinela2 = std::make_unique<LevelNode>("centinela2", "Centinela 2", LevelType::CENTINELA, 0);
    centinela2->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<Centinela2State>(w, g);
    };
    centinela2->permiteRetrocesoDesdeCentinela = true;

    // Final bueno - ESCAPAR (hijo izquierdo)
    auto finalBueno = std::make_unique<LevelNode>("final_bueno_Centinela2", "Final Bueno Centinela 2", LevelType::NORMAL, 0);
    finalBueno->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<VideoFinalState>(w, g, "assets/videos/Centinela2_BuenFinal", true);
    };

    // Final malo - UNIRSE (hijo derecho)
    auto finalMalo = std::make_unique<LevelNode>("final_malo_Centinela2", "Final Malo Centinela 2", LevelType::NORMAL, 0);
    finalMalo->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<VideoFinalState>(w, g, "assets/videos/Centinela2_FinalMalo", false);
    };

    // Hijos del centinela2
    centinela2->left = std::move(finalBueno);    
    centinela2->right = std::move(finalMalo);  


    auto centinela3 = std::make_unique<LevelNode>("centinela3", "Conductos", LevelType::CENTINELA, 0);
    centinela3->stateFactory = [](sf::RenderWindow *w, Game *g) -> std::unique_ptr<State>
    {
        return std::make_unique<CentinelaConductosState>(w, g);
    };
    centinela3->permiteRetrocesoDesdeCentinela = true;

    root = std::move(nivel1);
    root->left = std::move(nivel2);
    root->left->left = std::move(nivel3);
    root->left->left->right = std::move(centinela1);
    root->left->left->left = std::move(nivel4);
    root->left->left->left->left = std::move(nivel5);
    root->left->left->left->left->left = std::move(nivel6);
    root->left->left->left->left->left->right = std::move(centinela2);
    root->left->left->left->left->left->left = std::move(nivel7);
   
    root->left->left->left->left->left->left->right = std::move(centinela3);
    

    currentNode = root.get();
    visitedNodes.push_back(currentNode->id);

    std::cout << "Arbol de niveles construido. Raiz: " << root->displayName << std::endl;
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