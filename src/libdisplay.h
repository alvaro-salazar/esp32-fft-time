#ifndef LIBDISPLAY_H
#define LIBDISPLAY_H

// Librería para inicialización y visualización de FFT en pantalla OLED

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa la pantalla OLED.
 */
void initDisplay();

/**
 * @brief Dibuja la FFT en la pantalla OLED.
 * @param vReal Vector de magnitudes reales (FFT).
 * @param vImag Vector de magnitudes imaginarias (FFT).
 */
void drawFFT(double * vReal, double * vImag);

#ifdef __cplusplus
}
#endif

#endif // LIBDISPLAY_H