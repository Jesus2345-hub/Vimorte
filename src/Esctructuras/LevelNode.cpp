#include "Estructuras/LevelNode.hpp"

LevelNode::LevelNode(const std::string& id_, const std::string& displayName_, 
                     LevelType type_, int levelNum_)
    : id(id_), displayName(displayName_), type(type_), levelNumber(levelNum_),
      left(nullptr), right(nullptr), stateFactory(nullptr) {
}