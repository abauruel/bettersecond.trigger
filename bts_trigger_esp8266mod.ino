#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define LED_PIN 2 
#define LED_PIN2 4      // Melhor usar GPIO16 para LED
#define BUTTON_PIN 5     // Botão no GPIO4 (D2 em muitas placas)


// Configurações Wi-Fi
const char *ssid = "bts";
const char *password = "<my_wifi_password>";

// Variáveis de controle
bool buttonState = HIGH;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

unsigned long lastBlinkTime = 0;
bool ledState = false;
unsigned long blinkIntervalConnected = 1000;   // 1s
unsigned long blinkIntervalDisconnected = 200; // 0.2s

String postUrl = "http://10.42.0.1:5000/record/cam2";

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // botão para GND
  

  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_PIN2, LOW);
  

  WiFi.begin(ssid, password);
  Serial.println("Conectando ao WiFi...");
}

void loop() {
  // Controle do LED pelo status WiFi
  unsigned long currentMillis = millis();
  unsigned long interval = (WiFi.status() == WL_CONNECTED) ? blinkIntervalConnected : blinkIntervalDisconnected;

  if (currentMillis - lastBlinkTime >= interval) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    digitalWrite(LED_PIN2, ledState ? HIGH : LOW);
    lastBlinkTime = currentMillis;
  }

  // Leitura com debounce do botão
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {   // botão pressionado
        Serial.println("Botão pressionado!");
        handleButtonPress();
      }
    }
  }

  lastButtonState = reading;
}

void handleButtonPress() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED_PIN2, HIGH);
    
    delay(100);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_PIN2, LOW);
    
    delay(100);
  }
  sendPostRequest();
}

void sendPostRequest() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClient client;
    http.begin(client, postUrl);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST("{}");

    if (httpResponseCode > 0) {
      Serial.printf("POST enviado! Código: %d\n", httpResponseCode);
    } else {
      Serial.printf("Erro POST: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi desconectado, não foi possível enviar POST.");
  }
}
