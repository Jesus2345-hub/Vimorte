#ifndef ADMINMENUSTATE_HPP
#define ADMINMENUSTATE_HPP

#include "State.hpp"
#include "LevelTree.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

class AdminMenuState : public State {
private:
    sf::Font m_font;
    sf::RectangleShape m_background;
    sf::RectangleShape m_panel;
    std::unique_ptr<sf::Text> m_title;
    std::unique_ptr<sf::Text> m_instructionText;
    std::vector<std::unique_ptr<sf::Text>> m_nodeTexts;
    std::vector<sf::RectangleShape> m_nodeBoxes;
    std::vector<bool> m_nodeHover;
    std::vector<LevelNode*> m_allNodes;
    float m_scrollOffset;
    float m_maxScroll;
    
    void collectNodes(LevelTree& tree);
    void updateNodePositions();
    
public:
    AdminMenuState(sf::RenderWindow* window, Game* game);
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event& event) override;
};

#endif