/*
 * ESP32 FFT TIME - WebSocket
 * Copyright (c) 2025 Alvaro Salazar
 * Licensed under the MIT License.
 */

#include <SPIFFS.h>
#include "libwebsocket.h"

// Definición única de las instancias globales
AsyncWebServer server(80);
AsyncWebSocket ws(WS_PATH);

/**
 * @brief Inicializa el WebSocket y lo asocia al servidor.
 *        Debe llamarse en el setup principal.
 */
void setupWebSocket(){
    // Cuando ocurre un evento (conexión, desconexión, etc.) se llama a esta función
  ws.onEvent([](AsyncWebSocket *s, AsyncWebSocketClient*, AwsEventType t, void*, uint8_t*, size_t){});
  server.addHandler(&ws); // Agrega el WebSocket al servidor
  SPIFFS.begin(true); // Inicia SPIFFS que es el sistema de archivos SPI (memoria flash)
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html"); // Sirve el archivo index.html que está en SPIFFS (sistema de archivos SPI)
  server.begin(); // Inicia el servidor
  Serial.println("Servidor WebSocket iniciado");

}
