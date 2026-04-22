#pragma once
#include "SaveSlot.hpp"
#include <fstream>
#include <sstream>
#include <vector>

struct GameProgressData {
    int nivelActualId = 1;
    int nodoActualId = 1;
    bool centinelaDisponible = false;
    int centinelaId = 0;
    std::string rutaActual = "principal";
    std::string nombreJugador = "Jugador";
    float tiempoJugado = 0.0f;
    int muertes = 0;
    std::vector<std::string> itemsRecolectados;
    
    // === NUEVO: Modo de dificultad / elección de camino ===
    enum class ModoJuego {
        NO_ELEGIDO = 0,
        CAMINO_AGRADABLE = 1,      // Puede reintentar centinelas
        CAMINO_CON_CONSECUENCIAS = 2  // Sin reintentos, decisiones permanentes
    };
    ModoJuego modoElegido = ModoJuego::NO_ELEGIDO;
    
    // === NUEVO: Punto de guardado antes de entrar a un centinela (solo para modo agradable) ===
    bool tieneCheckpointCentinela = false;
    std::string checkpointRutaArbol;      // Ruta antes de entrar al centinela
    std::string checkpointCentinelaId;    // ID del centinela en el que se está
    
    bool guardarEnArchivo(int slotId);
    bool cargarDesdeArchivo(int slotId);
};

class GameSaveManager {
private:
    SaveIndexManager indexManager;
    GameProgressData currentProgress;
    int currentSlotId = -1;
    
public:
    GameSaveManager();
    
    std::vector<SaveSlotInfo> getSlotsDisponibles();
    bool crearNuevaPartida(int slotId, const std::string& nombre);
    bool cargarPartida(int slotId);
    bool guardarProgresoActual();
    bool eliminarPartida(int slotId);
    
    GameProgressData& getCurrentProgress() { return currentProgress; }
    int getCurrentSlotId() const { return currentSlotId; }
    
    void setNivelActual(int nivelId, int nodoId);
    void setCentinelaDisponible(bool disponible, int centinelaId = 0);
    void addItemRecolectado(const std::string& itemName);
    void addMuerte();
    void addTiempoJugado(float dt);
    void setRutaActual(const std::string& ruta);
};