#include "Inventory.hpp"
#include <iostream>
#include <algorithm>

Inventory::Inventory() : m_selectedSlot(-1), m_activeHotbarSlot(0), m_isOpen(false), 
                         m_fontLoaded(false), m_infoText(nullptr), m_draggedItemIndex(-1), m_isDraggingItem(false) {
    // ========== CARGA DE FUENTE CON VERIFICACIÓN ==========
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    if (!m_fontLoaded) {
        std::cerr << "ERROR: No se pudo cargar la fuente 'assets/fonts/menu/VCR_OSD_MONO.ttf'." << std::endl;
        std::cerr << "No se mostrarán textos en el inventario." << std::endl;
    }
    
    // Inicializar m_infoText solo si la fuente es válida, de lo contrario se queda nullptr
    if (m_fontLoaded) {
        m_infoText = std::make_unique<sf::Text>(m_font);
        m_infoText->setCharacterSize(12);
        m_infoText->setFillColor(sf::Color(200, 200, 200));
    }
    else
    {
        m_infoText = nullptr;
    }

    // Hotbar
    m_hotbarBg.setSize(sf::Vector2f(300.f, 50.f));
    m_hotbarBg.setFillColor(sf::Color(0, 0, 0, 160));
    m_hotbarBg.setOutlineThickness(1.f);
    m_hotbarBg.setOutlineColor(sf::Color(80, 80, 80));
    m_hotbarBg.setOrigin(sf::Vector2f(150.f, 25.f));

    m_hotbarSlots.resize(HOTBAR_SIZE);
    for (int i = 0; i < HOTBAR_SIZE; ++i)
    {
        m_hotbarSlots[i].setSize(sf::Vector2f(45.f, 45.f));
        m_hotbarSlots[i].setFillColor(sf::Color(30, 30, 30, 200));
        m_hotbarSlots[i].setOutlineThickness(1.f);
        m_hotbarSlots[i].setOutlineColor(sf::Color(70, 70, 70));
        m_hotbarSlots[i].setOrigin(sf::Vector2f(22.5f, 22.5f));
    }

    // Inventario grande (5x3)
    m_inventoryBg.setSize(sf::Vector2f(350.f, 250.f));
    m_inventoryBg.setFillColor(sf::Color(20, 20, 20, 230));
    m_inventoryBg.setOutlineThickness(2.f);
    m_inventoryBg.setOutlineColor(sf::Color(100, 100, 100));
    m_inventoryBg.setOrigin(sf::Vector2f(175.f, 125.f));

    m_inventorySlots.resize(INVENTORY_COLS * INVENTORY_ROWS);
    for (int i = 0; i < INVENTORY_COLS * INVENTORY_ROWS; ++i)
    {
        m_inventorySlots[i].setSize(sf::Vector2f(45.f, 45.f));
        m_inventorySlots[i].setFillColor(sf::Color(30, 30, 30, 200));
        m_inventorySlots[i].setOutlineThickness(1.f);
        m_inventorySlots[i].setOutlineColor(sf::Color(70, 70, 70));
        m_inventorySlots[i].setOrigin(sf::Vector2f(22.5f, 22.5f));
    }

    m_hotbar.resize(HOTBAR_SIZE, nullptr);

    // Cargar texturas de items
    std::vector<std::string> itemNames = {"Rifle", "Gallina", "Dientes", "Llave", "Balon Basket", "Destornillador"};
    std::vector<std::string> itemFiles = {
        "assets/images/items/rifle.png",
        "assets/images/items/gallo.png",
        "assets/images/items/dientes.png",
        "assets/images/items/llave.png",
        "assets/images/niveles/nivel7/balon_basket.png",
        "assets/images/items/destornillador.png"
    };

    for (size_t i = 0; i < itemNames.size(); i++)
    {
        sf::Texture tex;
        if (tex.loadFromFile(itemFiles[i]))
        {
            tex.setSmooth(true);
            m_itemTextures[itemNames[i]] = tex;
            std::cout << "✅ Textura cargada: " << itemNames[i] << std::endl;
        }
        else
        {
            std::cerr << "❌ Error cargando: " << itemFiles[i] << std::endl;
        }
    }
}

void Inventory::clear()
{
    m_items.clear();
    for (int i = 0; i < HOTBAR_SIZE; ++i)
    {
        m_hotbar[i] = nullptr;
    }
    m_selectedSlot = -1;
    m_activeHotbarSlot = 0;
}

