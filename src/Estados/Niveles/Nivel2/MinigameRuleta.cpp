#include "Estados/Niveles/Nivel2/MinigameRuleta.hpp"
#include <iostream>
#include <algorithm>

MinigameRuleta::MinigameRuleta() 
    : m_isActive(false), m_gen(m_rd())
{
    
    // Cargar fuente
    m_fontLoaded = m_font.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf");
    if (!m_fontLoaded) {
        std::cerr << "Error cargando fuente en MinigameRuleta" << std::endl;
        return;
    }
    
    inicializarUI();
}

void MinigameRuleta::setPosition(const sf::Vector2f& pos) {
    m_position = pos;
    m_background.setPosition(pos);
}

void MinigameRuleta::setSize(const sf::Vector2f& size) {
    m_size = size;
    m_background.setSize(size);
}

void MinigameRuleta::inicializarUI() {
    if (!m_fontLoaded) return;
    
    // Título elegante 
    m_tituloText = std::make_unique<sf::Text>(m_font, "RULETA VIMORTE", 32);
    m_tituloText->setFillColor(sf::Color(255, 215, 0));
    m_tituloText->setStyle(sf::Text::Bold);
    m_tituloText->setOutlineThickness(2.f);
    m_tituloText->setOutlineColor(sf::Color(139, 69, 19));
    
    // Instrucciones 
    m_instruccionesText = std::make_unique<sf::Text>(m_font, 
        "PASO 1: Elige tipo de apuesta  |  PASO 2: Selecciona  |  PASO 3: GIRA!\n"
        "Apuesta minima: $5  |  ESC para salir",
    11);
    m_instruccionesText->setFillColor(sf::Color(200, 180, 150));
    
    // Texto de apuesta
    m_apuestaText = std::make_unique<sf::Text>(m_font, "Apuesta: $5", 22);
    m_apuestaText->setFillColor(sf::Color(255, 215, 0));
    m_apuestaText->setStyle(sf::Text::Bold);
    
    // Texto de resultado
    m_resultadoText = std::make_unique<sf::Text>(m_font, "", 28);
    m_resultadoText->setFillColor(sf::Color::White);
    
    // Texto de dinero
    m_dineroText = std::make_unique<sf::Text>(m_font, "", 20);
    m_dineroText->setFillColor(sf::Color(255, 215, 0));
    m_dineroText->setStyle(sf::Text::Bold);
    
    // ===== ESTILO BASE PARA BOTONES =====
    auto crearBotonElegante = [](sf::RectangleShape& btn, float width, float height,
                                  sf::Color fillColor, sf::Color borderColor,
                                  float borderThickness = 2.5f) {
        btn.setSize(sf::Vector2f(width, height));
        btn.setFillColor(fillColor);
        btn.setOutlineThickness(borderThickness);
        btn.setOutlineColor(borderColor);
    };
    
    // ===== BOTONES DE TIPO DE APUESTA =====
    float btnWidth = 200.f;
    float btnHeight = 50.f;
    float btnSpacing = 15.f;
    
    // Botón Número Exacto
    crearBotonElegante(m_btnNumeroExacto, btnWidth, btnHeight,
                       sf::Color(60, 20, 10, 220), sf::Color(180, 120, 60));
    m_textoBtnExacto = std::make_unique<sf::Text>(m_font, "NUMERO EXACTO (x35)", 14);
    m_textoBtnExacto->setFillColor(sf::Color(255, 200, 150));
    m_textoBtnExacto->setStyle(sf::Text::Bold);
    
    // Botón Cuarto
    crearBotonElegante(m_btnCuarto, btnWidth, btnHeight,
                       sf::Color(20, 50, 20, 220), sf::Color(100, 180, 100));
    m_textoBtnCuarto = std::make_unique<sf::Text>(m_font, "CUARTO (x3)", 14);
    m_textoBtnCuarto->setFillColor(sf::Color(180, 255, 180));
    m_textoBtnCuarto->setStyle(sf::Text::Bold);
    
    // Botón Color
    crearBotonElegante(m_btnColor, btnWidth, btnHeight,
                       sf::Color(20, 20, 60, 220), sf::Color(100, 100, 200));
    m_textoBtnColor = std::make_unique<sf::Text>(m_font, "COLOR (x2)", 14);
    m_textoBtnColor->setFillColor(sf::Color(180, 200, 255));
    m_textoBtnColor->setStyle(sf::Text::Bold);
    
    // Botón Par/Impar
    crearBotonElegante(m_btnParImpar, btnWidth, btnHeight,
                       sf::Color(50, 15, 50, 220), sf::Color(180, 100, 200));
    m_textoBtnParImpar = std::make_unique<sf::Text>(m_font, "PAR/IMPAR (x1)", 14);
    m_textoBtnParImpar->setFillColor(sf::Color(220, 180, 255));
    m_textoBtnParImpar->setStyle(sf::Text::Bold);
    
    // ===== BOTONES DE APUESTA (+/-) =====
    float btnApuestaSize = 45.f;
    
    crearBotonElegante(m_btnDisminuir, btnApuestaSize, btnApuestaSize,
                       sf::Color(80, 20, 20, 220), sf::Color(200, 100, 80));
    m_textoBtnDisminuir = std::make_unique<sf::Text>(m_font, "-", 24);
    m_textoBtnDisminuir->setFillColor(sf::Color(255, 150, 130));
    m_textoBtnDisminuir->setStyle(sf::Text::Bold);
    
    crearBotonElegante(m_btnAumentar, btnApuestaSize, btnApuestaSize,
                       sf::Color(20, 60, 20, 220), sf::Color(100, 200, 100));
    m_textoBtnAumentar = std::make_unique<sf::Text>(m_font, "+", 24);
    m_textoBtnAumentar->setFillColor(sf::Color(150, 255, 150));
    m_textoBtnAumentar->setStyle(sf::Text::Bold);
    
    // ===== BOTÓN GIRAR =====
    crearBotonElegante(m_btnGirar, btnWidth * 2 + btnSpacing, 55.f,
                       sf::Color(180, 130, 20, 220), sf::Color(255, 215, 0), 3.f);
    m_textoBtnGirar = std::make_unique<sf::Text>(m_font, "GIRAR", 26);
    m_textoBtnGirar->setFillColor(sf::Color(255, 255, 200));
    m_textoBtnGirar->setStyle(sf::Text::Bold);
    
    // ===== SELECTOR DE NÚMEROS (0-36) =====
    for (int i = 0; i <= 36; i++) {
        sf::RectangleShape btn(sf::Vector2f(42.f, 34.f));
        btn.setOutlineThickness(1.5f);
        m_botonesNumeros.push_back(btn);
        
        auto texto = std::make_unique<sf::Text>(m_font, std::to_string(i), 15);
        texto->setStyle(sf::Text::Bold);
        m_textosNumeros.push_back(std::move(texto));
        m_hoverNumeros.push_back(false);
    }
    
    // ===== SELECTOR DE CUARTOS =====
    auto crearSelector = [&crearBotonElegante](sf::RectangleShape& btn) {
        crearBotonElegante(btn, 240.f, 40.f,
                          sf::Color(30, 30, 30, 220), sf::Color(180, 160, 120));
    };
    
    crearSelector(m_btnCuarto1);
    crearSelector(m_btnCuarto2);
    crearSelector(m_btnCuarto3);
    
    m_textoCuarto1 = std::make_unique<sf::Text>(m_font, "1er Cuarto (1-12)", 16);
    m_textoCuarto1->setFillColor(sf::Color(200, 200, 180));
    m_textoCuarto2 = std::make_unique<sf::Text>(m_font, "2do Cuarto (13-24)", 16);
    m_textoCuarto2->setFillColor(sf::Color(200, 200, 180));
    m_textoCuarto3 = std::make_unique<sf::Text>(m_font, "3er Cuarto (25-36)", 16);
    m_textoCuarto3->setFillColor(sf::Color(200, 200, 180));
    
    // ===== SELECTOR DE COLORES =====
    crearBotonElegante(m_btnRojo, 240.f, 40.f,
                       sf::Color(140, 10, 10, 220), sf::Color(255, 100, 100));
    crearBotonElegante(m_btnNegro, 240.f, 40.f,
                       sf::Color(20, 20, 20, 220), sf::Color(180, 180, 180));
    crearBotonElegante(m_btnVerde, 240.f, 40.f,
                       sf::Color(10, 80, 10, 220), sf::Color(100, 255, 100));
    
    m_textoRojo = std::make_unique<sf::Text>(m_font, "ROJO (x2)", 16);
    m_textoRojo->setFillColor(sf::Color(255, 200, 200));
    m_textoNegro = std::make_unique<sf::Text>(m_font, "NEGRO (x2)", 16);
    m_textoNegro->setFillColor(sf::Color(200, 200, 200));
    m_textoVerde = std::make_unique<sf::Text>(m_font, "VERDE (x36)", 16);
    m_textoVerde->setFillColor(sf::Color(200, 255, 200));
    
    // ===== SELECTOR DE PAR/IMPAR =====
    crearSelector(m_btnPar);
    crearSelector(m_btnImpar);
    
    m_textoPar = std::make_unique<sf::Text>(m_font, "PAR", 16);
    m_textoPar->setFillColor(sf::Color(200, 200, 255));
    m_textoImpar = std::make_unique<sf::Text>(m_font, "IMPAR", 16);
    m_textoImpar->setFillColor(sf::Color(255, 200, 150));

    // ===== INICIALIZAR TAPETE INTERACTIVO =====
    inicializarTapete();
    
    // ===== NUEVOS BOTONES DE APUESTA (visibles para ajustar) =====
    // Docenas
    float btnDocenaWidth = 200.f;
    float btnDocenaHeight = 40.f;
    
    crearBotonElegante(m_btnDocena1, btnDocenaWidth, btnDocenaHeight,
                       sf::Color(60, 30, 10, 220), sf::Color(180, 120, 60));
    if (!m_textoDocena1) m_textoDocena1 = std::make_unique<sf::Text>(m_font, "1ra DOCENA (1-12) x2", 14);
    m_textoDocena1->setFillColor(sf::Color(255, 200, 150));
    m_textoDocena1->setStyle(sf::Text::Bold);
    
    crearBotonElegante(m_btnDocena2, btnDocenaWidth, btnDocenaHeight,
                       sf::Color(60, 30, 10, 220), sf::Color(180, 120, 60));
    if (!m_textoDocena2) m_textoDocena2 = std::make_unique<sf::Text>(m_font, "2da DOCENA (13-24) x2", 14);
    m_textoDocena2->setFillColor(sf::Color(255, 200, 150));
    m_textoDocena2->setStyle(sf::Text::Bold);
    
    crearBotonElegante(m_btnDocena3, btnDocenaWidth, btnDocenaHeight,
                       sf::Color(60, 30, 10, 220), sf::Color(180, 120, 60));
    if (!m_textoDocena3) m_textoDocena3 = std::make_unique<sf::Text>(m_font, "3ra DOCENA (25-36) x2", 14);
    m_textoDocena3->setFillColor(sf::Color(255, 200, 150));
    m_textoDocena3->setStyle(sf::Text::Bold);
    
    // Columnas
    crearBotonElegante(m_btnColumna1, btnDocenaWidth, btnDocenaHeight,
                       sf::Color(20, 50, 20, 220), sf::Color(100, 180, 100));
    if (!m_textoColumna1) m_textoColumna1 = std::make_unique<sf::Text>(m_font, "COLUMNA 1 (x2)", 14);
    m_textoColumna1->setFillColor(sf::Color(180, 255, 180));
    m_textoColumna1->setStyle(sf::Text::Bold);
    
    crearBotonElegante(m_btnColumna2, btnDocenaWidth, btnDocenaHeight,
                       sf::Color(20, 50, 20, 220), sf::Color(100, 180, 100));
    if (!m_textoColumna2) m_textoColumna2 = std::make_unique<sf::Text>(m_font, "COLUMNA 2 (x2)", 14);
    m_textoColumna2->setFillColor(sf::Color(180, 255, 180));
    m_textoColumna2->setStyle(sf::Text::Bold);
    
    crearBotonElegante(m_btnColumna3, btnDocenaWidth, btnDocenaHeight,
                       sf::Color(20, 50, 20, 220), sf::Color(100, 180, 100));
    if (!m_textoColumna3) m_textoColumna3 = std::make_unique<sf::Text>(m_font, "COLUMNA 3 (x2)", 14);
    m_textoColumna3->setFillColor(sf::Color(180, 255, 180));
    m_textoColumna3->setStyle(sf::Text::Bold);
    
    // 1-18 / 19-36
    crearBotonElegante(m_btnMitadBaja, btnDocenaWidth, btnDocenaHeight,
                       sf::Color(50, 15, 50, 220), sf::Color(180, 100, 200));
    if (!m_textoMitadBaja) m_textoMitadBaja = std::make_unique<sf::Text>(m_font, "1-18 (x1)", 14);
    m_textoMitadBaja->setFillColor(sf::Color(220, 180, 255));
    m_textoMitadBaja->setStyle(sf::Text::Bold);
    
    crearBotonElegante(m_btnMitadAlta, btnDocenaWidth, btnDocenaHeight,
                       sf::Color(50, 15, 50, 220), sf::Color(180, 100, 200));
    if (!m_textoMitadAlta) m_textoMitadAlta = std::make_unique<sf::Text>(m_font, "19-36 (x1)", 14);
    m_textoMitadAlta->setFillColor(sf::Color(220, 180, 255));
    m_textoMitadAlta->setStyle(sf::Text::Bold);

}


