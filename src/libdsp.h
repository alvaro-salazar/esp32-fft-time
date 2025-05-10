#ifndef LIBDSP_H
#define LIBDSP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa el ADC usando I2S y DMA.
 */
void setupAdc();

/**
 * @brief Tarea para adquirir muestras del ADC y enviarlas a la cola.
 * @param pvParameters Parámetro de tarea FreeRTOS (no usado).
 */
void taskADC(void *pvParameters);

/**
 * @brief Tarea para procesar la FFT, enviar datos al WebSocket y actualizar la pantalla.
 * @param pvParameters Parámetro de tarea FreeRTOS (no usado).
 */
void taskFFT(void *pvParameters);

/**
 * @brief Crea la cola para el envío de bloques de muestras.
 */
void setQueue();

#ifdef __cplusplus
}
#endif

#endif // LIBDSP_H
