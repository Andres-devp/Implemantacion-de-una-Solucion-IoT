#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>

// Configuración WiFi
const char* ssid = "Familia_Cera_Gomez";        // Cambia por tu red WiFi
const char* password = "Pin12106*";             // Cambia por la contraseña de tu WiFi

// Configuración del Broker MQTT
const char* mqtt_server = "broker.emqx.io";     // Cambia si usas otro broker
const int mqtt_port = 1883;
const char* topic1 = "hospital/patient/info/events"; // Primer topic
const char* topic2 = "fallguard/enfermeras"; // Segundo topic

// Pines para LED y buzzer
const int ledPin = 6;    // Cambia según tu hardware
const int buzzerPin = 5; // Cambia según tu hardware

const String bot_token = "7882291525:AAEfHHGwX01omlYKM0E6C4WOln0Vh70jU9A";
const String chat_id = "1154902509";

WiFiClient espClient;
HTTPClient http;

PubSubClient client(espClient);

// Función para manejar mensajes MQTT
void mqttCallback(char* topic, byte* message, unsigned int length) {
  Serial.print("Mensaje recibido en el topic: ");
  Serial.println(topic);

  // Convertir mensaje en un string
  String receivedMessage;
  for (int i = 0; i < length; i++) {
    receivedMessage += (char)message[i];
  }
  Serial.print("Mensaje: ");
  Serial.println(receivedMessage);

  // Verificar si el mensaje es "apagar"
  if (receivedMessage == "apagar") {
    Serial.println("Apagando LED y buzzer...");
    digitalWrite(ledPin, LOW);    // Apagar LED
    digitalWrite(buzzerPin, LOW); // Apagar buzzer
  }

  // Analizar si contiene "caida_detectada" y "Estado critico"
  else if (receivedMessage.indexOf("caida_detectada") >= 0 && receivedMessage.indexOf("Estado critico") >= 0) {
    Serial.println("¡Alerta de caída detectada!");
    digitalWrite(ledPin, HIGH);   // Encender LED
    digitalWrite(buzzerPin, HIGH); // Encender buzzer           // Mantener encendido 1 segundo
    sendTelegramAlert("Aviso a enfermeras realizado.");
  }
}

// Función para conectarse al WiFi
void setupWiFi() {
  Serial.print("Conectando a WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
  }
  Serial.println("\nConexión WiFi establecida");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

// Función para reconectar al broker MQTT si se desconecta
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conectar al broker MQTT...");
    if (client.connect("ESP32_Client")) { // Cambia el ID si usas varios dispositivos
      Serial.println("Conectado al broker MQTT");
      client.subscribe(topic1);  // Suscribirse al primer topic
      client.subscribe(topic2);  // Suscribirse al segundo topic
    } else {
      Serial.print("Falló, rc=");
      Serial.print(client.state());
    }
  }
}

void sendTelegramAlert(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    String url = "https://api.telegram.org/bot" + bot_token + "/sendMessage?chat_id=" + chat_id + "&text=" + message;

    http.begin(url); // Inicializa la conexión HTTP
    int httpResponseCode = http.GET(); // Realiza la solicitud HTTP GET

    if (httpResponseCode > 0) {
      Serial.print("Mensaje enviado a Telegram. Código de respuesta: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error al enviar mensaje a Telegram: ");
      Serial.println(httpResponseCode);
    }
    http.end(); // Finaliza la conexión HTTP
  } else {
    Serial.println("No se puede conectar a Telegram. Verifica la conexión Wi-Fi.");
  }
}

void setup() {
  Serial.begin(115200);

  // Configuración de pines
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);

  // Conexión WiFi
  setupWiFi();

  // Configuración MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Escuchar mensajes MQTT
}
