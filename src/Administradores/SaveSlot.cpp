#include "Administradores/SaveSlot.hpp"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

// ===== SaveSlotInfo (Implementación de métodos) =====
std::string SaveSlotInfo::toString() const {
    std::stringstream ss;
    ss << slotId << ","
       << nombrePartida << ","
       << nivelActual << ","
       << fechaGuardado << ","
       << rutaActual << ","
       << (tieneCentinela ? "true" : "false");
    return ss.str();
}

SaveSlotInfo SaveSlotInfo::fromString(const std::string& linea) {
    SaveSlotInfo info;
    std::stringstream ss(linea);
    std::string token;
    
    std::getline(ss, token, ','); info.slotId = std::stoi(token);
    std::getline(ss, token, ','); info.nombrePartida = token;
    std::getline(ss, token, ','); info.nivelActual = std::stoi(token);
    std::getline(ss, token, ','); info.fechaGuardado = token;
    std::getline(ss, token, ','); info.rutaActual = token;
    std::getline(ss, token, ','); info.tieneCentinela = (token == "true");
    
    return info;
}

// ===== SaveIndexManager =====
SaveIndexManager::SaveIndexManager() {
    cargarIndice();
}

std::string SaveIndexManager::obtenerFechaActual() {
    auto ahora = std::chrono::system_clock::now();
    auto tiempo = std::chrono::system_clock::to_time_t(ahora);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&tiempo), "%Y-%m-%d %H:%M");
    return ss.str();
}

bool SaveIndexManager::cargarIndice() {
    slots.clear();
    std::ifstream file(indexFile);
    if (!file.is_open()) {
        // Crear directorio si no existe
        #ifdef _WIN32
            system("mkdir saves 2> nul");
        #else
            system("mkdir -p saves");
        #endif
        return false;
    }
    
    std::string linea;
    // Saltar cabecera
    std::getline(file, linea);
    
    while (std::getline(file, linea)) {
        if (!linea.empty()) {
            slots.push_back(SaveSlotInfo::fromString(linea));
        }
    }
    
    return true;
}

bool SaveIndexManager::guardarIndice() {
    std::ofstream file(indexFile);
    if (!file.is_open()) return false;
    
    file << "slot_id,nombre,nivel,fecha,ruta,tiene_centinela\n";
    for (const auto& slot : slots) {
        file << slot.toString() << "\n";
    }
    
    return true;
}

std::vector<SaveSlotInfo> SaveIndexManager::getSlotsDisponibles() {
    std::vector<SaveSlotInfo> disponibles;
    
    // Marcar slots ocupados
    std::vector<bool> ocupados(MAX_SLOTS, false);
    for (const auto& slot : slots) {
        if (slot.slotId >= 0 && slot.slotId < MAX_SLOTS) {
            ocupados[slot.slotId] = true;
        }
    }
    
    // Crear lista de slots (ocupados y vacíos)
    for (int i = 0; i < MAX_SLOTS; i++) {
        SaveSlotInfo info;
        info.slotId = i;
        
        // Buscar si este slot tiene datos
        auto it = std::find_if(slots.begin(), slots.end(), 
            [i](const SaveSlotInfo& s) { return s.slotId == i; });
        
        if (it != slots.end()) {
            info = *it;
        } else {
            info.nombrePartida = "[VACIO]";
            info.nivelActual = 1;
            info.fechaGuardado = "-";
        }
        
        disponibles.push_back(info);
    }
    
    return disponibles;
}

bool SaveIndexManager::crearNuevaPartida(int slotId, const std::string& nombre) {
    if (slotId < 0 || slotId >= MAX_SLOTS) return false;
    
    SaveSlotInfo nuevaPartida;
    nuevaPartida.slotId = slotId;
    nuevaPartida.nombrePartida = nombre;
    nuevaPartida.nivelActual = 1;
    nuevaPartida.fechaGuardado = obtenerFechaActual();
    nuevaPartida.rutaActual = "principal";
    nuevaPartida.tieneCentinela = false;
    
    // Eliminar slot existente si hay
    slots.erase(std::remove_if(slots.begin(), slots.end(),
        [slotId](const SaveSlotInfo& s) { return s.slotId == slotId; }), slots.end());
    
    slots.push_back(nuevaPartida);
    return guardarIndice();
}

bool SaveIndexManager::actualizarSlot(int slotId, int nivelActual, bool tieneCentinela, const std::string& ruta) {
    auto it = std::find_if(slots.begin(), slots.end(),
        [slotId](const SaveSlotInfo& s) { return s.slotId == slotId; });
    
    if (it != slots.end()) {
        it->nivelActual = nivelActual;
        it->fechaGuardado = obtenerFechaActual();
        it->rutaActual = ruta;
        it->tieneCentinela = tieneCentinela;
        return guardarIndice();
    }
    
    return false;
}

bool SaveIndexManager::eliminarPartida(int slotId) {
    slots.erase(std::remove_if(slots.begin(), slots.end(),
        [slotId](const SaveSlotInfo& s) { return s.slotId == slotId; }), slots.end());
    
    // Eliminar archivo de guardado
    std::string archivoSlot = "saves/slot_" + std::to_string(slotId) + ".csv";
    std::remove(archivoSlot.c_str());
    
    return guardarIndice();
}

SaveSlotInfo SaveIndexManager::getSlotInfo(int slotId) {
    auto it = std::find_if(slots.begin(), slots.end(),
        [slotId](const SaveSlotInfo& s) { return s.slotId == slotId; });
    
    if (it != slots.end()) {
        return *it;
    }
    
    return SaveSlotInfo();
}