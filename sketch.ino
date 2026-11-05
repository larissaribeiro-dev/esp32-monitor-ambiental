// Monitor ambiental com ESP32: leitura do DHT22, estado por temperatura,
// LEDs indicadores e exibição no OLED.
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHTesp.h"

// ----- Pinos -----
const int PINO_DHT = 15;
const int PINO_SDA = 21;
const int PINO_SCL = 22;
const int PINO_LED_VERDE = 18;
const int PINO_LED_VERMELHO = 19;

// ----- Display -----
const uint8_t ENDERECO_OLED = 0x3C;
const int LARGURA_OLED = 128;
const int ALTURA_OLED = 64;

// ----- Regra de alerta (fácil de alterar) -----
// Temperatura >= LIMITE_ALERTA_C resulta em ALERTA.
const float LIMITE_ALERTA_C = 30.0;

// ----- Temporização -----
// Igual ao intervalo mínimo informado pela biblioteca para o DHT22 (2000 ms).
const unsigned long INTERVALO_LEITURA_MS = 2000;

// ----- Faixa de medição do DHT22 (valores fora dela são tratados como inválidos) -----
const float TEMPERATURA_MIN_VALIDA = -40.0;
const float TEMPERATURA_MAX_VALIDA = 80.0;
const float UMIDADE_MIN_VALIDA = 0.0;
const float UMIDADE_MAX_VALIDA = 100.0;

enum EstadoSistema {
  ESTADO_NORMAL,
  ESTADO_ALERTA,
  ESTADO_ERRO_SENSOR
};

struct Leitura {
  float temperatura;
  float umidade;
  bool valida;
};

DHTesp sensor;
Adafruit_SSD1306 display(LARGURA_OLED, ALTURA_OLED, &Wire, -1);
unsigned long ultimaLeituraMs = 0;

// Último conteúdo desenhado no OLED, para redesenhar só quando algo mudou.
bool displayJaDesenhado = false;
Leitura leituraExibida = { NAN, NAN, false };
EstadoSistema estadoExibido = ESTADO_ERRO_SENSOR;

// Lê o sensor e só marca a leitura como válida se passar em todas as verificações.
Leitura lerSensor() {
  Leitura leitura = { NAN, NAN, false };
  TempAndHumidity dados = sensor.getTempAndHumidity();

  if (sensor.getStatus() != DHTesp::ERROR_NONE) {
    Serial.print("Falha na leitura. Status: ");
    Serial.println(sensor.getStatusString());
    return leitura;
  }
  if (isnan(dados.temperature) || isnan(dados.humidity)) {
    Serial.println("Falha na leitura: valor NaN");
    return leitura;
  }
  if (dados.temperature < TEMPERATURA_MIN_VALIDA || dados.temperature > TEMPERATURA_MAX_VALIDA ||
      dados.humidity < UMIDADE_MIN_VALIDA || dados.humidity > UMIDADE_MAX_VALIDA) {
    Serial.println("Falha na leitura: valor fora da faixa do sensor");
    return leitura;
  }

  leitura.temperatura = dados.temperature;
  leitura.umidade = dados.humidity;
  leitura.valida = true;
  return leitura;
}

// Aplica a regra de temperatura. Leitura inválida nunca entra na comparação.
EstadoSistema determinarEstado(const Leitura& leitura) {
  if (!leitura.valida) {
    return ESTADO_ERRO_SENSOR;
  }
  // O DHT22 tem resolução de 0,1 °C. Comparar em décimos de grau evita que um
  // erro de arredondamento de float (ex.: 29.999998) faça 30,0 °C passar como NORMAL.
  long temperaturaDecimos = lroundf(leitura.temperatura * 10.0f);
  long limiteDecimos = lroundf(LIMITE_ALERTA_C * 10.0f);
  if (temperaturaDecimos >= limiteDecimos) {
    return ESTADO_ALERTA;
  }
  return ESTADO_NORMAL;
}

// Verde = NORMAL, vermelho = ALERTA, ambos apagados = ERRO SENSOR.
void atualizarLeds(EstadoSistema estado) {
  digitalWrite(PINO_LED_VERDE, estado == ESTADO_NORMAL ? HIGH : LOW);
  digitalWrite(PINO_LED_VERMELHO, estado == ESTADO_ALERTA ? HIGH : LOW);
}

const char* nomeDoEstado(EstadoSistema estado) {
  switch (estado) {
    case ESTADO_NORMAL: return "NORMAL";
    case ESTADO_ALERTA: return "ALERTA";
    default:            return "ERRO SENSOR";
  }
}

// Redesenha a tela inteira a cada atualização, assim não sobram caracteres antigos.
void atualizarDisplay(const Leitura& leitura, EstadoSistema estado) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("MONITOR AMBIENTAL");
  display.drawFastHLine(0, 10, LARGURA_OLED, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 16);
  display.print("T: ");
  if (leitura.valida) {
    display.print(leitura.temperatura, 1);
    display.print(" C");
  } else {
    display.print("--");
  }

  display.setCursor(0, 34);
  display.print("U: ");
  if (leitura.valida) {
    display.print(leitura.umidade, 1);
    display.print(" %");
  } else {
    display.print("--");
  }

  display.setTextSize(1);
  display.setCursor(0, 54);
  display.print("Estado: ");
  display.print(nomeDoEstado(estado));

  display.display();
}

void registrarNoSerial(const Leitura& leitura, EstadoSistema estado) {
  if (leitura.valida) {
    Serial.print("Temperatura: ");
    Serial.print(leitura.temperatura, 1);
    Serial.print(" C | Umidade: ");
    Serial.print(leitura.umidade, 1);
    Serial.print(" % | ");
  }
  Serial.print("Estado: ");
  Serial.println(nomeDoEstado(estado));
}

// Diz se a tela está desatualizada em relação à leitura e ao estado novos.
// Leituras inválidas não têm valores a comparar: basta o estado.
bool displayPrecisaAtualizar(const Leitura& leitura, EstadoSistema estado) {
  if (!displayJaDesenhado) return true;
  if (estado != estadoExibido || leitura.valida != leituraExibida.valida) return true;
  if (!leitura.valida) return false;
  return leitura.temperatura != leituraExibida.temperatura ||
         leitura.umidade != leituraExibida.umidade;
}

void setup() {
  Serial.begin(115200);

  pinMode(PINO_LED_VERDE, OUTPUT);
  pinMode(PINO_LED_VERMELHO, OUTPUT);
  digitalWrite(PINO_LED_VERDE, LOW);
  digitalWrite(PINO_LED_VERMELHO, LOW);

  sensor.setup(PINO_DHT, DHTesp::DHT22);

  Wire.begin(PINO_SDA, PINO_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, ENDERECO_OLED)) {
    Serial.println("Falha ao iniciar o OLED");
    while (true) delay(1000);
  }

  // Faz a primeira leitura logo na partida, sem esperar o primeiro intervalo.
  ultimaLeituraMs = millis() - INTERVALO_LEITURA_MS;
}

void loop() {
  unsigned long agora = millis();
  if (agora - ultimaLeituraMs < INTERVALO_LEITURA_MS) {
    return;
  }
  ultimaLeituraMs = agora;

  Leitura leitura = lerSensor();
  EstadoSistema estado = determinarEstado(leitura);

  atualizarLeds(estado);
  if (displayPrecisaAtualizar(leitura, estado)) {
    atualizarDisplay(leitura, estado);
    leituraExibida = leitura;
    estadoExibido = estado;
    displayJaDesenhado = true;
  }
  registrarNoSerial(leitura, estado);
}
