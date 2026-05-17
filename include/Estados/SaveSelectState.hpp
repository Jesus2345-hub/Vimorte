#pragma once
#include "Estados/State.hpp"
#include "Administradores/GameSaveManager.hpp"
#include "Estados/VideoFinalState.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

class SaveSelectState : public State {
private:
    GameSaveManager& saveManager;
    std::vector<SaveSlotInfo> slots;
    
    sf::Font m_font;
    sf::RectangleShape m_background;
    sf::RectangleShape m_panel;
    
    std::unique_ptr<sf::Text> m_title;
    std::vector<std::unique_ptr<sf::Text>> m_slotTexts;
    std::vector<sf::RectangleShape> m_slotBoxes;
    std::vector<bool> m_slotHover;
    
    std::unique_ptr<sf::Text> m_instructionText;
    std::unique_ptr<sf::Text> m_selectedSlotText;
    
    bool m_modoNuevaPartida = false;
    bool m_soloCarga = false;
    std::string m_nombreInput;
    std::unique_ptr<sf::Text> m_inputText;
    int m_slotSeleccionado = -1;
    int m_slotSeleccionadoParaEliminar = -1; 
    
    // Botones de acción
    sf::RectangleShape m_btnEliminar;
    std::unique_ptr<sf::Text> m_btnEliminarText;
    bool m_btnEliminarHover = false;
    
public:
    SaveSelectState(sf::RenderWindow* window, Game* game, bool soloCarga = false);
    
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event& event) override;
    
private:

    void actualizarUI();
    void seleccionarSlot(int slotId);
    void ejecutarAccionSlot(int slotId);
    void iniciarNuevaPartida(int slotId);
    void cargarPartidaExistente(int slotId);
    void eliminarPartidaSeleccionada();
    void dibujarTecladoVirtual(sf::RenderWindow& window);
};