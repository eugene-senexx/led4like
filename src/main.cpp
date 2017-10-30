#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "config.h"

WiFiClient espClient;
const char* mqtt_server = MQTT_SERVER;
PubSubClient client(espClient);
volatile bool actionRequired = false;

void messageCallback(const char* topic, byte* payload, unsigned int length) {
    Serial.printf("Got message on topic %s\n", topic);
    digitalWrite(LED_PIN, HIGH);
}

void buttonCallback() {
    if (!actionRequired) {
        Serial.println("Button pressed");
        actionRequired = true;
        digitalWrite(LED_PIN, LOW);
    }
}

void mqttConnect(PubSubClient &client, Stream &logger) {
    if (!client.connected()) {
        logger.println("Connecting to MQTT server");
        if (
            client.connect(
                MQTT_TOPIC,
                MQTT_USER,
                MQTT_PASS)
        ) {
            logger.println("Connected to MQTT server");
          client.subscribe(MQTT_TOPIC);
        }
        else {
            logger.println("Could not connect to MQTT server");
        }
    }
}

void setup() {
    Serial.begin(9600);

    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(LED_PIN, LOW);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WLAN_SSID, WLAN_PASS);
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    client.setServer(mqtt_server, MQTT_PORT);
    client.setCallback(messageCallback);

    attachInterrupt(BUTTON_PIN, buttonCallback, RISING);

    Serial.println("ESP8266 ready");
}

void loop() {
    mqttConnect(client, Serial);

    client.loop();

    if(actionRequired) {
        client.publish(MQTT_RESPONSE_TOPIC, "Hello from ESP");
        actionRequired = false;
    }
}