/*
 * ESP32 FFT TIME - Main
 * Proyecto de adquisición, procesamiento y visualización de señales en tiempo real con ESP32.
 * Incluye dashboard web, WebSocket, OLED y FFT en C++.
 *
 * Copyright (c) 2025 Alvaro Salazar
 * Licensed under the MIT License.
 *
 * Repositorio: https://github.com/alvaro-salazar/esp32-fft-web
 */

#include <Arduino.h>
#include <libwifi.h>
#include "libdisplay.h"
#include "libdsp.h"
#include "libwebsocket.h"

const char* HOSTNAME = "fft32"; // aparecerá como fft32.local

/**
 * @brief Función principal de configuración del ESP32.
 *        Inicializa la comunicación serial, el WiFi, el mDNS,
 *        la pantalla OLED, el ADC y las tareas de ADC y FFT.
 */
void setup(){
  Serial.begin(115200);     //> 1. Inicializa la comunicación serial a 115200 baudios
  delay(100);               //> 2. Espera 1 segundo para que la comunicación serial esté lista
  listWiFiNetworks();       //> 3. Lista las redes WiFi disponibles
  startWiFi("");            //> 4. Inicializa el servicio de WiFi
  setMDNS(HOSTNAME);        //> 5. Inicializa el servicio de mDNS para responder a fft32.local
  initDisplay();            //> 6. Inicializa la pantalla OLED 
  setupAdc();               //> 7. Inicializa el ADC usando I2S y DMA
  setQueue();               //> 8. Crea la cola para envío de bloques de muestras
  xTaskCreatePinnedToCore(taskADC,"adc",4096,NULL,1,NULL,0);   //> 9. Agrega la tarea ADC al core 0
  xTaskCreatePinnedToCore(taskFFT,"fft",6144,NULL,1,NULL,1);   //> 10. Agrega la tarea FFT al core 1
  setupWebSocket();         //> 11. Inicializa el servidor web y el websocket
}

/**
 * @brief Función principal de bucle del ESP32.
 *        En este caso, no se utiliza el bucle principal,
 *        ya que las tareas se ejecutan en segundo plano.
 */
void loop(){ /* nada */ }
