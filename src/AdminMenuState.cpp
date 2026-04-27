// Reemplazar AdminMenuState.cpp completo:
#include "AdminMenuState.hpp"
#include "Game.hpp"
#include <iostream>
#include <algorithm>

AdminMenuState::AdminMenuState(sf::RenderWindow* window, Game* game)
    : State(window, game), m_scrollOffset(0.f), m_maxScroll(0.f) {
    
    if (!m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "❌ Error cargando fuente en AdminMenu" << std::endl;
    }
    
    m_background.setSize(sf::Vector2f(1280.f, 720.f));
    m_background.setFillColor(sf::Color(10, 10, 20, 255));
    m_panel.setSize(sf::Vector2f(900.f, 550.f));
    m_panel.setPosition(sf::Vector2f(190.f, 85.f));
    m_panel.setFillColor(sf::Color(30, 30, 50, 240));
    m_panel.setOutlineThickness(3.f);
    m_panel.setOutlineColor(sf::Color(255, 0, 0));
    
    m_title = std::make_unique<sf::Text>(m_font, "MODO ADMINISTRADOR", 36);
    m_title->setFillColor(sf::Color::Red);
    m_title->setPosition(sf::Vector2f(400.f, 50.f));
    
    m_instructionText = std::make_unique<sf::Text>(m_font, "Click: Saltar | ESC: Volver", 14);
    m_instructionText->setFillColor(sf::Color(180, 180, 180));
    m_instructionText->setPosition(sf::Vector2f(500.f, 680.f));
    
    collectNodes(game->getLevelTree());
    updateNodePositions();
}

void AdminMenuState::collectNodes(LevelTree& tree) {
    m_allNodes.clear();
    std::function<void(LevelNode*)> traverse = [&](LevelNode* node) {
        if (!node) return;
        m_allNodes.push_back(node);
        traverse(node->left.get());
        traverse(node->right.get());
    };
    traverse(tree.getRoot());
    
    for (size_t i = 0; i < m_allNodes.size(); ++i) {
        sf::RectangleShape box(sf::Vector2f(840.f, 35.f));
        box.setFillColor(sf::Color(40, 40, 60, 200));
        m_nodeBoxes.push_back(box);
        auto text = std::make_unique<sf::Text>(m_font, m_allNodes[i]->displayName, 15);
        text->setFillColor(sf::Color::White);
        m_nodeTexts.push_back(std::move(text));
        m_nodeHover.push_back(false);
    }
    m_maxScroll = std::max(0.f, (m_allNodes.size() * 45.f) - 480.f);
}

void AdminMenuState::updateNodePositions() {
    float startY = 140.f - m_scrollOffset;
    for (size_t i = 0; i < m_nodeBoxes.size(); ++i) {
        float y = startY + i * 45.f;
        m_nodeBoxes[i].setPosition(sf::Vector2f(220.f, y));
        m_nodeTexts[i]->setPosition(sf::Vector2f(240.f, y + 8.f));
    }
}

void AdminMenuState::handleEvent(const sf::Event& event) {
    sf::Vector2f mousePos = window->mapPixelToCoords(sf::Mouse::getPosition(*window));
    
    if (const auto* scroll = event.getIf<sf::Event::MouseWheelScrolled>()) {
        m_scrollOffset = std::clamp(m_scrollOffset - scroll->delta * 30.f, 0.f, m_maxScroll);
        updateNodePositions();
    }
    
    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouse->button == sf::Mouse::Button::Left) {
            for (size_t i = 0; i < m_nodeBoxes.size(); ++i) {
                if (m_nodeBoxes[i].getGlobalBounds().contains(mousePos)) {
                    game->getLevelTree().jumpToNode(m_allNodes[i]->id);
                    auto newState = game->getLevelTree().createCurrentState(window, game);
                    if (newState) game->changeState(std::move(newState));
                    return;
                }
            }
        }
    }
    
    if (event.is<sf::Event::KeyPressed>() && 
        event.getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape) {
        game->popState();
    }
}

void AdminMenuState::update(float dt) {}

void AdminMenuState::draw() {
    if (!window) return;
    
    float winW = static_cast<float>(window->getSize().x);
    float winH = static_cast<float>(window->getSize().y);
    float centerX = winW / 2.f;
    
    m_background.setSize(sf::Vector2f(winW, winH));
    window->draw(m_background);
    
    float panelW = winW * 0.7f;
    float panelH = winH * 0.76f;
    m_panel.setSize(sf::Vector2f(panelW, panelH));
    m_panel.setPosition(sf::Vector2f(centerX - panelW/2.f, winH * 0.12f));
    window->draw(m_panel);
    
    if (m_title) {
        m_title->setPosition(sf::Vector2f(centerX - panelW/2.f + 30.f, winH * 0.07f));
        window->draw(*m_title);
    }
    
    if (m_instructionText) {
        m_instructionText->setPosition(sf::Vector2f(centerX - 100.f, winH * 0.94f));
        window->draw(*m_instructionText);
    }
    
    float nodeW = panelW - 60.f;
    float nodeX = centerX - panelW/2.f + 30.f;
    
    for (size_t i = 0; i < m_nodeBoxes.size(); ++i) {
        float y = winH * 0.19f - m_scrollOffset + i * 45.f;
        if (y >= winH * 0.18f && y <= winH * 0.83f) {
            m_nodeBoxes[i].setSize(sf::Vector2f(nodeW, 35.f));
            m_nodeBoxes[i].setPosition(sf::Vector2f(nodeX, y));
            window->draw(m_nodeBoxes[i]);
            if (m_nodeTexts[i]) {
                m_nodeTexts[i]->setPosition(sf::Vector2f(nodeX + 20.f, y + 8.f));
                window->draw(*m_nodeTexts[i]);
            }
        }
    }
}