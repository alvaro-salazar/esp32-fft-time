# README

Proyecto : **Captura de señales con el ADC interno del ESP32 + FFT + dashboard web**

Este repo enseña a:

1. Leer señales analógicas (0‑3.3 V) usando I²S + ADC interno del ESP32.
2. Procesar 1024 muestras en una FFT en el segundo núcleo.
3. Enviar los datos por WebSocket y verlos en el navegador en tiempo real (tiempo y frecuencia).

Todo se compila en VS Code con PlatformIO.

---

### requisitos rápidos

* Placa ESP32 DevKit (cualquier WROOM/WROVER).
* VS Code + extensión PlatformIO.
* Navegador moderno (Chrome, Edge, Firefox).
* Cable USB decente.

---

### cómo clonar y abrir

```
git clone https://github.com/tu‑usuario/esp32‑fft‑web.git
cd esp32‑fft‑web
code .
```

PlatformIO detecta el proyecto al abrir VS Code.

---

### librerías que se descargan solas

* arduinoFFT
* ESP Async WebServer
* AsyncTCP

(No tienes que hacer nada: `platformio.ini` ya las declara).

---

### construir y flashear

1. Compilar + subir firmware

   ```
   pio run -t upload
   ```

2. Subir los archivos web (carpeta `data/`)

   ```
   pio run -t uploadfs
   ```

3. Pulsa RESET en la placa.

---

### abrir el dashboard

* Modo AP: el ESP crea la red **FFT‑ESP32**.
  Conéctate, abre `http://192.168.4.1/`.

* Modo STA (si le pones tu Wi‑Fi): mira la IP en el monitor serie.
  Ejemplo: `http://192.168.0.23/`.

También responde por mDNS: `http://fft32.local/` (algunos sistemas necesitan Bonjour/Avahi).

---

### carpetas

```
src/        código principal (main.cpp)
data/       index.html + assets web
include/    headers opcionales
platformio.ini
README.md   este archivo
```

---

### qué hace el código

* Core 0 lee el ADC vía I²S DMA → cola FreeRTOS.
* Core 1 coge bloques de 1024 puntos → ventana Hamming + FFT.
* Se calcula la magnitud y se manda junto al bloque de tiempo crudo por WebSocket.
* `index.html` dibuja dos gráficas con Chart.js:

  * Voltaje en tiempo (0‑3.3 V).
  * FFT en eje vertical dB (0 dB arriba, –120 dB abajo).
* El dashboard se ajusta a móvil y a pantalla full‑HD.

---

### cosas por hacer

* Agregar filtrado FIR/IIR en tiempo real.
* Guardar los datos en microSD.
* Enviar por MQTT a un broker externo.
* Usar un front‑end analógico real (ADS1299) para EEG serio.

---

© 2025 Alvaro Salazar
