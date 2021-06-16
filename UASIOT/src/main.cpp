#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
const int Fire_digital = D0;
const int Buzzer = D3;
const int redLED = D6;
const int Gas_digital = A0;
// inialisasi variabel data
int data;
int dataAsap;
//Nilai threshold
int sensorThres = 450;

const char *ssid = "Pak Rowi";      //silakan disesuaikan sendiri
const char *password = "tanyaayah"; //silakan disesuaikan sendiri

const char *mqtt_server = "ec2-34-226-220-168.compute-1.amazonaws.com";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    // String clientId = "ESP8266Client-";
    // clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect("ESP8266Client", "andri", "12345"))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
// ----------- program default/setting awal ----------- //
void setup()
{
  // inialisasi status I/O pin
  pinMode(Fire_digital, INPUT); // pin sebagai input
  pinMode(Gas_digital, INPUT);  // pin sebagai input
  pinMode(Buzzer, OUTPUT);      // pin sebagai output
  pinMode(redLED, OUTPUT);
  Serial.begin(115200);
  Serial.println("Mqtt Node-RED");

  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

// ----- program utama, looping/berulang terus-menerus ----- //
void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop(); // iki batese seng ojok tok ganti nggae connect nak wifi karo nodered
  unsigned long now = millis();
  if (now - lastMsg > 100)
  {
    lastMsg = now;
    ++value;
    // variabel data adalah hasil dari pembacaan sensor
    // berupa logic LOW/HIGH
    data = digitalRead(Fire_digital);
    int analogSensor = analogRead(Gas_digital);

    Serial.print("Pin A0: ");
    Serial.println(analogSensor);

    // jika data pada sensor API bernilai logic LOW
    if (data == LOW)
    {
      // alarm dinyalakan
      digitalWrite(Buzzer, HIGH);
      digitalWrite(redLED, HIGH);
      delay(100);
    }

    // jika data Analog pada sensor Asap lebih besar dari batas
    else if (analogSensor > sensorThres)
    {
      digitalWrite(redLED, HIGH);
      tone(Buzzer, 1000, 200);
      delay(100);
    }

    // jika data tidak bernilai logic LOW
    else
    {
      //Lampu Dimatikan
      digitalWrite(redLED, LOW);
      // alarm dimatikan
      digitalWrite(Buzzer, LOW);
      delay(100);
    }

    static char gasTemp[7];
    dtostrf(analogSensor, 4, 2, gasTemp);
    // Serial.println(gasTemp);
    client.publish("room/gas", gasTemp);
  }
}