void Inventory::addItem(const Item& item) {
    // Crear el Item manualmente sin copiar 
    auto newItem = std::make_unique<Item>();
    newItem->name = item.name;
    newItem->color = item.color;
    newItem->rutaImagen = item.rutaImagen;
    
    // Copiar textura y sprite correctamente
    if (item.textura) {
        newItem->textura = item.textura;  
        if (item.sprite) {
            newItem->sprite = std::make_unique<sf::Sprite>(*item.sprite);
        }
    }
    
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (!m_items[i]) {
            m_items[i] = std::move(newItem);
            if (i < HOTBAR_SIZE) m_hotbar[i] = m_items[i].get();
            // Si no hay nada seleccionado, seleccionar este
            if (m_selectedSlot == -1) {
                m_selectedSlot = i;
                if (i < HOTBAR_SIZE) m_activeHotbarSlot = i;
            }
            return;
        }
    }

    m_items.push_back(std::move(newItem));
    size_t index = m_items.size() - 1;
    if (index < HOTBAR_SIZE) m_hotbar[index] = m_items[index].get();
    // Si no hay nada seleccionado, seleccionar este
    if (m_selectedSlot == -1) {
        m_selectedSlot = index;
        if (index < HOTBAR_SIZE) m_activeHotbarSlot = index;
    }
}

void Inventory::removeItem(int index)
{
    if (index >= 0 && index < (int)m_items.size())
    {
        bool wasSelected = (index == m_selectedSlot);
        m_items.erase(m_items.begin() + index);

        // Actualizar punteros de la hotbar
        for (int i = 0; i < HOTBAR_SIZE; ++i)
        {
            if (i < (int)m_items.size())
                m_hotbar[i] = m_items[i].get();
            else
                m_hotbar[i] = nullptr;
        }

        // Si eliminamos el item seleccionado, buscar otro
        if (wasSelected)
        {
            m_selectedSlot = -1;
            for (int i = 0; i < (int)m_items.size(); ++i)
            {
                if (m_items[i])
                {
                    m_selectedSlot = i;
                    if (i < HOTBAR_SIZE)
                        m_activeHotbarSlot = i;
                    break;
                }
            }
        }
        // Si el índice seleccionado cambió, ajustarlo
        else if (index < m_selectedSlot)
        {
            m_selectedSlot--;
        }
    }
}

void Inventory::swapItems(int index1, int index2)
{
    if (index1 < 0 || index1 >= (int)m_items.size() ||
        index2 < 0 || index2 >= (int)m_items.size())
    {
        return;
    }

    // Intercambiar los punteros
    std::swap(m_items[index1], m_items[index2]);

    // Actualizar punteros de la hotbar
    for (int i = 0; i < HOTBAR_SIZE; ++i)
    {
        if (i < (int)m_items.size())
            m_hotbar[i] = m_items[i].get();
        else
            m_hotbar[i] = nullptr;
    }

    // Actualizar selección si es necesario
    if (m_selectedSlot == index1)
    {
        m_selectedSlot = index2;
        if (index2 < HOTBAR_SIZE)
            m_activeHotbarSlot = index2;
    }
    else if (m_selectedSlot == index2)
    {
        m_selectedSlot = index1;
        if (index1 < HOTBAR_SIZE)
            m_activeHotbarSlot = index1;
    }
}

void Inventory::setOpen(bool open)
{
    m_isOpen = open;
}

void Inventory::nextSlot()
{
    if (m_items.empty())
        return;

    // Solo navegar entre slots de la hotbar que tengan items
    int originalSlot = m_activeHotbarSlot;
    do
    {
        m_activeHotbarSlot = (m_activeHotbarSlot + 1) % HOTBAR_SIZE;
    } while (m_activeHotbarSlot != originalSlot &&
             (m_activeHotbarSlot >= (int)m_items.size() || !m_items[m_activeHotbarSlot]));

    m_selectedSlot = m_activeHotbarSlot;
}

void Inventory::prevSlot()
{
    if (m_items.empty())
        return;

    // Solo navegar entre slots de la hotbar que tengan items
    int originalSlot = m_activeHotbarSlot;
    do
    {
        m_activeHotbarSlot = (m_activeHotbarSlot - 1 + HOTBAR_SIZE) % HOTBAR_SIZE;
    } while (m_activeHotbarSlot != originalSlot &&
             (m_activeHotbarSlot >= (int)m_items.size() || !m_items[m_activeHotbarSlot]));

    m_selectedSlot = m_activeHotbarSlot;
}

