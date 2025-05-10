#ifndef LIBWEBSOCKET_H
#define LIBWEBSOCKET_H

#include <ESPAsyncWebServer.h>

// Ruta del WebSocket
#define WS_PATH "/ws"

// Instancias globales del servidor y WebSocket (declaradas como extern)
extern AsyncWebServer server;
extern AsyncWebSocket ws;

/**
 * @brief Inicializa el WebSocket y lo asocia al servidor.
 *        Debe llamarse en el setup principal.
 */
void setupWebSocket();

#endif // LIBWEBSOCKET_H