void MinigameRuleta::activate() {
    m_isActive = true;
    m_tipoApuesta = TipoApuesta::NINGUNA;
    m_numeroElegido = -1;
    m_cuartoElegido = -1;
    m_colorElegido = ColorElegido::NINGUNO;
    m_parImparElegido = -1;
    m_columnaElegida = -1;      // NUEVO
    m_mitadElegida = -1;        // NUEVO
    m_apuesta = 5;
    m_mostrandoResultado = false;
    m_mostrandoTableroNumeros = false;
    m_mostrandoSelectorCuarto = false;
    m_mostrandoSelectorColor = false;
    m_mostrandoSelectorParImpar = false;
    m_mostrandoTapete = true;   // NUEVO: siempre visible
    
    inicializarRuletaAnimada();
    
    if (!m_ruletaSprite) {
        if (m_ruletaTexture.loadFromFile("assets/images/niveles/nivel2/ruleta.jpg")) {
            m_ruletaSprite = std::make_unique<sf::Sprite>(m_ruletaTexture);
        }
    }
}

void MinigameRuleta::deactivate() {
    m_isActive = false;
}

bool MinigameRuleta::esRojo(int numero) {
    // Números rojos en la ruleta clásica
    int rojos[] = {1, 3, 5, 7, 9, 12, 14, 16, 18, 19, 21, 23, 25, 27, 30, 32, 34, 36};
    for (int r : rojos) {
        if (r == numero) return true;
    }
    return false;
}

bool MinigameRuleta::esNegro(int numero) {
    if (numero == 0) return false;
    return !esRojo(numero);
}

int MinigameRuleta::obtenerCuarto(int numero) {
    if (numero == 0) return -1;
    if (numero <= 12) return 0;
    if (numero <= 24) return 1;
    return 2;
}