void Inventory::selectSlot(int slot)
{
    if (slot >= 0 && slot < (int)m_items.size() && m_items[slot])
    {
        m_selectedSlot = slot;
        if (slot < HOTBAR_SIZE)
        {
            m_activeHotbarSlot = slot;
        }
        std::cout << "Slot seleccionado: " << slot << std::endl; // Debug
    }
}
void Inventory::update(float dt)
{
    // No necesita actualización por frame
}

void Inventory::handleEvent(const sf::Event &event, sf::RenderWindow &window)
{
    if (const auto *keyEvent = event.getIf<sf::Event::KeyPressed>())
    {

        if (keyEvent->code == sf::Keyboard::Key::E)
        {
            toggleOpen();
            m_isDraggingItem = false;
            m_draggedItemIndex = -1;
        }

        if (!m_isOpen)
        {
            // Teclas 1-5 para seleccionar slots de la hotbar
            if (keyEvent->code == sf::Keyboard::Key::Num1)
                selectSlot(0);
            else if (keyEvent->code == sf::Keyboard::Key::Num2)
                selectSlot(1);
            else if (keyEvent->code == sf::Keyboard::Key::Num3)
                selectSlot(2);
            else if (keyEvent->code == sf::Keyboard::Key::Num4)
                selectSlot(3);
            else if (keyEvent->code == sf::Keyboard::Key::Num5)
                selectSlot(4);
        }
    }

    // Rueda del ratón
    if (!m_isOpen)
    {
        if (const auto *scrollEvent = event.getIf<sf::Event::MouseWheelScrolled>())
        {
            if (scrollEvent->delta > 0)
                prevSlot();
            else if (scrollEvent->delta < 0)
                nextSlot();
        }
    }

    // Manejo de clicks en el inventario extendido
    if (m_isOpen)
    {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        
        // ===== CLICK IZQUIERDO PRESIONADO =====
        if (const auto* mousePress = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePress->button == sf::Mouse::Button::Left) {
                for (size_t i = 0; i < m_inventorySlots.size(); ++i) {
                    if (m_inventorySlots[i].getGlobalBounds().contains(mousePos)) {
                        // Verificar si hay item en este slot
                        if (i < m_items.size() && m_items[i])
                        {
                            // Iniciar arrastre
                            m_draggedItemIndex = i;
                            m_isDraggingItem = true;
                            m_dragOffset = mousePos - m_inventorySlots[i].getPosition();

                            // SELECCIONAR INMEDIATAMENTE AL HACER CLICK (incluso antes de soltar)
                            selectSlot(i);
                        }
                        break;
                    }
                }
            }
        }
        
        // ===== CLICK IZQUIERDO SOLTADO =====
        if (const auto* mouseRelease = event.getIf<sf::Event::MouseButtonReleased>()) {
            if (mouseRelease->button == sf::Mouse::Button::Left) {
                // Si estábamos arrastrando, soltar el item
                if (m_isDraggingItem)
                {
                    for (size_t i = 0; i < m_inventorySlots.size(); ++i)
                    {
                        if (m_inventorySlots[i].getGlobalBounds().contains(mousePos))
                        {
                            // Intercambiar items
                            if (m_draggedItemIndex != -1 && i != (size_t)m_draggedItemIndex)
                            {
                                // Asegurar que ambos índices tengan espacio en el vector
                                size_t maxIndex = std::max(i, (size_t)m_draggedItemIndex);
                                while (m_items.size() <= maxIndex)
                                {
                                    m_items.push_back(nullptr);
                                }
                                swapItems(m_draggedItemIndex, i);
                            }
                            break;
                        }
                    }
                    m_isDraggingItem = false;
                    m_draggedItemIndex = -1;
                }
            }
        }
        
        // ===== CLICK DERECHO: Eliminar item =====
        if (const auto* mousePress = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePress->button == sf::Mouse::Button::Right) {
                for (size_t i = 0; i < m_inventorySlots.size(); ++i) {
                    if (m_inventorySlots[i].getGlobalBounds().contains(mousePos)) {
                        if (i < m_items.size() && m_items[i]) {
                            removeItem(i);
                        }
                        break;
                    }
                }
            }
        }
    }
}

