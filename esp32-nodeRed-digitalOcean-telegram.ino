#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN 23
#define DHTTYPE DHT11   // DHT 11
#define MOTOR_RELAY_PIN 21

const char* ssid = "Nama WiFi Anda";
const char* password = "Password WiFi Anda";
const char* mqtt_server = "xxx.xx.xxx.xx"; // Isikan dengan IP address server anda

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

DHT dht(DHTPIN, DHTTYPE);
float temperature = 0;
float humidity = 0;

const int ledPin = 13;
  
void setup() {
  Serial.begin(115200);

  dht.begin();  // initialize DHT

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(ledPin, OUTPUT);
  pinMode(MOTOR_RELAY_PIN, OUTPUT); 
  digitalWrite(MOTOR_RELAY_PIN, LOW);
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on") {
      Serial.println("on");
      digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off") {
      Serial.println("off");
      digitalWrite(ledPin, LOW);
    }
  }
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
    int check = dht.read(DHTPIN);
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    
    char tempString[8];
    dtostrf(temperature, 1, 2, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    client.publish("esp32/temperature", tempString);

    char humString[8];
    dtostrf(humidity, 1, 2, humString);
    Serial.print("Humidity: ");
    Serial.println(humString);
    client.publish("esp32/humidity", humString);

    // Check humidity and control the motor accordingly
    if (humidity < 90) {
      Serial.println("Humidity is below 90%. Activating the motor.");
      digitalWrite(MOTOR_RELAY_PIN, HIGH); // Activate the motor
    } else {
      Serial.println("Humidity is 90% or above. Deactivating the motor.");
      digitalWrite(MOTOR_RELAY_PIN, LOW); // Deactivate the motor
    }
  }
}
