#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define LED_PIN 16
// #define LED_PIN 16
#define BUTTON_PIN 4
#define BUZZER_PIN 2
// Configurações Wi-Fi
const char *ssid = "bts";
const char *password = "<my_wifi_password>"; // Deixe vazio se não tiver senha

// Configurações de pinos
const int ledPin = LED_PIN;       // Ajuste conforme seu circuito
const int buttonPin = BUTTON_PIN; // Botão
const int buzzerPin = BUZZER_PIN; // Buzzer

// Variáveis de controle
bool buttonPressed = false;
unsigned long lastBlinkTime = 0;
bool ledState = false;
unsigned long blinkIntervalConnected = 1000;     // 1 segundo
unsigned long blinkIntervalDisconnected = 200;   // 0.2 segundo
String postUrl = "http://10.42.0.1:5000/record"; // Altere para o destino correto

void setup()
{
  Serial.begin(115200);

  // Inicializa pinos
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

  // Conecta ao Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Conectando ao WiFi...");

  // Inicializa estados
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);
}

void loop()
{
  // Atualiza conexão Wi-Fi
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Não conectado ao WiFi.");
  }

  // Controle do LED de acordo com o status Wi-Fi
  unsigned long currentMillis = millis();
  unsigned long interval = (WiFi.status() == WL_CONNECTED) ? blinkIntervalConnected : blinkIntervalDisconnected;

  if (currentMillis - lastBlinkTime >= interval)
  {
    ledState = !ledState;
    digitalWrite(ledPin, ledState ? HIGH : LOW);
    lastBlinkTime = currentMillis;
  }

  // Verifica se o botão foi pressionado
  if (digitalRead(buttonPin) == HIGH)
  {
    delay(50); // Debounce simples
    if (digitalRead(buttonPin) == HIGH)
    {
      Serial.println("Botão pressionado!");
      handleButtonPress();
    }
  }
}

void handleButtonPress()
{
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(ledPin, HIGH);
    digitalWrite(buzzerPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);
    delay(100);
  }
  sendPostRequest();
}

void sendPostRequest()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    WiFiClient client;
    http.begin(client, postUrl);
    http.addHeader("Content-Type", "application/json"); // ou outro tipo conforme seu servidor
    int httpResponseCode = http.POST({});               // corpo do POST, se precisar

    if (httpResponseCode > 0)
    {
      Serial.printf("POST enviado! Código de resposta: %d\n", httpResponseCode);
    }
    else
    {
      Serial.printf("Erro no envio POST: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  }
  else
  {
    Serial.println("WiFi desconectado, não foi possível enviar POST.");
  }
}
