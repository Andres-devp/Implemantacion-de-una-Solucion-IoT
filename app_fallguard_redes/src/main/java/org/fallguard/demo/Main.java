package org.fallguard.demo;

import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Scene;
import javafx.stage.Stage;
import org.eclipse.paho.client.mqttv3.MqttException;

import java.io.IOException;

public class Main extends Application {

    private MqttClientWrapper mqttClient;

    @Override
    public void start(Stage stage) throws IOException {
        // Iniciar cliente MQTT
        try {
            mqttClient = new MqttClientWrapper();

            // Conectar y suscribirse a un topic
            mqttClient.subscribeToTopic("hospital/patient/info");

            // Publicar un mensaje (esto lo podrías hacer basado en alguna acción en tu UI)
            mqttClient.publishMessage("hospital/patient/info", "Mensaje desde JavaFX");

        } catch (MqttException e) {
            e.printStackTrace();
        }

        // Cargar la interfaz de usuario
        FXMLLoader fxmlLoader = new FXMLLoader(Main.class.getResource("Fx_page.fxml"));
        Scene scene = new Scene(fxmlLoader.load());
        stage.setTitle("FallGuard App");
        stage.setScene(scene);
        stage.show();
    }

    public static void main(String[] args) {
        launch();
    }

    @Override
    public void stop() {
        // Cerrar la conexión MQTT cuando la aplicación se cierre
        try {
            if (mqttClient != null) {
                mqttClient.disconnect();
            }
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }
}
