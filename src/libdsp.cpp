/*
 * ESP32 FFT TIME - DSP
 * Copyright (c) 2025 Alvaro Salazar
 * Licensed under the MIT License.
 */

#include <driver/i2s.h>
#include <driver/adc.h>
#include <Arduino.h>
#include <arduinoFFT.h>
#include "libdisplay.h"
#include "libwebsocket.h"
#include "libdsp.h"

#define I2S_PORT      I2S_NUM_0       // Este es el puerto I2S
#define ADC_CH        ADC1_CHANNEL_0  // Este es el canal ADC
#define SR_HZ         1000            // Esta es la frecuencia de muestreo  
#define N_SAMPLES     1024            // Esta es la cantidad de muestras por bloque
#define DMA_LEN       256             // Este es el tamaño del buffer de DMA
#define DMA_CNT       4               // Este es el número de buffers de DMA

static uint16_t  rawBuf[N_SAMPLES];   // Este es el buffer para almacenar las muestras
static QueueHandle_t qFFT;            // Esta es la cola para envío de bloques de muestras

static double    vReal[N_SAMPLES];    // Este es el buffer para almacenar las magnitudes reales de la FFT
static double    vImag[N_SAMPLES];    // Este es el buffer para almacenar las magnitudes imaginarias de la FFT

ArduinoFFT<double> FFT(vReal, vImag, N_SAMPLES, SR_HZ); // Este es el objeto FFT

/**
 * @brief Configura el ADC y el I2S para la lectura de la señal analoga
 * 
 */
void setupAdc(){
  adc1_config_width(ADC_WIDTH_BIT_12); // Configura el ancho del ADC a 12 bits
  adc1_config_channel_atten(ADC_CH, ADC_ATTEN_DB_12); // Configura el canal ADC y el atenuador

  // Configuración del I2S para la lectura de la señal analoga
  const i2s_config_t cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER|I2S_MODE_RX|I2S_MODE_ADC_BUILT_IN), // Configuración del modo del I2S
    .sample_rate = SR_HZ, // Configuración de la frecuencia de muestreo
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // Configuración del número de bits por muestra
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // Configuración del formato de canal
    .communication_format = I2S_COMM_FORMAT_STAND_I2S, // Configuración del formato de comunicación
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Configuración de la interrupción
    .dma_buf_count = DMA_CNT, // Configuración del número de buffers de DMA
    .dma_buf_len = DMA_LEN, // Configuración del tamaño del buffer de DMA
    .use_apll=true, // Configuración del PLL
    .tx_desc_auto_clear=false, // Configuración del auto-clear del descriptor de transmisión
    .fixed_mclk=0 // Configuración del reloj fijo
  };
  i2s_driver_install(I2S_PORT,&cfg,0,NULL); // Instala el driver del I2S
  i2s_set_pin(I2S_PORT,NULL); // Configura los pines del I2S
  i2s_set_adc_mode(ADC_UNIT_1,ADC_CH); // Configura el ADC
  i2s_adc_enable(I2S_PORT); // Habilita el ADC
}

/**
 * @brief Define la cola para envío de bloques de muestras
 */
void setQueue(){
  qFFT = xQueueCreate(2, N_SAMPLES*sizeof(uint16_t)); // aqui se define la cola para envío de bloques de muestras
}

/**
 * @brief Tarea para la lectura del ADC
 */
void taskADC(void *){
  size_t bytes; // Tamaño de los datos leídos
  uint8_t tmp[DMA_LEN*2]; // Buffer temporal para almacenar los datos leídos
  uint32_t idx = 0; // Índice para recorrer el buffer de muestras

  for(;;){
    if(i2s_read(I2S_PORT, tmp, sizeof(tmp), &bytes, portMAX_DELAY)==ESP_OK){
      for(size_t i=0; i<bytes; i+=2){ // Recorre el buffer de muestras
        rawBuf[idx++] = tmp[i] | (tmp[i+1]<<8); // Convierte los datos leídos a enteros de 16 bits
        if(idx==N_SAMPLES){ // Si se ha leído el número de muestras definido
          idx = 0;
          xQueueSend(qFFT, rawBuf, 0);   // envía bloque completo a la cola
        }
      }
    }
  }
}

/**
 * @brief Tarea para la FFT y el WebSocket
 */
void taskFFT(void *){
  uint16_t block[N_SAMPLES]; // Buffer para almacenar el bloque de muestras

  for(;;){
    if(xQueueReceive(qFFT, block, portMAX_DELAY)==pdTRUE){
      double mean = 0; // Media de las muestras
      for (uint16_t i=0;i<N_SAMPLES;i++) mean += block[i]; // Calcula la media de las muestras
      mean /= N_SAMPLES; // Divide la suma por el número de muestras para obtener la media
      
      // Aquí puedes aplicar un filtro digital sobre 'block' antes de la FFT
      // Por ejemplo: filtro pasa bajos, pasa altos, etc.
      // for (uint16_t i=0; i<N_SAMPLES; i++) block[i] = filtro(block[i]);
      
      for (uint16_t i=0;i<N_SAMPLES;i++){ // Recorre todas las muestras
        vReal[i] = block[i] - mean;   // quita DC
        vImag[i] = 0; // Inicializa la parte imaginaria a 0
      }
      FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD); // Aplica la ventana de Hamming
      FFT.compute(FFT_FORWARD); // Realiza la FFT
      FFT.complexToMagnitude(); // Convierte a magnitud

      /* armar JSON (solo 0‑500 Hz para no saturar) */
      String json = "{\"time\":[";
      for(uint16_t i=0;i<N_SAMPLES;i++){ // Recorre todas las muestras
        json += String(block[i]); // Agrega el valor de la muestra al JSON
        if(i<N_SAMPLES-1) json += ","; // Agrega una coma si no es la última muestra
      }
      json += "],\"freq\":["; // Agrega la etiqueta "freq" al JSON
      uint16_t limit = N_SAMPLES/2; // Límite para recorrer las muestras
      for(uint16_t i=0;i<limit;i++){ // Recorre las muestras hasta el límite
        json += String(vReal[i], 1);     // magnitud
        if(i<limit-1) json += ","; // Agrega una coma si no es la última muestra
      }
      json += "]}"; // Agrega el cierre del JSON

      ws.textAll(json);   // Envía el JSON a todos los clientes

      drawFFT(vReal, vImag); // Dibuja el espectro FFT en la pantalla OLED
    }
  }
}