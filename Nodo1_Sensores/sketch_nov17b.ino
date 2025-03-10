#include <WiFi.h>
#include <WiFiUdp.h> 
#include <PubSubClient.h>
#include <MPU9250_asukiaaa.h>
#include <Adafruit_BMP280.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <ArduinoJson.h>

// Pines I2C para ESP32
#define SDA_PIN 21
#define SCL_PIN 22

// Configuración de red Wi-Fi
const char* ssid = "Familia_Cera_Gomez"; // Cambia por el nombre de tu red Wi-Fi
const char* password = "Pin12106*"; // Cambia por la contraseña de tu red
// Configuración ThingsBoard
const char* thingsboard_server = "http://demo.thingsboard.io/api/v1/hi3zzCIxxLKRy1Y8ulYZ/telemetry"; // URL del API de ThingsBoard
// Configuración del broker MQTT
const char* mqtt_server = "broker.emqx.io"; // WebSocket con protocolo seguro
const int mqtt_port = 1883;  // Puerto estándar MQTT

const String bot_token = "7882291525:AAEfHHGwX01omlYKM0E6C4WOln0Vh70jU9A";
const String chat_id = "1154902509";


//Instancia de los clientes y sensores
WiFiClient wifiClient;
HTTPClient http;
WiFiUDP udp;

WiFiClient espClient;
PubSubClient client(espClient);
NTPClient timeClient(udp, "pool.ntp.org", 0, 60000);

Adafruit_BMP280 bme;
MPU9250_asukiaaa mySensor;


//Informacion usuario
const int userId  = 510;
const String userName = "Alberto Gomez";
const String location = "H510";
const String gender = "Male";


//Variables sensor
float aX, aY, aZ, gX, gY, gZ;

// Función para enviar los datos a ThingsBoard
void sendThings(String jsonData, String mqttTopic) {
  // 1. Enviar datos a ThingsBoard a través de HTTP
  if (WiFi.status() == WL_CONNECTED) {
    // Realizar la conexión HTTP para enviar los datos a ThingsBoard
    http.begin(wifiClient, thingsboard_server);
    http.addHeader("Content-Type", "application/json");

    // Enviar el JSON por HTTP
    int httpResponseCode = http.POST(jsonData);

    // Verificar la respuesta de la solicitud HTTP
    if (httpResponseCode > 0) {
      Serial.print("Código de respuesta HTTP: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error en la solicitud HTTP: ");
      Serial.println(httpResponseCode);
    }
    http.end(); // Finalizar la solicitud HTTP
  } else {
    Serial.println("No se puede conectar a ThingsBoard. Verifica la conexión Wi-Fi.");
  }
  if (!client.connected()) {
    connectMQTT();
  }

  if (client.publish(mqttTopic.c_str(), jsonData.c_str())) {
    Serial.println("Mensaje enviado correctamente.");
    Serial.println("Mensaje enviado al MQTT broker: " + jsonData);  // Confirmación en consola
  } else {
    int retries = 5;
    while (!client.publish(mqttTopic.c_str(), jsonData.c_str()) && retries > 0) {
      Serial.println("Reintentando enviar mensaje...");
      retries--;
    }
    if (retries == 0) {
      Serial.println("Fallo al enviar el mensaje tras varios intentos.");
    }
  }
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando al broker MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado");
    } else {
      Serial.print("Error, rc=");
      Serial.print(client.state());
      Serial.println(" Reintentando en 5 segundos...");
    }
  }
}

bool detectFall(float aX, float aY, float aZ) {
  float accel_magnitude = 0.0;
  float angle = 0.0;

  const float threshold_accel = 2.0; // Umbral de aceleración para detectar caída (ajústalo según sea necesario)
  const float threshold_angle = 90.0; // Umbral de ángulo (para detectar si la persona está de espaldas)

  // Calcular la magnitud de la aceleración
  accel_magnitude = sqrt(aX * aX +
                         aY * aY +
                         aZ * aZ);

  // Calcular el ángulo de inclinación usando el acelerómetro (suponiendo que caemos sobre el eje Z)
  angle = acos(accel_magnitude / 9.8) * 180.0 / PI;

  // Mostrar los valores
  Serial.print("Aceleración Magnitud: "); Serial.println(accel_magnitude);
  Serial.print("Ángulo de inclinación: "); Serial.println(angle);

  // Detección de caída
  if (accel_magnitude > threshold_accel || angle > threshold_angle) {
    Serial.println("Posible caída detectada debido a inclinación excesiva.");
    Serial.println("Posible caída detectada debido a alta aceleración.");
    return true;
  }
  return false; 
}