void Inventory::draw(sf::RenderWindow &window)
{
    sf::Vector2u windowSize = window.getSize();
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    
    // ========== DIBUJAR HOTBAR (INVENTARIO CERRADO) ==========
    if (!m_isOpen) {
        m_hotbarBg.setPosition(sf::Vector2f(windowSize.x / 2.f, windowSize.y - 35.f));
        window.draw(m_hotbarBg);

        for (int i = 0; i < HOTBAR_SIZE; ++i)
        {
            float x = windowSize.x / 2.f - 100.f + i * 50.f;
            float y = windowSize.y - 35.f;

            m_hotbarSlots[i].setPosition(sf::Vector2f(x, y));
            
            // El slot activo de la hotbar se muestra en amarillo
            if (i == m_activeHotbarSlot && i < (int)m_items.size() && m_items[i]) {
                m_hotbarSlots[i].setOutlineColor(sf::Color(255, 200, 0));
                m_hotbarSlots[i].setOutlineThickness(2.f);
            }
            else
            {
                m_hotbarSlots[i].setOutlineColor(sf::Color(70, 70, 70));
                m_hotbarSlots[i].setOutlineThickness(1.f);
            }

            window.draw(m_hotbarSlots[i]);
            
            if (i < (int)m_items.size() && m_items[i]) {
            // Intentar dibujar sprite primero
            if (m_items[i]->sprite) {
                m_items[i]->scaleSprite(35.f, 35.f);
                sf::FloatRect bounds = m_items[i]->sprite->getLocalBounds();
                m_items[i]->sprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
                m_items[i]->sprite->setPosition(sf::Vector2f(x, y));
                window.draw(*m_items[i]->sprite);
            } else {
                // Fallback: rectángulo de color
                sf::RectangleShape itemShape(sf::Vector2f(35.f, 35.f));
                itemShape.setFillColor(m_items[i]->color);
                itemShape.setOutlineThickness(1.f);
                itemShape.setOutlineColor(sf::Color::Black);
                itemShape.setOrigin(sf::Vector2f(17.5f, 17.5f));
                itemShape.setPosition(sf::Vector2f(x, y));
                window.draw(itemShape);
            }
        }
            
            // Dibujar número del slot SÓLO si la fuente es válida
            if (m_fontLoaded) {
                sf::Text slotNum(m_font, std::to_string(i + 1), 10);
                slotNum.setFillColor(sf::Color(150, 150, 150));
                slotNum.setPosition(sf::Vector2f(x - 18.f, y - 18.f));
                window.draw(slotNum);
            }
        }
    }
    
    // ========== DIBUJAR INVENTARIO EXTENDIDO (ABIERTO) ==========
    if (m_isOpen) {
        m_inventoryBg.setPosition(sf::Vector2f(windowSize.x / 2.f, windowSize.y / 2.f));
        window.draw(m_inventoryBg);
        
        // Título (solo si fuente cargada)
        if (m_fontLoaded) {
            sf::Text title(m_font, "INVENTARIO", 18);
            title.setFillColor(sf::Color::White);
            sf::FloatRect titleBounds = title.getLocalBounds();
            title.setOrigin(sf::Vector2f(titleBounds.size.x / 2.f, 0.f));
            title.setPosition(sf::Vector2f(windowSize.x / 2.f, windowSize.y / 2.f - 105.f));
            window.draw(title);
        }
        
        // Instrucciones (solo si m_infoText es válido)
        if (m_infoText) {
            m_infoText->setString("Click: Seleccionar | Arrastrar: Mover | Click der: Eliminar | E: Cerrar");
            sf::FloatRect infoBounds = m_infoText->getLocalBounds();
            m_infoText->setOrigin(sf::Vector2f(infoBounds.size.x / 2.f, 0.f));
            m_infoText->setPosition(sf::Vector2f(windowSize.x / 2.f, windowSize.y / 2.f + 115.f));
            window.draw(*m_infoText);
        }
        
        // Dibujar slots del inventario
        for (int row = 0; row < INVENTORY_ROWS; ++row) {
            for (int col = 0; col < INVENTORY_COLS; ++col) {
                int index = row * INVENTORY_COLS + col;
                float x = windowSize.x / 2.f - 100.f + col * 50.f;
                float y = windowSize.y / 2.f - 40.f + row * 50.f;

                m_inventorySlots[index].setPosition(sf::Vector2f(x, y));

                bool isHovered = m_inventorySlots[index].getGlobalBounds().contains(mousePos);
                
                // MOSTRAR BORDE AMARILLO EN EL SLOT SELECCIONADO (CUALQUIER FILA)
                if (index == m_selectedSlot && index < (int)m_items.size() && m_items[index]) {
                    m_inventorySlots[index].setOutlineColor(sf::Color(255, 200, 0));
                    m_inventorySlots[index].setOutlineThickness(3.f);
                }
                else if (isHovered && !m_isDraggingItem)
                {
                    m_inventorySlots[index].setOutlineColor(sf::Color(150, 150, 150));
                    m_inventorySlots[index].setOutlineThickness(2.f);
                }
                else
                {
                    m_inventorySlots[index].setOutlineColor(sf::Color(70, 70, 70));
                    m_inventorySlots[index].setOutlineThickness(1.f);
                }

                window.draw(m_inventorySlots[index]);
                
                // Dibujar item si existe y NO es el que se está arrastrando
                if (index < (int)m_items.size() && m_items[index]) {
                    if (!m_isDraggingItem || index != m_draggedItemIndex) {
                        // Intentar dibujar sprite primero
                        if (m_items[index]->sprite) {
                            m_items[index]->scaleSprite(35.f, 35.f);
                            sf::FloatRect bounds = m_items[index]->sprite->getLocalBounds();
                            m_items[index]->sprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
                            m_items[index]->sprite->setPosition(sf::Vector2f(x, y));
                            window.draw(*m_items[index]->sprite);
                        } else {
                            // Fallback: rectángulo de color
                            sf::RectangleShape itemShape(sf::Vector2f(35.f, 35.f));
                            itemShape.setFillColor(m_items[index]->color);
                            itemShape.setOutlineThickness(1.f);
                            itemShape.setOutlineColor(sf::Color::Black);
                            itemShape.setOrigin(sf::Vector2f(17.5f, 17.5f));
                            itemShape.setPosition(sf::Vector2f(x, y));
                            window.draw(itemShape);
                        }
                    }
                }
                
                // Dibujar números en la primera fila (hotbar) SÓLO si la fuente es válida
                if (row == 0 && m_fontLoaded) {
                    sf::Text slotNum(m_font, std::to_string(col + 1), 10);
                    slotNum.setFillColor(sf::Color(150, 150, 150));
                    slotNum.setPosition(sf::Vector2f(x - 18.f, y - 18.f));
                    window.draw(slotNum);
                }
            }
        }
        
        // Dibujar el item que se está arrastrando
        if (m_isDraggingItem && m_draggedItemIndex != -1 && 
            m_draggedItemIndex < (int)m_items.size() && m_items[m_draggedItemIndex]) {
            
            if (m_items[m_draggedItemIndex]->sprite) {
                m_items[m_draggedItemIndex]->scaleSprite(45.f, 45.f);
                sf::FloatRect bounds = m_items[m_draggedItemIndex]->sprite->getLocalBounds();
                m_items[m_draggedItemIndex]->sprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
                m_items[m_draggedItemIndex]->sprite->setPosition(mousePos - m_dragOffset);
                window.draw(*m_items[m_draggedItemIndex]->sprite);
            } else {
                sf::RectangleShape itemShape(sf::Vector2f(45.f, 45.f));
                itemShape.setFillColor(m_items[m_draggedItemIndex]->color);
                itemShape.setOutlineThickness(2.f);
                itemShape.setOutlineColor(sf::Color::Yellow);
                itemShape.setOrigin(sf::Vector2f(22.5f, 22.5f));
                itemShape.setPosition(mousePos - m_dragOffset);
                window.draw(itemShape);
            }
        }
    }
}

bool Inventory::tryCollectItem(const std::string &itemName, const sf::Color &color)
{
    if ((int)m_items.size() < INVENTORY_COLS * INVENTORY_ROWS ||
        std::any_of(m_items.begin(), m_items.end(), [](const auto &ptr)
                    { return ptr == nullptr; }))
    {
        addItem({itemName, color});
        return true;
    }
    return false;
}

void Inventory::addDefaultItems()
{
    // Vacío por defecto
}

// OBTENER ITEM POR ÍNDICE - Devuelve el item en la posición dada
// ============================================================
Item* Inventory::getItem(int index) {
    if (index >= 0 && index < (int)m_items.size() && m_items[index]) {
        return m_items[index].get();
    }
    return nullptr;
}

float Inventory::getItemScale(const std::string &name)
{
    if (name == "Rifle")
        return 0.034f;
    if (name == "Gallina")
        return 0.195f;
    if (name == "Dientes")
        return 0.12f;
    if (name == "Llave")
        return 0.077f;
    if (name == "Balon Basket")
        return 0.15f;
    if (name == "Destornillador")
        return 0.15f;
    return 0.5f; // Escala por defecto
}