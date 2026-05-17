#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <algorithm>

struct SaveSlotInfo {
    int slotId;
    std::string nombrePartida;
    int nivelActual;
    std::string fechaGuardado;
    std::string rutaActual;
    bool tieneCentinela;
    
    // Constructor implementado
    SaveSlotInfo() : slotId(-1), nombrePartida("[VACIO]"), nivelActual(1), 
                     fechaGuardado("-"), rutaActual("principal"), tieneCentinela(false) {}
    
    std::string toString() const;
    static SaveSlotInfo fromString(const std::string& linea);
};

class SaveIndexManager {
private:
    std::vector<SaveSlotInfo> slots;
    const int MAX_SLOTS = 5;
    std::string indexFile = "saves/index.csv";
    
    std::string obtenerFechaActual();
    
public:
    SaveIndexManager();
    bool cargarIndice();
    bool guardarIndice();
    std::vector<SaveSlotInfo> getSlotsDisponibles();
    bool crearNuevaPartida(int slotId, const std::string& nombre);
    bool actualizarSlot(int slotId, int nivelActual, bool tieneCentinela, const std::string& ruta);
    bool eliminarPartida(int slotId);
    SaveSlotInfo getSlotInfo(int slotId);
};