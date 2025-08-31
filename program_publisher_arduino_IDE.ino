#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Crypto.h>
#include <SHA256.h>
#include <AESLib.h>
#include "Base64.h"

const char* ssid = "AndroidShare_L5";
const char* password = "94756132";
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);
AESLib aesLib;

unsigned long lastMsg = 0;
int dataIdCounter = 0;

String lastEncryptedTemp = "";
String lastEncryptedHum = "";
String lastChecksum = "";
String lastID = "";
bool hasResend = false;

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

  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
}

const char* pre_shared_salt ="#y0uR$3cr3t54Lt*1sH3R3*";

byte aes_key[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                  0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};

String encryptData(String data) {
  byte iv[16] = {0};
  byte encrypted[128];

  char dataBuffer[128];
  data.toCharArray(dataBuffer, sizeof(dataBuffer));
  int inputLength = strlen(dataBuffer);

  uint16_t cipherLength = aesLib.encrypt((byte*)dataBuffer, inputLength, encrypted, aes_key, 128, iv);

  int base64Len = base64_enc_len(cipherLength);
  char base64Result[base64Len + 1];
  base64_encode(base64Result, (const char*)encrypted, cipherLength);
  base64Result[base64Len] = '\0';
  
  return String(base64Result);
}

String calculateChecksum(String data) {
  byte hash[32];
  SHA256 sha256;

  String saltedData = data + pre_shared_salt;

  sha256.update((const uint8_t*)saltedData.c_str(), saltedData.length());
  sha256.finalize(hash, sizeof(hash));

  String checksum = "";
  for (int i = 0; i < 32; i++) {
    checksum += String(hash[i], HEX);
  }
  return checksum;
}

String generateID() {
  dataIdCounter++;
  return ": " + String(dataIdCounter);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String received = "";
  for (unsigned int i = 0; i < length; i++) {
    received += (char)payload[i];
  }
  if (String(topic) == "/ESP32/IotTopic/resend" && received == "resend") {
    if (!hasResend) {
      hasResend = true;
      Serial.println("Data [RESEND] to subscriber!");

      client.publish("/ESP32/IotTopic/temp", lastEncryptedTemp.c_str());
      client.publish("/ESP32/IotTopic/hum", lastEncryptedHum.c_str());
      client.publish("/ESP32/IotTopic/checksum", lastChecksum.c_str());
      client.publish("/ESP32/IotTopic/id", lastID.c_str());
    } else {
      Serial.println("Resend request ignored (cooldown active!)");
    }
  }
}

int retryDelay = 1000;

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
      client.publish("/ESP32/Publish", "Welcome");
      client.subscribe("/ESP32/Subscribe");
      client.subscribe("/ESP32/IotTopic/resend");
      retryDelay = 1000;
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("Try again in few seconds");
      delay(retryDelay);
      retryDelay = min(retryDelay * 2, 16000);
    }
  }
}

void setup() {
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  dht.begin();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 25000) {
    lastMsg = now;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (isnan(h) || isnan(t)) {
      Serial.println("DHT Sensor Error");
      return;
    }

    String temp = String(t, 1);
    String hum = String(h, 1);

    lastEncryptedTemp = encryptData(temp);
    lastEncryptedHum = encryptData(hum);

    String combinedCipher = lastEncryptedTemp + lastEncryptedHum;
    lastChecksum = calculateChecksum(combinedCipher);
    lastID = generateID();

    client.publish("/ESP32/IotTopic/temp", lastEncryptedTemp.c_str());
    client.publish("/ESP32/IotTopic/hum", lastEncryptedHum.c_str());
    client.publish("/ESP32/IotTopic/checksum", lastChecksum.c_str());
    client.publish("/ESP32/IotTopic/id", lastID.c_str());

    hasResend = false;

    Serial.println("ID"+ lastID);
    Serial.print("Temperature: ");
    Serial.println(temp);
    Serial.print("Humidity: ");
    Serial.println(hum);
    Serial.print("Encrypted Temp: ");
    Serial.println(lastEncryptedTemp);
    Serial.print("Encrypted Hum: ");
    Serial.println(lastEncryptedHum);
    Serial.print("Hash: ");
    Serial.println(lastChecksum);
  }
}