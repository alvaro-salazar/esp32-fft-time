#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "libdisplay.h"

// OLED display settings
#define SCREEN_WIDTH 128 // Ancho de la pantalla OLED
#define SCREEN_HEIGHT 64 // Alto de la pantalla OLED
#define OLED_RESET    -1 // Pin de reset del OLED
#define SCREEN_ADDRESS 0x3C // Dirección I2C del OLED

#define N_SAMPLES 1024 // Número de muestras por bloque
#define SR_HZ     1000 // Frecuencia de muestreo en Hz

// Instancia global del display OLED
static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Variables globales para peak hold de escala dB (no usadas en la versión actual)
float oled_dB_max_hold = -200.0f;
unsigned long oled_dB_max_hold_time = 0;

/**
 * @brief Inicializa la pantalla OLED y muestra un mensaje de inicio.
 *        Debe llamarse una sola vez en el setup del programa.
 */
void initDisplay() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Error al inicializar OLED");
    for(;;);
  }
  display.clearDisplay();

  // Título grande
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds("ESP32", 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH-w)/2, 2);
  display.print("ESP32");

  // Subtítulo
  display.setTextSize(1);
  display.getTextBounds("FFT TIME", 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH-w)/2, 20);
  display.print("FFT TIME");

  // Gráfico de barras aún más pequeño y más abajo
  int barBaseY = 54;
  int barWidth = 3;
  int barSpace = 2;
  int bars[] = {2, 4, 8, 12, 8, 4, 2}; // Alturas de barras más pequeñas
  int nBars = sizeof(bars)/sizeof(bars[0]);
  int totalWidth = nBars*barWidth + (nBars-1)*barSpace;
  int startX = (SCREEN_WIDTH - totalWidth)/2;
  for(int i=0; i<nBars; i++) {
    display.fillRect(startX + i*(barWidth+barSpace), barBaseY - bars[i], barWidth, bars[i], SSD1306_WHITE);
  }

  // Usuario de GitHub en la parte inferior, versión corta para que quepa
  display.setTextSize(1);
  const char* github = "alvaro-salazar";
  display.getTextBounds(github, 0, 0, &x1, &y1, &w, &h);
  int y_github = SCREEN_HEIGHT - h - 1;
  display.fillRect(0, y_github, SCREEN_WIDTH, h+2, SSD1306_BLACK); // Limpia fondo
  display.setCursor((SCREEN_WIDTH-w)/2, y_github);
  display.print(github);

  display.display();
  delay(1800);
}

/**
 * @brief Dibuja el espectro FFT normalizado en la pantalla OLED.
 *        - El espectro se normaliza respecto al valor máximo actual.
 *        - Las frecuencias bajas aparecen a la izquierda.
 *        - Se muestran etiquetas de frecuencia, dB/div y frecuencia del pico máximo.
 * 
 * @param vReal Vector de magnitudes reales (FFT).
 * @param vImag Vector de magnitudes imaginarias (FFT). (No usado aquí, pero mantenido para compatibilidad)
 */
void drawFFT(double * vReal, double * vImag) {
    display.clearDisplay();

    // --- Parámetros de visualización ---
    const float dB_min = -120.0f;   // Límite inferior de la escala dB (fijo)
    const float dB_max = 43.0f;     // Límite superior de la escala dB (fijo)
    const int barWidth = 2;         // Ancho de cada barra del espectro
    const int maxBars = (SCREEN_WIDTH - 4) / barWidth; // Número máximo de barras a mostrar
    const int startBin = 1;         // Omitir el bin DC
    const int endBin = min(N_SAMPLES/4, maxBars); // Solo hasta 250 Hz aprox.

    // --- Dibuja la cuadrícula de puntos ---
    const int gridStep = 16;
    for(int i = 0; i < SCREEN_WIDTH; i += gridStep) {
      for(int j = 0; j < SCREEN_HEIGHT; j += gridStep) {
        display.drawPixel(i, j, SSD1306_WHITE);
      }
    }

    // --- Encuentra el valor máximo y la frecuencia del pico ---
    float max_val = 1e-6f;
    int max_idx = 0;
    for(int i = 0; i < endBin; i++) {
      if(vReal[i] > max_val) {
        max_val = vReal[i];
        max_idx = i;
      }
    }
    float freq_max = (float)(max_idx + startBin) * ((float)SR_HZ / (float)N_SAMPLES);

    // --- Dibuja las barras normalizadas (pico más alto llega arriba) ---
    for(int i = 0; i < endBin; i++) {
      float norm = vReal[i] / max_val;
      int y = (int)(norm * (SCREEN_HEIGHT - 8));
      y = constrain(y, 0, SCREEN_HEIGHT - 8);
      int x = (endBin - 1 - i) * barWidth + 2; // Invertido: bajas a la izquierda
      if(y > 1) {
        display.fillRect(x, SCREEN_HEIGHT-4-y, barWidth, y, SSD1306_WHITE);
      }
    }

    // --- Etiquetas de frecuencia (0Hz y 250Hz) ---
    display.setTextSize(1);
    display.fillRect(0, 0, 40, 10, SSD1306_BLACK); // 0Hz
    display.fillRect(SCREEN_WIDTH-28, 0, 28, 10, SSD1306_BLACK); // 250Hz

    display.setTextColor(SSD1306_WHITE);
    display.setCursor(2, 2);
    display.print("0Hz");
    display.setCursor(SCREEN_WIDTH-24, 2);
    display.print("250Hz");

    // --- Cálculo y visualización de dB/div y Fmax ---
    int num_div = SCREEN_HEIGHT / gridStep;
    int db_per_div = (int)round((dB_max - dB_min) / num_div);

    // Posiciones para no superponer etiquetas
    int label_width = 58;
    int label_height = 10;
    int margin = 2;
    int y_dbdiv = 14; // Debajo de 250Hz
    int y_fmax = y_dbdiv + label_height + margin;

    display.fillRect(SCREEN_WIDTH-label_width, y_dbdiv, label_width, label_height, SSD1306_BLACK); // dB/div
    display.fillRect(SCREEN_WIDTH-label_width, y_fmax, label_width, label_height, SSD1306_BLACK); // Fmax

    display.setCursor(SCREEN_WIDTH-label_width+2, y_dbdiv+1);
    display.print("dB/div:");
    display.print(db_per_div);

    display.setCursor(SCREEN_WIDTH-label_width+2, y_fmax+1);
    display.print("Fmax:");
    display.print((int)freq_max);
    display.print("Hz");

    display.display();
}