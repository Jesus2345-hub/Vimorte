#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <memory>
#include <stack>

class State;

class Game {
private:
    std::unique_ptr<sf::RenderWindow> window;
    std::stack<std::unique_ptr<State>> states;

    float volGeneral = 100.f;
    float volMusica = 50.f;
    float volEfectos = 80.f;

public:
    Game();
    ~Game();
    void run();
    
    void setVolGeneral(float v) { volGeneral = v; }
    void setVolMusica(float v) { volMusica = v; }
    void setVolEfectos(float v) { volEfectos = v; }
    
    float getVolGeneral() const { return volGeneral; }
    float getVolMusica() const { return volMusica; }
    float getVolEfectos() const { return volEfectos; }
    float getRealMusica() const { return (volMusica * volGeneral) / 100.f; }

    void pushState(std::unique_ptr<State> state);
    void popState();
    void changeState(std::unique_ptr<State> state);
    void returnToMenu(); // Para salir desde la pausa al menú principal
};

#endif