#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>

struct Item {
    std::string name;
    sf::Color color;
};

class Inventory {
private:
    std::vector<std::unique_ptr<Item>> m_items;
    std::vector<Item*> m_hotbar;
    int m_selectedSlot;        // Ahora puede ser cualquier índice (0-14)
    int m_activeHotbarSlot;    // Slot de la hotbar que está activo (0-4)
    bool m_isOpen;
    
    // UI
    sf::Font m_font;
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
};