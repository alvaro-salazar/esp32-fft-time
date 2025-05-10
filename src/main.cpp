#include <Arduino.h>
#include <driver/i2s.h>
#include <driver/adc.h>
#include <arduinoFFT.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <libwifi.h>

#define I2S_PORT      I2S_NUM_0
#define ADC_CH        ADC1_CHANNEL_0  // GPIO36
#define SR_HZ         1000
#define N_SAMPLES     1024            // potencia de 2
#define DMA_LEN       256
#define DMA_CNT       4
#define WS_PATH       "/ws"

const char* HOSTNAME = "fft32"; // aparecerá como fft32.local

static uint16_t  rawBuf[N_SAMPLES];   // dominio del tiempo
static double    vReal[N_SAMPLES];
static double    vImag[N_SAMPLES];

static QueueHandle_t qFFT;
static AsyncWebServer  server(80);
static AsyncWebSocket  ws(WS_PATH);


ArduinoFFT<double> FFT(vReal, vImag, N_SAMPLES, SR_HZ);

/* ---------- lectura ADC en Core 0 ---------- */
void taskADC(void *){
  size_t bytes;
  uint8_t tmp[DMA_LEN*2];
  uint32_t idx = 0;

  for(;;){
    if(i2s_read(I2S_PORT, tmp, sizeof(tmp), &bytes, portMAX_DELAY)==ESP_OK){
      for(size_t i=0; i<bytes; i+=2){
        rawBuf[idx++] = tmp[i] | (tmp[i+1]<<8);
        if(idx==N_SAMPLES){
          idx = 0;
          xQueueSend(qFFT, rawBuf, 0);   // envía bloque completo
        }
      }
    }
  }
}

/* ---------- FFT y WebSocket en Core 1 ---------- */
void taskFFT(void *){
  uint16_t block[N_SAMPLES];

  for(;;){
    if(xQueueReceive(qFFT, block, portMAX_DELAY)==pdTRUE){
      double mean = 0;
      for (uint16_t i=0;i<N_SAMPLES;i++) mean += block[i];
      mean /= N_SAMPLES;
      
      for (uint16_t i=0;i<N_SAMPLES;i++){
        vReal[i] = block[i] - mean;   // quita DC
        vImag[i] = 0;
      }
      FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
      FFT.compute(FFT_FORWARD);
      FFT.complexToMagnitude();

      /* armar JSON (solo 0‑500 Hz para no saturar) */
      String json = "{\"time\":[";
      for(uint16_t i=0;i<N_SAMPLES;i++){
        json += String(block[i]);
        if(i<N_SAMPLES-1) json += ",";
      }
      json += "],\"freq\":[";
      uint16_t limit = N_SAMPLES/2;
      for(uint16_t i=0;i<limit;i++){
        json += String(vReal[i], 1);     // magnitud
        if(i<limit-1) json += ",";
      }
      json += "]}";
      ws.textAll(json);   // broadcast
    }
  }
}

/* ---------- setup ---------- */
void setup(){
  Serial.begin(115200);

  listWiFiNetworks();       // Paso 2. Lista las redes WiFi disponibles
  delay(1000);              // -- Espera 1 segundo para ver las redes disponibles
  startWiFi("");            // Paso 5. Inicializa el servicio de WiFi

  if (!MDNS.begin(HOSTNAME)) {                // responde a fft32.local
    Serial.println("mDNS no pudo iniciar");
  } else {
    MDNS.addService("http", "tcp", 80);       // anuncia servicio HTTP
    MDNS.addServiceTxt("http", "tcp", "path", "/");
    Serial.print("mDNS listo: http://");
    Serial.print(HOSTNAME);
    Serial.println(".local/");
  }


  /* ADC + I2S igual que antes (con I2S_MODE_RX & set_pin(NULL)) */
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC_CH, ADC_ATTEN_DB_12);

  const i2s_config_t cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER|I2S_MODE_RX|I2S_MODE_ADC_BUILT_IN),
    .sample_rate = SR_HZ,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = DMA_CNT,
    .dma_buf_len = DMA_LEN,
    .use_apll=true,
    .tx_desc_auto_clear=false,
    .fixed_mclk=0
  };
  i2s_driver_install(I2S_PORT,&cfg,0,NULL);
  i2s_set_pin(I2S_PORT,NULL);
  i2s_set_adc_mode(ADC_UNIT_1,ADC_CH);
  i2s_adc_enable(I2S_PORT);

  /* cola y tareas */
  qFFT = xQueueCreate(2, N_SAMPLES*sizeof(uint16_t));
  xTaskCreatePinnedToCore(taskADC,"adc",4096,NULL,1,NULL,0);   // core 0
  xTaskCreatePinnedToCore(taskFFT,"fft",6144,NULL,1,NULL,1);   // core 1

  /* servidor y websocket */
  ws.onEvent([](AsyncWebSocket *s, AsyncWebSocketClient*, AwsEventType t, void*, uint8_t*, size_t){});
  server.addHandler(&ws);
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  server.begin();
  SPIFFS.begin(true);
}

void loop(){ /* nada */ }
