package org.fallguard.demo;

import org.eclipse.paho.client.mqttv3.*;

public class MqttClientWrapper {

    private final MqttClient client;

    public MqttClientWrapper() throws MqttException {
        // Conectar al cliente MQTT
        // Cambia el broker si lo necesitas
        String broker = "tcp://broker.emqx.io:1883";
        String clientId = MqttClient.generateClientId();
        client = new MqttClient(broker, clientId);
        MqttConnectOptions options = new MqttConnectOptions();
        options.setCleanSession(true);
        client.connect(options);
    }

    public MqttClientWrapper(MqttClient client) {
        this.client = client;
    }

    // Enviar un mensaje al broker
    public void publishMessage(String topic, String message) throws MqttException {
        MqttMessage mqttMessage = new MqttMessage(message.getBytes());
        mqttMessage.setQos(1);
        client.publish(topic, mqttMessage);
    }

    // Suscribirse a un topic
    public void subscribeToTopic(String topic) throws MqttException {
        client.subscribe(topic, 1);
    }

    // Cerrar la conexión
    public void disconnect() throws MqttException {
        if (client.isConnected()) {
            client.disconnect();
        }
    }

    public void setCallback(MqttCallback mqttCallback) {
    }
}