void MinigameRuleta::girarRuleta() {
    // Verificar que haya apuesta válida
    if (m_tipoApuesta == TipoApuesta::NINGUNA) return;
    if (!m_dineroJugador) return;
    if (*m_dineroJugador < m_apuesta) return;
    
    // Verificar selección según tipo
    if (m_tipoApuesta == TipoApuesta::NUMERO_EXACTO && m_numeroElegido < 0) return;
    if (m_tipoApuesta == TipoApuesta::CUARTO && m_cuartoElegido < 0) return;
    if (m_tipoApuesta == TipoApuesta::COLUMNA && m_columnaElegida < 0) return;
    if (m_tipoApuesta == TipoApuesta::COLOR && m_colorElegido == ColorElegido::NINGUNO) return;
    if (m_tipoApuesta == TipoApuesta::PAR_IMPAR && m_parImparElegido < 0) return;
    if (m_tipoApuesta == TipoApuesta::MANQUE_PASSE && m_mitadElegida < 0) return;
    
    // Cobrar apuesta
    *m_dineroJugador -= m_apuesta;
    
    // Generar número ganador (0-36)
    std::uniform_int_distribution<int> dist(0, 36);
    m_numeroGanador = dist(m_gen);
    
    // Calcular ángulo objetivo de la pelotita
    int indiceGanador = 0;
    for (int i = 0; i < 37; i++) {
        if (m_ordenRuleta[i] == m_numeroGanador) {
            indiceGanador = i;
            break;
        }
    }
    float anguloObjetivo = indiceGanador * (360.f / 37.f) + (360.f / 74.f);

    // Iniciar animación de la pelotita (varias vueltas + ángulo final)
    m_pelotitaGirando = true;
    m_anguloPelotita = 0.f;
    m_velocidadPelotita = 1500.f;
    m_anguloObjetivoPelotita = anguloObjetivo + 1800.f;
    
    // Iniciar animación de giro
    m_girando = true;
    m_velocidadGiro = 720.f + (rand() % 720);
    m_anguloObjetivo = m_numeroGanador * (360.f / 37.f);
    
    // Calcular ganancia
    int ganancia = m_apuesta;
    
    switch (m_tipoApuesta) {
        case TipoApuesta::NUMERO_EXACTO:
            if (m_numeroElegido == m_numeroGanador) {
                ganancia = m_apuesta * 35;
                *m_dineroJugador += ganancia;
                m_mensajeResultado = "GANASTE! $" + std::to_string(ganancia) + 
                    "\nNumero: " + std::to_string(m_numeroGanador);
            } else {
                m_mensajeResultado = "Perdiste... Numero: " + std::to_string(m_numeroGanador) +
                    "\nTu numero: " + std::to_string(m_numeroElegido);
            }
            break;
            
        case TipoApuesta::CUARTO: {
            int cuartoGanador = obtenerCuarto(m_numeroGanador);
            if (cuartoGanador == m_cuartoElegido) {
                ganancia = m_apuesta * 3;
                *m_dineroJugador += ganancia;
                m_mensajeResultado = "GANASTE! $" + std::to_string(ganancia) +
                    "\nNumero: " + std::to_string(m_numeroGanador);
            } else if (m_numeroGanador == 0) {
                m_mensajeResultado = "Cayo 0 verde. Perdiste.";
            } else {
                m_mensajeResultado = "Perdiste... Numero: " + std::to_string(m_numeroGanador);
            }
            break;
        }
        
        case TipoApuesta::COLUMNA: {  // NUEVO
            if (m_numeroGanador == 0) {
                m_mensajeResultado = "Cayo 0 verde. Perdiste.";
            } else {
                int columnaGanadora = obtenerColumna(m_numeroGanador);
                if (columnaGanadora == m_columnaElegida) {
                    ganancia = m_apuesta * 3; // 2:1 (apuesta + ganancia)
                    *m_dineroJugador += ganancia;
                    m_mensajeResultado = "GANASTE! $" + std::to_string(ganancia) +
                        "\nNumero: " + std::to_string(m_numeroGanador) + " (Columna " + std::to_string(m_columnaElegida + 1) + ")";
                } else {
                    m_mensajeResultado = "Perdiste... Numero: " + std::to_string(m_numeroGanador);
                }
            }
            break;
        }
        
        case TipoApuesta::COLOR: {
            ColorElegido colorGanador;
            if (m_numeroGanador == 0) {
                colorGanador = ColorElegido::VERDE;
            } else if (esRojo(m_numeroGanador)) {
                colorGanador = ColorElegido::ROJO;
            } else {
                colorGanador = ColorElegido::NEGRO;
            }
            
            if (colorGanador == m_colorElegido) {
                if (m_colorElegido == ColorElegido::VERDE) {
                    ganancia = m_apuesta * 36;
                } else {
                    ganancia = m_apuesta * 2;
                }
                *m_dineroJugador += ganancia;
                m_mensajeResultado = "GANASTE! $" + std::to_string(ganancia) +
                    "\nNumero: " + std::to_string(m_numeroGanador);
            } else {
                m_mensajeResultado = "Perdiste... Numero: " + std::to_string(m_numeroGanador);
            }
            break;
        }
        
        case TipoApuesta::PAR_IMPAR: {
            if (m_numeroGanador == 0) {
                m_mensajeResultado = "Cayo 0 verde. Perdiste.";
            } else {
                bool esParNum = (m_numeroGanador % 2 == 0);
                bool gano = (m_parImparElegido == 0 && esParNum) || (m_parImparElegido == 1 && !esParNum);
                if (gano) {
                    ganancia = m_apuesta * 2;
                    *m_dineroJugador += ganancia;
                    m_mensajeResultado = "GANASTE! $" + std::to_string(ganancia) +
                        "\nNumero: " + std::to_string(m_numeroGanador);
                } else {
                    m_mensajeResultado = "Perdiste... Numero: " + std::to_string(m_numeroGanador);
                }
            }
            break;
        }
        
        case TipoApuesta::MANQUE_PASSE: {  // NUEVO
            if (m_numeroGanador == 0) {
                m_mensajeResultado = "Cayo 0 verde. Perdiste.";
            } else {
                bool esManque = (m_numeroGanador >= 1 && m_numeroGanador <= 18);
                bool gano = (m_mitadElegida == 0 && esManque) || (m_mitadElegida == 1 && !esManque);
                if (gano) {
                    ganancia = m_apuesta * 2;
                    *m_dineroJugador += ganancia;
                    m_mensajeResultado = "GANASTE! $" + std::to_string(ganancia) +
                        "\nNumero: " + std::to_string(m_numeroGanador);
                } else {
                    m_mensajeResultado = "Perdiste... Numero: " + std::to_string(m_numeroGanador);
                }
            }
            break;
        }
        
        default:
            break;
    }
    
    // Resetear selección
    m_tipoApuesta = TipoApuesta::NINGUNA;
    m_numeroElegido = -1;
    m_cuartoElegido = -1;
    m_colorElegido = ColorElegido::NINGUNO;
    m_parImparElegido = -1;
    m_columnaElegida = -1;
    m_mitadElegida = -1;
    m_mostrandoTableroNumeros = false;
    m_mostrandoSelectorCuarto = false;
    m_mostrandoSelectorColor = false;
    m_mostrandoSelectorParImpar = false;
    
    std::cout << "Ruleta gira... Numero: " << m_numeroGanador << std::endl;
}

