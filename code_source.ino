#include <WiFi.h>
#include <PubSubClient.h>

// Définir les pins
const int LDR_PIN = 36;  // Pin analogique pour le LDR
const int LED_PIN = 13;  // Pin de la LED (facultatif pour tester)
const int PIR_PIN = 27;  // Pin pour le capteur PIR
// Informations réseau Wi-Fi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Serveur MQTT
const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
      client.publish("/ThinkIOT/Publish", "Sensors Active");
      client.publish("/ThinkIOT/Mouvement", "Sensors Active");

      client.subscribe("/ThinkIOT/Subscribe");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);  // Pour tester la LED
  pinMode(PIR_PIN, INPUT_PULLUP);  // Configurer le capteur PIR en entrée
  pinMode(LDR_PIN, INPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 500) {  // Envoyer toutes les 500 ms
    lastMsg = now;
    
    // Lire la valeur du LDR
    int ldrValue = analogRead(LDR_PIN);  
    String ldrStr = String(ldrValue);

    // Lire la valeur du capteur PIR
    int pirValue = digitalRead(PIR_PIN);

    String pirStr = String(pirValue);


    client.publish("/ThinkIOT/Mouvement", pirStr.c_str());  // Publier la valeur du PIR
    client.publish("/ThinkIOT/Publish", ldrStr.c_str());  // Publier la valeur LDR

    // Afficher la luminosité dans le terminal
    Serial.print("Luminosité : ");
    Serial.println(ldrStr);

    // Allumer/éteindre la LED selon la luminosité
    if (pirValue == HIGH && ldrValue < 2000) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }

    
   
  }

  delay(500);  // Petit délai pour stabiliser les lectures
}
