package org.fallguard.demo;

import javafx.fxml.FXML;
import javafx.scene.web.WebView;
import javafx.scene.control.TextField;
import javafx.scene.control.Button;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import java.io.File;

public class FxController {

    @FXML
    private WebView webView;

    @FXML
    private TextField textField;

    @FXML
    private Button publishButton;

    private static final String BROKER_URL = "tcp://broker.emqx.io:1883"; // Cambia esto al broker de tu preferencia
    private static final String TOPIC = "fallguard/enfermeras"; // El tópico al que publicarás los mensajes
    private MqttClient mqttClient;

    // Método para cargar la URL automáticamente cuando la página se inicie
    @FXML
    private void initialize() {
        // Habilitar JavaScript para el WebView
        webView.getEngine().setJavaScriptEnabled(true);

        // Habilitar manejo de SSL
        webView.getEngine().setUserDataDirectory(new File("user-data-dir"));

        // Manejo de errores de carga
        webView.getEngine().setOnError(event -> {
            System.out.println("Error: " + event.getMessage());
        });

        // Cargar la URL automáticamente al iniciar la aplicación
        webView.getEngine().load("https://demo.thingsboard.io/dashboards/7213fbe0-a525-11ef-b5a8-ed1aed9a651f");

        // Configurar el User-Agent si es necesario
        webView.getEngine().setUserAgent("Tu User-Agent personalizado");

        try {
            // Iniciar la conexión MQTT
            mqttClient = new MqttClient(BROKER_URL, MqttClient.generateClientId());
            mqttClient.connect();
            System.out.println("Conectado al broker MQTT");
        } catch (MqttException e) {
            e.printStackTrace();
            System.out.println("Error al conectar al broker MQTT");
        }
    }

    // Método para manejar el clic en el botón
    @FXML
    private void onPublishButtonClicked() {
        String text = textField.getText();
        System.out.println("Texto ingresado: " + text);

        // Publicar el texto en el tópico MQTT
        try {
            MqttMessage message = new MqttMessage(text.getBytes());
            message.setQos(1); // Calidad de servicio (QoS)
            mqttClient.publish(TOPIC, message);
            System.out.println("Mensaje publicado en el tópico " + TOPIC);
        } catch (MqttException e) {
            e.printStackTrace();
            System.out.println("Error al publicar el mensaje en el tópico MQTT");
        }
    }

    // Método para desconectar el cliente MQTT cuando se cierra la aplicación
    @FXML
    private void onClose() {
        try {
            if (mqttClient.isConnected()) {
                mqttClient.disconnect();
                System.out.println("Desconectado del broker MQTT");
            }
        } catch (MqttException e) {
            e.printStackTrace();
            System.out.println("Error al desconectar del broker MQTT");
        }
    }
}
