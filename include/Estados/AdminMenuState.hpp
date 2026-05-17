#ifndef ADMINMENUSTATE_HPP
#define ADMINMENUSTATE_HPP

#include "Estados/State.hpp"
#include "Estructuras/LevelTree.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

class AdminMenuState : public State
{
private:
    sf::View m_camera;

    sf::Font m_font;
    sf::RectangleShape m_background;
    sf::RectangleShape m_panel;
    std::unique_ptr<sf::Text> m_title;
    std::unique_ptr<sf::Text> m_instructionText;

    sf::RenderTexture m_arbolTexture;
    std::unique_ptr<sf::Sprite> m_arbolSprite;
    bool m_textureCreada = false;

    sf::RectangleShape m_scrollBar;
    sf::RectangleShape m_scrollThumb;
    bool m_arrastrandoScroll = false;
    float m_scrollThumbY = 0.f;

    struct NodoVisual
    {
        LevelNode *nodo;
        sf::RectangleShape caja;
        std::unique_ptr<sf::Text> texto;
        float x, y;
        float espacioXOriginal = 500.f;
        int profundidad = 0;
    };

    std::vector<NodoVisual> m_nodosVisuales;
    std::vector<sf::VertexArray> m_lineas;

    float m_scrollOffset = 0.f;
    float m_maxScroll = 0.f;
    float m_arbolTotalY = 0.f;

    void construirArbolVisual(LevelNode *nodo, float x, float y, float espacioX, float espacioY, int profundidad);

public:
    AdminMenuState(sf::RenderWindow *window, Game *game);
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event &event) override;
};

#endif