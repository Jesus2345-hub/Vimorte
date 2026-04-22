#include "GameSaveManager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

// ===== GameProgressData =====
bool GameProgressData::guardarEnArchivo(int slotId) {
    std::string archivo = "saves/slot_" + std::to_string(slotId) + ".csv";
    std::ofstream file(archivo);
    if (!file.is_open()) {
        std::cerr << "❌ Error: No se pudo guardar en " << archivo << std::endl;
        return false;
    }
    
    file << "nivel_actual,nodo_actual,centinela_disponible,centinela_id,ruta_actual,nombre,tiempo,muertes,modo_juego,checkpoint_ruta,checkpoint_centinela\n";
    file << nivelActualId << ","
         << nodoActualId << ","
         << (centinelaDisponible ? "true" : "false") << ","
         << centinelaId << ","
         << rutaActual << ","
         << nombreJugador << ","
         << tiempoJugado << ","
         << muertes << ","
         << static_cast<int>(modoElegido) << ","
         << checkpointRutaArbol << ","
         << checkpointCentinelaId << "\n";
    
    // Guardar items
    file << "#ITEMS\n";
    for (const auto& item : itemsRecolectados) {
        file << item << "\n";
    }
    
    std::cout << "✅ Progreso guardado en slot " << slotId << std::endl;
    return true;
}

bool GameProgressData::cargarDesdeArchivo(int slotId) {
    std::string archivo = "saves/slot_" + std::to_string(slotId) + ".csv";
    std::ifstream file(archivo);
    if (!file.is_open()) {
        std::cerr << "❌ Error: No se pudo cargar " << archivo << std::endl;
        return false;
    }
    
    std::string linea;
    // Saltar cabecera
    std::getline(file, linea);
    // Leer datos principales
    if (std::getline(file, linea)) {
        std::stringstream ss(linea);
        std::string token;
        
        std::getline(ss, token, ','); nivelActualId = std::stoi(token);
        std::getline(ss, token, ','); nodoActualId = std::stoi(token);
        std::getline(ss, token, ','); centinelaDisponible = (token == "true");
        std::getline(ss, token, ','); centinelaId = std::stoi(token);
        std::getline(ss, token, ','); rutaActual = token;
        std::getline(ss, token, ','); nombreJugador = token;
        std::getline(ss, token, ','); tiempoJugado = std::stof(token);
        std::getline(ss, token, ','); muertes = std::stoi(token);
        
        // Intentar leer modo de juego (para compatibilidad con saves antiguos)
        if (std::getline(ss, token, ',')) {
            modoElegido = static_cast<ModoJuego>(std::stoi(token));
        }
        if (std::getline(ss, token, ',')) {
            checkpointRutaArbol = token;
        }
        if (std::getline(ss, token, ',')) {
            checkpointCentinelaId = token;
        }
    }
    
    // Leer items
    itemsRecolectados.clear();
    bool leyendoItems = false;
    while (std::getline(file, linea)) {
        if (linea == "#ITEMS") {
            leyendoItems = true;
            continue;
        }
        if (leyendoItems && !linea.empty()) {
            itemsRecolectados.push_back(linea);
        }
    }
    
    std::cout << "✅ Progreso cargado desde slot " << slotId << " (Nivel " << nivelActualId << ")" << std::endl;
    std::cout << "   Modo de juego: " << static_cast<int>(modoElegido) << std::endl;
    return true;
}

// ===== GameSaveManager =====
GameSaveManager::GameSaveManager() : currentSlotId(-1) {
    // Constructor implementado
}

std::vector<SaveSlotInfo> GameSaveManager::getSlotsDisponibles() {
    return indexManager.getSlotsDisponibles();
}

bool GameSaveManager::crearNuevaPartida(int slotId, const std::string& nombre) {
    if (!indexManager.crearNuevaPartida(slotId, nombre)) {
        std::cerr << "❌ Error: No se pudo crear nueva partida en slot " << slotId << std::endl;
        return false;
    }
    
    currentSlotId = slotId;
    currentProgress = GameProgressData();
    currentProgress.nombreJugador = nombre;
    currentProgress.rutaActual = "nivel1";  // Ruta inicial
    currentProgress.modoElegido = GameProgressData::ModoJuego::NO_ELEGIDO;
    
    std::cout << "✅ Nueva partida creada: '" << nombre << "' en slot " << slotId << std::endl;
    return guardarProgresoActual();
}

bool GameSaveManager::cargarPartida(int slotId) {
    if (!currentProgress.cargarDesdeArchivo(slotId)) {
        std::cerr << "❌ Error: No se pudo cargar partida del slot " << slotId << std::endl;
        return false;
    }
    
    currentSlotId = slotId;
    std::cout << "✅ Partida cargada: '" << currentProgress.nombreJugador << "' desde slot " << slotId << std::endl;
    return true;
}

bool GameSaveManager::guardarProgresoActual() {
    if (currentSlotId < 0) {
        std::cerr << "⚠️ Advertencia: No hay slot activo para guardar" << std::endl;
        return false;
    }
    
    if (!currentProgress.guardarEnArchivo(currentSlotId)) {
        return false;
    }
    
    return indexManager.actualizarSlot(
        currentSlotId,
        currentProgress.nivelActualId,
        currentProgress.centinelaDisponible,
        currentProgress.rutaActual
    );
}

bool GameSaveManager::eliminarPartida(int slotId) {
    if (currentSlotId == slotId) {
        currentSlotId = -1;
        currentProgress = GameProgressData();
    }
    
    std::cout << "🗑️ Partida eliminada del slot " << slotId << std::endl;
    return indexManager.eliminarPartida(slotId);
}

void GameSaveManager::setNivelActual(int nivelId, int nodoId) {
    currentProgress.nivelActualId = nivelId;
    currentProgress.nodoActualId = nodoId;
    guardarProgresoActual();
}

void GameSaveManager::setCentinelaDisponible(bool disponible, int centinelaId) {
    currentProgress.centinelaDisponible = disponible;
    currentProgress.centinelaId = centinelaId;
    guardarProgresoActual();
}

void GameSaveManager::addItemRecolectado(const std::string& itemName) {
    currentProgress.itemsRecolectados.push_back(itemName);
    guardarProgresoActual();
    std::cout << "📦 Item guardado: " << itemName << std::endl;
}

void GameSaveManager::addTiempoJugado(float dt) {
    currentProgress.tiempoJugado += dt;
}

void GameSaveManager::addMuerte() {
    currentProgress.muertes++;
    guardarProgresoActual();
}

void GameSaveManager::setRutaActual(const std::string& ruta) {
    currentProgress.rutaActual = ruta;
    guardarProgresoActual();
}