void MinigameRuleta::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!m_isActive || m_mostrandoResultado) return;
    
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    
    // Actualizar hovers
    
    m_hoverGirar = m_btnGirar.getGlobalBounds().contains(mousePos);
    m_hoverAumentar = m_btnAumentar.getGlobalBounds().contains(mousePos);
    m_hoverDisminuir = m_btnDisminuir.getGlobalBounds().contains(mousePos);
    
   
    
        // Hovers del tapete
    if (m_mostrandoTapete) {
        for (size_t i = 0; i < m_hoverCeldas.size(); i++) {
            m_hoverCeldas[i] = m_celdasTapete[i].getGlobalBounds().contains(mousePos);
        }
        
        // Hovers de columnas
        m_hoverColumna1 = m_btnColumna1.getGlobalBounds().contains(mousePos);
        m_hoverColumna2 = m_btnColumna2.getGlobalBounds().contains(mousePos);
        m_hoverColumna3 = m_btnColumna3.getGlobalBounds().contains(mousePos);
        
        // Hovers de docenas
        m_hoverDocena1 = m_btnDocena1.getGlobalBounds().contains(mousePos);
        m_hoverDocena2 = m_btnDocena2.getGlobalBounds().contains(mousePos);
        m_hoverDocena3 = m_btnDocena3.getGlobalBounds().contains(mousePos);
        
        // Hovers de externas
        m_hoverMitadBaja = m_btnMitadBaja.getGlobalBounds().contains(mousePos);
        m_hoverPar = m_btnPar.getGlobalBounds().contains(mousePos);
        m_hoverRojo = m_btnRojo.getGlobalBounds().contains(mousePos);
        m_hoverNegro = m_btnNegro.getGlobalBounds().contains(mousePos);
        m_hoverImpar = m_btnImpar.getGlobalBounds().contains(mousePos);
        m_hoverMitadAlta = m_btnMitadAlta.getGlobalBounds().contains(mousePos);
    }

    // Eventos de click
    if (event.is<sf::Event::MouseButtonPressed>()) {
        const auto& mouseEvent = event.getIf<sf::Event::MouseButtonPressed>();
        if (mouseEvent->button == sf::Mouse::Button::Left) {
            
           
            
            // Botón Girar
            if (m_hoverGirar) {
                girarRuleta();
            }
            
            // Botones de apuesta
            if (m_hoverAumentar && m_apuesta < m_apuestaMaxima) {
                m_apuesta += 5;
                if (m_dineroJugador && m_apuesta > *m_dineroJugador) {
                    m_apuesta = *m_dineroJugador;
                }
            }
            if (m_hoverDisminuir && m_apuesta > m_apuestaMinima) {
                m_apuesta -= 5;
            }
            
           

            // ===== TAPETE INTERACTIVO (DENTRO DEL IF DEL LEFT CLICK) =====
            if (m_mostrandoTapete) {
                // Clic en celdas de números
                for (size_t i = 0; i < m_celdasTapete.size(); i++) {
                    if (m_hoverCeldas[i]) {
                        m_tipoApuesta = TipoApuesta::NUMERO_EXACTO;
                        m_numeroElegido = i;
                        std::cout << "Tapete: Número elegido: " << i << std::endl;
                    }
                }
                
                // Clic en columnas
                if (m_hoverColumna1) {
                    m_tipoApuesta = TipoApuesta::COLUMNA;
                    m_columnaElegida = 0;
                }
                if (m_hoverColumna2) {
                    m_tipoApuesta = TipoApuesta::COLUMNA;
                    m_columnaElegida = 1;
                }
                if (m_hoverColumna3) {
                    m_tipoApuesta = TipoApuesta::COLUMNA;
                    m_columnaElegida = 2;
                }
                
                // Clic en docenas
                if (m_hoverDocena1) {
                    m_tipoApuesta = TipoApuesta::CUARTO;
                    m_cuartoElegido = 0;
                }
                if (m_hoverDocena2) {
                    m_tipoApuesta = TipoApuesta::CUARTO;
                    m_cuartoElegido = 1;
                }
                if (m_hoverDocena3) {
                    m_tipoApuesta = TipoApuesta::CUARTO;
                    m_cuartoElegido = 2;
                }
                
                // Clic en externas
                if (m_hoverMitadBaja) {
                    m_tipoApuesta = TipoApuesta::MANQUE_PASSE;
                    m_mitadElegida = 0;
                }
                if (m_hoverMitadAlta) {
                    m_tipoApuesta = TipoApuesta::MANQUE_PASSE;
                    m_mitadElegida = 1;
                }
                if (m_hoverPar) {
                    m_tipoApuesta = TipoApuesta::PAR_IMPAR;
                    m_parImparElegido = 0;
                }
                if (m_hoverImpar) {
                    m_tipoApuesta = TipoApuesta::PAR_IMPAR;
                    m_parImparElegido = 1;
                }
                if (m_hoverRojo) {
                    m_tipoApuesta = TipoApuesta::COLOR;
                    m_colorElegido = ColorElegido::ROJO;
                }
                if (m_hoverNegro) {
                    m_tipoApuesta = TipoApuesta::COLOR;
                    m_colorElegido = ColorElegido::NEGRO;
                }
            }
        }
    }
    

    // Hover para números
    if (m_mostrandoTableroNumeros) {
        for (size_t i = 0; i < m_botonesNumeros.size(); i++) {
            m_hoverNumeros[i] = m_botonesNumeros[i].getGlobalBounds().contains(mousePos);
        }
    }
}
    


void MinigameRuleta::update(float dt) {
    if (!m_isActive) return;
    
    // Animación de la pelotita
    if (m_pelotitaGirando) {
        // Avanzar la pelotita (independiente del framerate)
        m_anguloPelotita += m_velocidadPelotita * dt;
        
        // Frenar gradualmente
        m_velocidadPelotita *= std::pow(0.997f, dt * 60.f);
        
        // Calcular cuánto falta para llegar al objetivo
        float distanciaFaltante = m_anguloObjetivoPelotita - m_anguloPelotita;
        
        // Si ya pasó el objetivo o está muy cerca, detener
        if (distanciaFaltante <= 5.f) {
            m_pelotitaGirando = false;
            m_anguloPelotita = m_anguloObjetivoPelotita;
            m_mostrandoResultado = true;
            m_tiempoResultado = 3.0f;
        }
        
        // Si se pasó, ajustar
        if (m_anguloPelotita > m_anguloObjetivoPelotita) {
            m_pelotitaGirando = false;
            m_anguloPelotita = m_anguloObjetivoPelotita;
            m_mostrandoResultado = true;
            m_tiempoResultado = 3.0f;
        }
    }
    
    if (m_mostrandoResultado) {
        m_tiempoResultado -= dt;
        if (m_tiempoResultado <= 0.0f) {
            m_mostrandoResultado = false;
        }
    }
}