String createJson_user_data(int userId, String userName, String location, String gender) {
  // Crear el JSON con los datos utilizando ArduinoJson
  DynamicJsonDocument doc(1024);  // Tamaño del documento JSON (ajústalo según tus necesidades)

  // Asignar valores a las claves del JSON
  doc["userId"] = userId;
  doc["userName"] = userName;
  doc["location"] = location;
  doc["gender"] = gender;

  // Convertir el documento JSON a un string
  String jsonData;
  serializeJson(doc, jsonData);

  // Retornar el JSON como string
  return jsonData;
}

String createJson_sensor_data(float aX, float aY, float aZ, float gX, float gY, float gZ) {
  // Crear el JSON con los datos utilizando ArduinoJson
  DynamicJsonDocument doc(1024);  // Tamaño del documento JSON (ajústalo según tus necesidades)

  // Asignar valores a las claves del JSON
  doc["aX"] = aX;
  doc["aY"] = aY;
  doc["aZ"] = aZ;
  doc["gX"] = gX;
  doc["gY"] = gY;
  doc["gZ"] = gZ;

  // Convertir el documento JSON a un string
  String jsonData;
  serializeJson(doc, jsonData);
  // Retornar el JSON como string
  return jsonData;
}

String createJson_alerts_data(String alertType, String alertMessage, String alertTimestamp) {
  // Crear el JSON con los datos utilizando ArduinoJson
  DynamicJsonDocument doc(1024);  // Tamaño del documento JSON (ajústalo según tus necesidades)
  // Asignar valores a las claves del JSON
  doc["alertType"] = alertType;
  doc["alertMessage"] = alertMessage;
  doc["alertTimestamp"] = alertTimestamp;

  // Convertir el documento JSON a un string
  String jsonData;
  serializeJson(doc, jsonData);

  // Retornar el JSON como string
  return jsonData;
}

void sendTelegramAlert(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http; // Crea una instancia local de HTTPClient
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
  while (!Serial);

  // Conectar a Wi-Fi
  Serial.print("Conectando a Wi-Fi");
  WiFi.begin(ssid, password);
  timeClient.begin();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado");
  Serial.print("IP asignada: ");
  Serial.println(WiFi.localIP());

  // Configurar cliente MQTT
  client.setServer(mqtt_server, mqtt_port);

  // Inicializar sensores
  Wire.begin(SDA_PIN, SCL_PIN);
  mySensor.setWire(&Wire);
  mySensor.beginAccel();
  mySensor.beginGyro();

}

void loop() {
  String userJson;   // Para guardar el JSON de usuario
  String sensorJson; // Para guardar el JSON de sensores
  String alertJson;  // Para guardar el JSON de alertas
  String alertType = "-";
  String alertMessage = "-";

  if (mySensor.accelUpdate() == 0) {
    aX = mySensor.accelX();
    aY = mySensor.accelY();
    aZ = mySensor.accelZ();
  }
    if (mySensor.gyroUpdate() == 0) {
    gX = mySensor.gyroX();
    gY = mySensor.gyroY();
    gZ = mySensor.gyroZ();
  }

  if(detectFall(aX, aY, aZ))
  {
    if (!client.connected()) {
      connectMQTT();
    }
    client.loop();
    alertType = "caida_detectada";
    alertMessage = "Estado critico";
    sendTelegramAlert("Alerta, se ha generado una caida");

    timeClient.update();
    unsigned long timestamp = timeClient.getEpochTime();
    String timestampStr = String(timestamp);

    userJson = createJson_user_data(userId, userName, location, gender);
    sensorJson = createJson_sensor_data(aX, aY, aZ, gX, gY, gZ);
    alertJson = createJson_alerts_data(alertType, alertMessage, timestampStr);

    String alertMqttTopic = "hospital/patient/info/events";  // Topic para las alertas
    sendThings(alertJson, alertMqttTopic);
    alertJson = "";
      
    String userMqttTopic = "hospital/patient/info";  // Topic para los datos del usuario
    sendThings(userJson, userMqttTopic);
    userJson = "";

    String sensorMqttTopic = "hospital/patient/info/sensor_data";  // Topic para los datos del sensor
    sendThings(sensorJson, sensorMqttTopic);
    sensorJson = "";

  }
}