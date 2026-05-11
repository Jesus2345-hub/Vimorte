#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <string>
#include <memory>

struct Item {
    std::string name;
    sf::Color color;
    std::string rutaImagen;
    std::shared_ptr<sf::Texture> textura;
    std::unique_ptr<sf::Sprite> sprite;  
    
    // Constructor por defecto
    Item() : name(""), color(sf::Color::White), rutaImagen(""), textura(nullptr), sprite(nullptr) {}
    
    // Constructor con parámetros
    Item(const std::string& n, const sf::Color& c, const std::string& ruta = "")
        : name(n), color(c), rutaImagen(ruta), textura(nullptr), sprite(nullptr) {
        if (!rutaImagen.empty()) {
            textura = std::make_shared<sf::Texture>();
            if (textura->loadFromFile(rutaImagen)) {
                sprite = std::make_unique<sf::Sprite>(*textura);
            } else {
                std::cout << "Error cargando textura: " << rutaImagen << std::endl;
                textura = nullptr;
            }
        }
    }
    
    // Método para escalar el sprite (si existe)
    void scaleSprite(float targetWidth, float targetHeight) {
        if (sprite && textura) {
            sf::Vector2u texSize = textura->getSize();
            if (texSize.x > 0 && texSize.y > 0) {
                float scaleX = targetWidth / static_cast<float>(texSize.x);
                float scaleY = targetHeight / static_cast<float>(texSize.y);
                sprite->setScale(sf::Vector2f(scaleX, scaleY));
            }
        }
    }
    
    // Método para dibujar el sprite (si existe)
    void draw(sf::RenderWindow& window, const sf::Vector2f& position, const sf::Vector2f& origin) {
        if (sprite) {
            sprite->setPosition(position);
            sprite->setOrigin(origin);
            window.draw(*sprite);
        }
    }
    
    // Método para dibujar sin origen específico
    void draw(sf::RenderWindow& window, const sf::Vector2f& position) {
        if (sprite) {
            sprite->setPosition(position);
            window.draw(*sprite);
        }
    }
};

class Inventory {
private:
    std::vector<std::unique_ptr<Item>> m_items;
    std::vector<Item*> m_hotbar;
    
    int m_selectedSlot;
    int m_activeHotbarSlot;
    bool m_isOpen;
    
    // UI
    sf::Font m_font;
    bool m_fontLoaded;                    
    sf::RectangleShape m_hotbarBg;
    sf::RectangleShape m_inventoryBg;
    std::vector<sf::RectangleShape> m_hotbarSlots;
    std::vector<sf::RectangleShape> m_inventorySlots;
    std::unique_ptr<sf::Text> m_infoText;
    
    // Variables para arrastrar items
    int m_draggedItemIndex;
    bool m_isDraggingItem;
    sf::Vector2f m_dragOffset;
    
    static const int HOTBAR_SIZE = 5;
    static const int INVENTORY_COLS = 5;
    static const int INVENTORY_ROWS = 3;
    
public:
    Inventory();
    
    void addItem(const Item& item);
    void removeItem(int index);
    void clear();
    void swapItems(int index1, int index2);
    
    void setOpen(bool open);
    bool isOpen() const { return m_isOpen; }
    void toggleOpen() { m_isOpen = !m_isOpen; }
    
    void nextSlot();
    void prevSlot();
    void selectSlot(int slot);
    int getSelectedSlot() const { return m_selectedSlot; }
    int getActiveHotbarSlot() const { return m_activeHotbarSlot; }
    
    void update(float dt);
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);
    
    bool tryCollectItem(const std::string& itemName, const sf::Color& color);
    void addDefaultItems();
    
    Item* getItem(int index);
};