void MinigameRuleta::draw(sf::RenderWindow& window) {
    if (!m_isActive) return;
    
    // Ruleta (imagen de fondo)
    if (m_ruletaSprite) {
        float escalaX = m_size.x / m_ruletaTexture.getSize().x;
        float escalaY = m_size.y / m_ruletaTexture.getSize().y;
        float escala = std::min(escalaX, escalaY);
        
        m_ruletaSprite->setScale(sf::Vector2f(escala, escala));
        
        sf::FloatRect bounds = m_ruletaSprite->getLocalBounds();
        m_ruletaSprite->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_ruletaSprite->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f,
            m_position.y + m_size.y / 2.f
        ));
        window.draw(*m_ruletaSprite);
    }
    
    // ===== RULETA FIJA CON PELOTITA GIRANDO =====
    float proporcionX = m_size.x / 1280.f;
    float proporcionY = m_size.y / 720.f;

    sf::Vector2f centroRuleta(
        m_position.x + m_size.x - (1100.f * proporcionX),
        m_position.y + m_size.y / 2.f - (30.f * proporcionY)
    );

    // Factor de escala para F11
    float escalaRuleta = std::min(m_size.x / 800.f, m_size.y / 600.f);
    float radioBorde = 195.f * escalaRuleta;
    float radioPista = 185.f * escalaRuleta;
    float radioNumeros = 150.f * escalaRuleta;
    float radioPelotita = 165.f * escalaRuleta;
    float radioCentro = 40.f * escalaRuleta;
    float radioAro = 45.f * escalaRuleta;

    // Círculo exterior
    sf::CircleShape bordeExterior(radioBorde);
    bordeExterior.setOrigin(sf::Vector2f(radioBorde, radioBorde));
    bordeExterior.setPosition(centroRuleta);
    bordeExterior.setFillColor(sf::Color(80, 40, 20));
    bordeExterior.setOutlineThickness(3.f * escalaRuleta);
    bordeExterior.setOutlineColor(sf::Color(120, 80, 40));
    window.draw(bordeExterior);

    // Pista negra
    sf::CircleShape pista(radioPista);
    pista.setOrigin(sf::Vector2f(radioPista, radioPista));
    pista.setPosition(centroRuleta);
    pista.setFillColor(sf::Color(15, 10, 5));
    window.draw(pista);

    // Sectores de colores (FIJOS, no giran)
    for (int i = 0; i < 37; i++) {
        float anguloInicio = (i * (360.f / 37.f)) * 3.14159f / 180.f;
        float anguloFin = ((i + 1) * (360.f / 37.f)) * 3.14159f / 180.f;
        
        sf::VertexArray sector(sf::PrimitiveType::TriangleFan, 3);
        sector[0].position = centroRuleta;
        
        int numero = m_ordenRuleta[i];
        sf::Color colorSector;
        if (numero == 0) {
            colorSector = sf::Color(0, 140, 0);
        } else if (esRojo(numero)) {
            colorSector = sf::Color(190, 0, 0);
        } else {
            colorSector = sf::Color(25, 25, 25);
        }
        
        sector[0].color = colorSector;
        sector[1].position = centroRuleta + sf::Vector2f(
            radioPista * std::cos(anguloInicio),
            radioPista * std::sin(anguloInicio)
        );
        sector[1].color = colorSector;
        sector[2].position = centroRuleta + sf::Vector2f(
            radioPista * std::cos(anguloFin),
            radioPista * std::sin(anguloFin)
        );
        sector[2].color = colorSector;
        
        window.draw(sector);
    }

    // Líneas separadoras
    for (int i = 0; i < 37; i++) {
        float angulo = (i * (360.f / 37.f)) * 3.14159f / 180.f;
        
        sf::VertexArray linea(sf::PrimitiveType::Lines, 2);
        linea[0].position = centroRuleta;
        linea[0].color = sf::Color(80, 70, 50);
        linea[1].position = centroRuleta + sf::Vector2f(
            radioPista * std::cos(angulo),
            radioPista * std::sin(angulo)
        );
        linea[1].color = sf::Color(80, 70, 50);
        window.draw(linea);
    }

    // Números (FIJOS)
    for (int i = 0; i < 37; i++) {
        m_numerosRuleta[i].setFillColor(sf::Color::White);
        m_numerosRuleta[i].setOutlineThickness(1.f);
        m_numerosRuleta[i].setOutlineColor(sf::Color::Black);
        m_numerosRuleta[i].setCharacterSize(13 * escalaRuleta);
        
        float angulo = (i * (360.f / 37.f) + (360.f / 74.f)) * 3.14159f / 180.f;
        float x = centroRuleta.x + radioNumeros * std::cos(angulo);
        float y = centroRuleta.y + radioNumeros * std::sin(angulo);
        
        sf::FloatRect tb = m_numerosRuleta[i].getLocalBounds();
        m_numerosRuleta[i].setOrigin(sf::Vector2f(tb.size.x / 2.f, tb.size.y / 2.f));
        m_numerosRuleta[i].setPosition(sf::Vector2f(x, y));
        m_numerosRuleta[i].setString(std::to_string(m_ordenRuleta[i]));
        
        window.draw(m_numerosRuleta[i]);
    }

    // Círculo interior
    sf::CircleShape centroInterior(radioCentro);
    centroInterior.setOrigin(sf::Vector2f(radioCentro, radioCentro));
    centroInterior.setPosition(centroRuleta);
    centroInterior.setFillColor(sf::Color(50, 25, 10));
    centroInterior.setOutlineThickness(3.f * escalaRuleta);
    centroInterior.setOutlineColor(sf::Color(180, 150, 50));
    window.draw(centroInterior);

    // Aro metálico
    sf::CircleShape aroMetalico(radioAro);
    aroMetalico.setOrigin(sf::Vector2f(radioAro, radioAro));
    aroMetalico.setPosition(centroRuleta);
    aroMetalico.setFillColor(sf::Color::Transparent);
    aroMetalico.setOutlineThickness(2.f * escalaRuleta);
    aroMetalico.setOutlineColor(sf::Color(200, 200, 200));
    window.draw(aroMetalico);

    // Flecha arriba
    m_flecha.setSize(sf::Vector2f(6.f * escalaRuleta, 25.f * escalaRuleta));
    m_flecha.setOrigin(sf::Vector2f(3.f * escalaRuleta, 25.f * escalaRuleta));
    m_flecha.setPosition(sf::Vector2f(centroRuleta.x, centroRuleta.y - radioBorde));
    window.draw(m_flecha);

    // Pelotita girando
    if (m_pelotitaGirando || m_mostrandoResultado) {
        float anguloPelotita = fmod(m_anguloPelotita, 360.f) * 3.14159f / 180.f;
        
        m_pelotita.setRadius(7.f * escalaRuleta);
        m_pelotita.setOrigin(sf::Vector2f(7.f * escalaRuleta, 7.f * escalaRuleta));
        
        sf::Vector2f posPelotita(
            centroRuleta.x + radioPelotita * std::cos(anguloPelotita),
            centroRuleta.y + radioPelotita * std::sin(anguloPelotita)
        );
        
        m_pelotita.setPosition(posPelotita);
        window.draw(m_pelotita);
    }

    // Iluminar franja ganadora
    if (m_mostrandoResultado) {
        int indiceGanador = 0;
        for (int i = 0; i < 37; i++) {
            if (m_ordenRuleta[i] == m_numeroGanador) {
                indiceGanador = i;
                break;
            }
        }
        
        float anguloInicioFranja = (indiceGanador * (360.f / 37.f)) * 3.14159f / 180.f;
        float anguloFinFranja = ((indiceGanador + 1) * (360.f / 37.f)) * 3.14159f / 180.f;
        
        sf::VertexArray franjaIluminada(sf::PrimitiveType::TriangleFan, 3);
        franjaIluminada[0].position = centroRuleta;
        franjaIluminada[0].color = sf::Color(255, 255, 100, 100);
        franjaIluminada[1].position = centroRuleta + sf::Vector2f(
            radioPista * std::cos(anguloInicioFranja),
            radioPista * std::sin(anguloInicioFranja)
        );
        franjaIluminada[1].color = sf::Color(255, 255, 100, 100);
        franjaIluminada[2].position = centroRuleta + sf::Vector2f(
            radioPista * std::cos(anguloFinFranja),
            radioPista * std::sin(anguloFinFranja)
        );
        franjaIluminada[2].color = sf::Color(255, 255, 100, 100);
        window.draw(franjaIluminada);
    }
    
    // Función para centrar texto en botón
    auto centerTextInButton = [](sf::Text& text, const sf::RectangleShape& btn) {
        sf::FloatRect tb = text.getLocalBounds();
        text.setOrigin(sf::Vector2f(tb.size.x / 2.f, tb.size.y / 2.f));
        text.setPosition(sf::Vector2f(
            btn.getPosition().x + btn.getSize().x / 2.f,
            btn.getPosition().y + btn.getSize().y / 2.f - 2.f
        ));
    };
    
    // Título centrado arriba
    if (m_tituloText) {
        sf::FloatRect tb = m_tituloText->getLocalBounds();
        m_tituloText->setOrigin(sf::Vector2f(tb.size.x / 2.f, 0.f));
        m_tituloText->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f,
            m_position.y + 10.f
        ));
        window.draw(*m_tituloText);
    }
    
    // Dinero (esquina superior izquierda)
    if (m_dineroText && m_dineroJugador) {
        m_dineroText->setString("Tu dinero: $" + std::to_string(*m_dineroJugador));
        m_dineroText->setPosition(sf::Vector2f(m_position.x + 15.f, m_position.y + 10.f));
        window.draw(*m_dineroText);
    }
    
    // Instrucciones (abajo)
    if (m_instruccionesText) {
        m_instruccionesText->setPosition(sf::Vector2f(
            m_position.x + 15.f,
            m_position.y + m_size.y - 40.f
        ));
        window.draw(*m_instruccionesText);
    }
    
        // ===== TAPETE INTERACTIVO =====
    if (m_mostrandoTapete) {
        // Resolución base de referencia: 1280x720
        float refWidth = 1280.f;
        float refHeight = 720.f;
        
        // Proporciones basadas en los valores deseados
        float propX1 = 415.f / refWidth;
        float propX2 = 1235.f / refWidth;
        float propY1 = 165.f / refHeight;
        float propY2 = 190.f / refHeight;
        
        // Calcular espacio disponible
        float anchoDisponible = (propX2 - propX1) * m_size.x;
        float altoDisponible = (propY2 - propY1) * m_size.y;
        
        // Posición del tapete
        m_tapeteX = m_position.x + propX1 * m_size.x;
        m_tapeteY = m_position.y + propY1 * m_size.y;
        
        // Calcular escala
        float escalaTapete = anchoDisponible / (13.f * (41.f + 2.f));
        
        m_celdaWidth = 35.f * escalaTapete;
        m_celdaHeight = 55.f * escalaTapete;
        float celdaSpacing = 2.f * escalaTapete;
        
        // Cero
        float ceroX = m_tapeteX;
        float ceroY = m_tapeteY;
        m_celdasTapete[0].setSize(sf::Vector2f(m_celdaWidth, m_celdaHeight * 3.f));
        m_celdasTapete[0].setPosition(sf::Vector2f(ceroX, ceroY));
        m_celdasTapete[0].setFillColor(sf::Color::Transparent);
        
        if (m_hoverCeldas[0]) {
            m_celdasTapete[0].setOutlineColor(sf::Color::White);
            m_celdasTapete[0].setOutlineThickness(2.5f);
        } else if (m_numeroElegido == 0 && m_tipoApuesta == TipoApuesta::NUMERO_EXACTO) {
            m_celdasTapete[0].setOutlineColor(sf::Color::Yellow);
            m_celdasTapete[0].setOutlineThickness(3.f);
        } else {
            m_celdasTapete[0].setOutlineThickness(0.f);
        }
        window.draw(m_celdasTapete[0]);
        
        // Números 1-36
        for (int fila = 0; fila < 3; fila++) {
            for (int col = 0; col < 12; col++) {
                int numero;
                if (fila == 0) numero = 3 + col * 3;
                else if (fila == 1) numero = 2 + col * 3;
                else numero = 1 + col * 3;
                
                int idx = numero;
                
                float x = ceroX + m_celdaWidth + celdaSpacing + col * (m_celdaWidth + celdaSpacing);
                float y = ceroY + fila * (m_celdaHeight + celdaSpacing);
                
                m_celdasTapete[idx].setSize(sf::Vector2f(m_celdaWidth, m_celdaHeight));
                m_celdasTapete[idx].setPosition(sf::Vector2f(x, y));
                m_celdasTapete[idx].setFillColor(sf::Color::Transparent);
                
                if (m_hoverCeldas[idx]) {
                    m_celdasTapete[idx].setOutlineColor(sf::Color::White);
                    m_celdasTapete[idx].setOutlineThickness(2.5f);
                } else if (m_numeroElegido == numero && m_tipoApuesta == TipoApuesta::NUMERO_EXACTO) {
                    m_celdasTapete[idx].setOutlineColor(sf::Color::Yellow);
                    m_celdasTapete[idx].setOutlineThickness(3.f);
                } else {
                    m_celdasTapete[idx].setOutlineThickness(0.f);
                }
                
                window.draw(m_celdasTapete[idx]);
            }
        }
        
        // Botones "2 to 1" (columnas)
        for (int fila = 0; fila < 3; fila++) {
            float xBtn = ceroX + m_celdaWidth + celdaSpacing + 12 * (m_celdaWidth + celdaSpacing);
            float yBtn = ceroY + fila * (m_celdaHeight + celdaSpacing);
            
            sf::RectangleShape* btnColumna = nullptr;
            std::unique_ptr<sf::Text>* textoColumna = nullptr;
            
            if (fila == 0) { btnColumna = &m_btnColumna3; textoColumna = &m_textoColumna3; }
            else if (fila == 1) { btnColumna = &m_btnColumna2; textoColumna = &m_textoColumna2; }
            else { btnColumna = &m_btnColumna1; textoColumna = &m_textoColumna1; }
            
            btnColumna->setSize(sf::Vector2f(m_celdaWidth, m_celdaHeight));
            btnColumna->setPosition(sf::Vector2f(xBtn, yBtn));
            btnColumna->setFillColor(sf::Color::Transparent);
            
            bool hoverColumna = (fila == 0) ? m_hoverColumna3 : (fila == 1) ? m_hoverColumna2 : m_hoverColumna1;
            if (m_columnaElegida == (2 - fila) && m_tipoApuesta == TipoApuesta::COLUMNA) {
                btnColumna->setOutlineColor(sf::Color::Yellow);
                btnColumna->setOutlineThickness(3.f);
            } else if (hoverColumna) {
                btnColumna->setOutlineColor(sf::Color::White);
                btnColumna->setOutlineThickness(2.5f);
            } else {
                btnColumna->setOutlineThickness(0.f);
            }
            
            window.draw(*btnColumna);
        }
        
        // Docenas
        float docenaY = ceroY + 3 * (m_celdaHeight + celdaSpacing);
        float docenaWidth = (m_celdaWidth + celdaSpacing) * 4.f - celdaSpacing;
        
        for (int d = 0; d < 3; d++) {
            float x = ceroX + m_celdaWidth + celdaSpacing + d * (docenaWidth + celdaSpacing);
            
            sf::RectangleShape* btnDocena = nullptr;
            std::unique_ptr<sf::Text>* textoDocena = nullptr;
            
            if (d == 0) { btnDocena = &m_btnDocena1; textoDocena = &m_textoDocena1; }
            else if (d == 1) { btnDocena = &m_btnDocena2; textoDocena = &m_textoDocena2; }
            else { btnDocena = &m_btnDocena3; textoDocena = &m_textoDocena3; }
            
            btnDocena->setSize(sf::Vector2f(docenaWidth, m_celdaHeight));
            btnDocena->setPosition(sf::Vector2f(x, docenaY));
            btnDocena->setFillColor(sf::Color::Transparent);
            
            bool hoverDocena = (d == 0) ? m_hoverDocena1 : (d == 1) ? m_hoverDocena2 : m_hoverDocena3;
            if (m_cuartoElegido == d && m_tipoApuesta == TipoApuesta::CUARTO) {
                btnDocena->setOutlineColor(sf::Color::Yellow);
                btnDocena->setOutlineThickness(3.f);
            } else if (hoverDocena) {
                btnDocena->setOutlineColor(sf::Color::White);
                btnDocena->setOutlineThickness(2.5f);
            } else {
                btnDocena->setOutlineThickness(0.f);
            }
            
            window.draw(*btnDocena);
        }
        
        // Apuestas externas
        float externaY = docenaY + m_celdaHeight + celdaSpacing;
        float externaWidth = (m_celdaWidth + celdaSpacing) * 2.f - celdaSpacing;
        
        for (int e = 0; e < 6; e++) {
            float x = ceroX + m_celdaWidth + celdaSpacing + e * (externaWidth + celdaSpacing);
            
            sf::RectangleShape* btnExterno = nullptr;
            std::unique_ptr<sf::Text>* textoExterno = nullptr;
            bool seleccionado = false;
            bool hoverExterno = false;
            
            switch (e) {
                case 0: btnExterno = &m_btnMitadBaja; textoExterno = &m_textoMitadBaja; 
                        seleccionado = (m_mitadElegida == 0 && m_tipoApuesta == TipoApuesta::MANQUE_PASSE);
                        hoverExterno = m_hoverMitadBaja;
                        break;
                case 1: btnExterno = &m_btnPar; textoExterno = &m_textoPar;
                        seleccionado = (m_parImparElegido == 0 && m_tipoApuesta == TipoApuesta::PAR_IMPAR);
                        hoverExterno = m_hoverPar;
                        break;
                case 2: btnExterno = &m_btnRojo; textoExterno = &m_textoRojo;
                        seleccionado = (m_colorElegido == ColorElegido::ROJO && m_tipoApuesta == TipoApuesta::COLOR);
                        hoverExterno = m_hoverRojo;
                        break;
                case 3: btnExterno = &m_btnNegro; textoExterno = &m_textoNegro;
                        seleccionado = (m_colorElegido == ColorElegido::NEGRO && m_tipoApuesta == TipoApuesta::COLOR);
                        hoverExterno = m_hoverNegro;
                        break;
                case 4: btnExterno = &m_btnImpar; textoExterno = &m_textoImpar;
                        seleccionado = (m_parImparElegido == 1 && m_tipoApuesta == TipoApuesta::PAR_IMPAR);
                        hoverExterno = m_hoverImpar;
                        break;
                case 5: btnExterno = &m_btnMitadAlta; textoExterno = &m_textoMitadAlta;
                        seleccionado = (m_mitadElegida == 1 && m_tipoApuesta == TipoApuesta::MANQUE_PASSE);
                        hoverExterno = m_hoverMitadAlta;
                        break;
            }
            
            btnExterno->setSize(sf::Vector2f(externaWidth, m_celdaHeight));
            btnExterno->setPosition(sf::Vector2f(x, externaY));
            btnExterno->setFillColor(sf::Color::Transparent);
            
            if (seleccionado) {
                btnExterno->setOutlineColor(sf::Color::Yellow);
                btnExterno->setOutlineThickness(3.f);
            } else if (hoverExterno) {
                btnExterno->setOutlineColor(sf::Color::White);
                btnExterno->setOutlineThickness(2.5f);
            } else {
                btnExterno->setOutlineThickness(0.f);
            }
            
            window.draw(*btnExterno);
        }
    }
    
    // ===== BOTONES DE APUESTA Y GIRAR =====
    float escalaBotones = std::min(m_size.x / 800.f, m_size.y / 600.f);
    float btnWidth = 140.f * escalaBotones;
    float btnHeight = 40.f * escalaBotones;
    float btnSpacing = 15.f * escalaBotones;
    
    float btnStartX = m_position.x + 30.f;
    float btnStartY = m_position.y + m_size.y * 0.85f;
    
    float btnApuestaWidth = 50.f * escalaBotones;
    float apuestaGroupWidth = btnWidth * 2 + btnSpacing;
    
    // Botón -
    m_btnDisminuir.setSize(sf::Vector2f(btnApuestaWidth, btnHeight));
    m_btnDisminuir.setPosition(sf::Vector2f(btnStartX, btnStartY));
    
    // Botón +
    m_btnAumentar.setSize(sf::Vector2f(btnApuestaWidth, btnHeight));
    m_btnAumentar.setPosition(sf::Vector2f(btnStartX + apuestaGroupWidth - btnApuestaWidth, btnStartY));
    
    // Texto de apuesta
    if (m_apuestaText) {
        m_apuestaText->setString("$" + std::to_string(m_apuesta));
        sf::FloatRect tb = m_apuestaText->getLocalBounds();
        m_apuestaText->setOrigin(sf::Vector2f(tb.size.x / 2.f, tb.size.y / 2.f));
        m_apuestaText->setPosition(sf::Vector2f(
            btnStartX + apuestaGroupWidth / 2.f,
            btnStartY + btnHeight / 2.f
        ));
        window.draw(*m_apuestaText);
    }
    
    // Botón GIRAR
    m_btnGirar.setSize(sf::Vector2f(btnWidth * 2 + btnSpacing, btnHeight));
    m_btnGirar.setPosition(sf::Vector2f(btnStartX + apuestaGroupWidth + btnSpacing, btnStartY));
    
    // Dibujar botones
    window.draw(m_btnDisminuir);
    window.draw(m_btnAumentar);
    window.draw(m_btnGirar);
    
    // Textos
    if (m_textoBtnDisminuir) {
        sf::FloatRect tb = m_textoBtnDisminuir->getLocalBounds();
        m_textoBtnDisminuir->setOrigin(sf::Vector2f(tb.size.x / 2.f, tb.size.y / 2.f));
        m_textoBtnDisminuir->setPosition(sf::Vector2f(
            m_btnDisminuir.getPosition().x + btnApuestaWidth / 2.f,
            m_btnDisminuir.getPosition().y + btnHeight / 2.f
        ));
        window.draw(*m_textoBtnDisminuir);
    }
    
    if (m_textoBtnAumentar) {
        sf::FloatRect tb = m_textoBtnAumentar->getLocalBounds();
        m_textoBtnAumentar->setOrigin(sf::Vector2f(tb.size.x / 2.f, tb.size.y / 2.f));
        m_textoBtnAumentar->setPosition(sf::Vector2f(
            m_btnAumentar.getPosition().x + btnApuestaWidth / 2.f,
            m_btnAumentar.getPosition().y + btnHeight / 2.f
        ));
        window.draw(*m_textoBtnAumentar);
    }
    
    if (m_textoBtnGirar) {
        centerTextInButton(*m_textoBtnGirar, m_btnGirar);
        window.draw(*m_textoBtnGirar);
    }
    
    // Resultado
    if (m_mostrandoResultado && m_resultadoText) {
        m_resultadoText->setString(m_mensajeResultado);
        m_resultadoText->setFillColor(sf::Color::White);
        
        sf::FloatRect bounds = m_resultadoText->getLocalBounds();
        m_resultadoText->setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));
        m_resultadoText->setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f,
            m_position.y + m_size.y / 2.f + 150.f
        ));
        
        sf::RectangleShape fondoMensaje(sf::Vector2f(bounds.size.x + 40.f, bounds.size.y + 30.f));
        fondoMensaje.setFillColor(sf::Color(0, 0, 0, 200));
        fondoMensaje.setOutlineThickness(2.f);
        fondoMensaje.setOutlineColor(sf::Color::Yellow);
        fondoMensaje.setOrigin(sf::Vector2f((bounds.size.x + 40.f) / 2.f, (bounds.size.y + 30.f) / 2.f));
        fondoMensaje.setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f,
            m_position.y + m_size.y / 2.f + 150.f
        ));
        window.draw(fondoMensaje);
        window.draw(*m_resultadoText);
        
        // Número ganador
        sf::Text numeroGanadorText(m_font, std::to_string(m_numeroGanador), 60);
        numeroGanadorText.setFillColor(sf::Color::White);
        numeroGanadorText.setStyle(sf::Text::Bold);
        
        sf::FloatRect numBounds = numeroGanadorText.getLocalBounds();
        numeroGanadorText.setOrigin(sf::Vector2f(numBounds.size.x / 2.f, numBounds.size.y / 2.f));
        numeroGanadorText.setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f,
            m_position.y + m_size.y / 2.f + 50.f
        ));
        
        sf::RectangleShape fondoNumero(sf::Vector2f(numBounds.size.x + 40.f, numBounds.size.y + 30.f));
        fondoNumero.setFillColor(sf::Color(0, 0, 0, 200));
        fondoNumero.setOutlineThickness(2.f);
        fondoNumero.setOutlineColor(sf::Color::Yellow);
        fondoNumero.setOrigin(sf::Vector2f((numBounds.size.x + 40.f) / 2.f, (numBounds.size.y + 30.f) / 2.f));
        fondoNumero.setPosition(sf::Vector2f(
            m_position.x + m_size.x / 2.f,
            m_position.y + m_size.y / 2.f + 50.f
        ));
        window.draw(fondoNumero);
        window.draw(numeroGanadorText);
    }
}

bool MinigameRuleta::esPar(int numero) {
    return (numero % 2 == 0);
}

void MinigameRuleta::inicializarRuletaAnimada() {
    if (!m_fontLoaded) return;
    
    // Orden real de la ruleta europea
    m_ordenRuleta = {0, 32, 15, 19, 4, 21, 2, 25, 17, 34, 6, 27, 13, 36, 11, 30, 8, 23, 10, 5, 24, 16, 33, 1, 20, 14, 31, 9, 22, 18, 29, 7, 28, 12, 35, 3, 26};
    
    
    float radio = 180.f;
    m_circuloRuleta.setRadius(radio);
    m_circuloRuleta.setPointCount(64);
    m_circuloRuleta.setFillColor(sf::Color(60, 30, 20));
    m_circuloRuleta.setOutlineThickness(4.f);
    m_circuloRuleta.setOutlineColor(sf::Color(180, 150, 50));
    m_circuloRuleta.setOrigin(sf::Vector2f(radio, radio));
    
    // Crear números
    m_numerosRuleta.clear();
    for (int i = 0; i < 37; i++) {
        sf::Text num(m_font, std::to_string(i), 13);
        num.setStyle(sf::Text::Bold);
        m_numerosRuleta.push_back(num);
    }
    
    // Flecha
    m_flecha.setSize(sf::Vector2f(6.f, 25.f));
    m_flecha.setFillColor(sf::Color::Yellow);
    m_flecha.setOrigin(sf::Vector2f(3.f, 25.f));
    
   // Pelotita
m_pelotita.setRadius(7.f);
m_pelotita.setFillColor(sf::Color::White);
m_pelotita.setOutlineThickness(1.f);
m_pelotita.setOutlineColor(sf::Color(180, 180, 180));
m_pelotita.setOrigin(sf::Vector2f(7.f, 7.f));


}

void MinigameRuleta::inicializarTapete() {
    if (!m_fontLoaded) return;
    
    // Crear las celdas del tapete (0-36)
    for (int i = 0; i <= 36; i++) {
        sf::RectangleShape celda(sf::Vector2f(42.f, 34.f));
        celda.setOutlineThickness(1.5f);
        m_celdasTapete.push_back(celda);
        
        auto texto = std::make_unique<sf::Text>(m_font, std::to_string(i), 14);
        texto->setStyle(sf::Text::Bold);
        m_textosTapete.push_back(std::move(texto));
        m_hoverCeldas.push_back(false);
    }
    
    // Crear botones de docenas (1st 12, 2nd 12, 3rd 12)
    auto crearBotonDocena = [this](sf::RectangleShape& btn, std::unique_ptr<sf::Text>& texto, const std::string& str) {
        btn.setSize(sf::Vector2f(126.f, 34.f)); // 3 celdas de ancho
        btn.setFillColor(sf::Color(30, 30, 30, 180));
        btn.setOutlineThickness(1.5f);
        btn.setOutlineColor(sf::Color(180, 160, 120));
        
        texto = std::make_unique<sf::Text>(m_font, str, 13);
        texto->setFillColor(sf::Color(200, 200, 180));
        texto->setStyle(sf::Text::Bold);
    };
    
    crearBotonDocena(m_btnDocena1, m_textoDocena1, "1st 12");
    crearBotonDocena(m_btnDocena2, m_textoDocena2, "2nd 12");
    crearBotonDocena(m_btnDocena3, m_textoDocena3, "3rd 12");
    
    // Crear botones de columna (2 to 1)
    auto crearBotonColumna = [this](sf::RectangleShape& btn, std::unique_ptr<sf::Text>& texto) {
        btn.setSize(sf::Vector2f(42.f, 34.f));
        btn.setFillColor(sf::Color(20, 20, 20, 180));
        btn.setOutlineThickness(1.5f);
        btn.setOutlineColor(sf::Color(100, 100, 100));
        
        texto = std::make_unique<sf::Text>(m_font, "2 to 1", 11);
        texto->setFillColor(sf::Color(180, 180, 180));
        texto->setStyle(sf::Text::Bold);
    };
    
    crearBotonColumna(m_btnColumna1, m_textoColumna1);
    crearBotonColumna(m_btnColumna2, m_textoColumna2);
    crearBotonColumna(m_btnColumna3, m_textoColumna3);
    
    // Crear botones de apuestas externas inferiores
    auto crearBotonExterno = [this](sf::RectangleShape& btn, std::unique_ptr<sf::Text>& texto, 
                                     const std::string& str, sf::Color fillColor) {
        btn.setSize(sf::Vector2f(84.f, 34.f));
        btn.setFillColor(fillColor);
        btn.setOutlineThickness(1.5f);
        btn.setOutlineColor(sf::Color(150, 150, 150));
        
        texto = std::make_unique<sf::Text>(m_font, str, 13);
        texto->setFillColor(sf::Color::White);
        texto->setStyle(sf::Text::Bold);
    };
    
    crearBotonExterno(m_btnMitadBaja, m_textoMitadBaja, "1 to 18", sf::Color(20, 50, 20, 180));
        // Para el tapete, reutilizamos los botones existentes m_btnPar, m_btnRojo, m_btnNegro, m_btnImpar
    // Solo configuramos sus textos si no existen
    if (!m_textoPar) m_textoPar = std::make_unique<sf::Text>(m_font, "EVEN", 13);
    if (!m_textoRojo) m_textoRojo = std::make_unique<sf::Text>(m_font, "RED", 13);
    if (!m_textoNegro) m_textoNegro = std::make_unique<sf::Text>(m_font, "BLACK", 13);
    if (!m_textoImpar) m_textoImpar = std::make_unique<sf::Text>(m_font, "ODD", 13);
    crearBotonExterno(m_btnMitadAlta, m_textoMitadAlta, "19 to 36", sf::Color(20, 50, 20, 180));
}

int MinigameRuleta::obtenerColumna(int numero) {
    if (numero == 0) return -1;
    // Columna 0: 1,4,7,10,13,16,19,22,25,28,31,34
    // Columna 1: 2,5,8,11,14,17,20,23,26,29,32,35
    // Columna 2: 3,6,9,12,15,18,21,24,27,30,33,36
    return (numero - 1) % 3;
}