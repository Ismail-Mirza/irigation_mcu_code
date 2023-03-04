#include <WiFi.h>
#include <Arduino.h>
#include <HTTPClient.h>
// #include <DHT.h>

const char *ssid = "Slow NET!!!"; // Enter your WiFi Name

const char *password = "qwertyuiop"; // Enter your WiFi Password
String apiKey = "THFV3G18BMO4S3XX";  //  Enter your Write API key here
const char *server = "api.thingspeak.com";
#define DHTPIN 32 // GPIO Pin where the dht11 is connected
String server_name = "https://api.thingspeak.com/update?api_key=THFV3G18BMO4S3XX&field1=";
    // DHT dht(DHTPIN, DHT11);

WiFiClient client;
String translateEncryptionType(wifi_auth_mode_t encryptionType)
{

  switch (encryptionType)
  {
  case (WIFI_AUTH_OPEN):
    return "Open";
  case (WIFI_AUTH_WEP):
    return "WEP";
  case (WIFI_AUTH_WPA_PSK):
    return "WPA_PSK";
  case (WIFI_AUTH_WPA2_PSK):
    return "WPA2_PSK";
  case (WIFI_AUTH_WPA_WPA2_PSK):
    return "WPA_WPA2_PSK";
  case (WIFI_AUTH_WPA2_ENTERPRISE):
    return "WPA2_ENTERPRISE";
  }
}

void scanNetworks()
{

  int numberOfNetworks = WiFi.scanNetworks();

  Serial.print("Number of networks found: ");
  Serial.println(numberOfNetworks);

  for (int i = 0; i < numberOfNetworks; i++)
  {

    Serial.print("Network name: ");
    Serial.println(WiFi.SSID(i));

    Serial.print("Signal strength: ");
    Serial.println(WiFi.RSSI(i));

    Serial.print("MAC address: ");
    Serial.println(WiFi.BSSIDstr(i));

    Serial.print("Encryption type: ");
    String encryptionTypeDescription = translateEncryptionType(WiFi.encryptionType(i));
    Serial.println(encryptionTypeDescription);
    Serial.println("-----------------------");
  }
}

void connectToNetwork()
{
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Establishing connection to WiFi..");
  }

  Serial.println("Connected to network");
}


const int moisturePin = 32; // moisteure sensor pin

const int motorPin = 25;

unsigned long interval = 10000;

unsigned long previousMillis = 0;

unsigned long interval1 = 1000;

unsigned long previousMillis1 = 0;
float mois_in;

float moisturePercentage; // moisture reading
void sendThingspeak();

float h; // humidity reading

float t; // temperature reading
void setup()
{

  Serial.begin(115200);
  // dht.begin();

  scanNetworks();
  connectToNetwork();
  pinMode(motorPin,OUTPUT);

  Serial.println(WiFi.macAddress());
  Serial.println(WiFi.localIP());

  // WiFi.disconnect(true);
  //Serial.println(WiFi.localIP());
}

void loop()
{
  delay(500);

  unsigned long currentMillis = millis(); // grab current time
  
  h = 55; // read humiduty
  
  // Serial.println(h);

  t = 32; // read temperature

  if (isnan(h) || isnan(t))

  {

    Serial.println("Failed to read from DHT sensor!");

    return;
  }
  mois_in = analogRead(moisturePin);
  if (mois_in>4000)
  {
    Serial.println("out of soil");
    digitalWrite(motorPin, LOW);
    return;
  }
  Serial.println(mois_in);
  moisturePercentage = 1.85*(100 - (mois_in/4000)*100);
  Serial.println(moisturePercentage);


  if ((unsigned long)(currentMillis - previousMillis1) >= interval1)
  {

    Serial.print("Soil Moisture is  = ");

    Serial.print(moisturePercentage);

    Serial.println("%");

    previousMillis1 = millis();
  }

  if (moisturePercentage < 55)
  {

    Serial.println("I am in motor on");
    digitalWrite(motorPin, HIGH); // tun on motor
  }

  if (moisturePercentage > 55)
  {

    digitalWrite(motorPin, LOW); // turn off mottor
  }

  if ((unsigned long)(currentMillis - previousMillis) >= interval)
  {

    sendThingspeak(); // send data to thing speak

    previousMillis = millis();

    
  }
}

void sendThingspeak()
{

  if (client.connect(server, 80))

  {

    String postStr = apiKey; // add api key in the postStr string

    postStr += "&field1=";

    postStr += String(moisturePercentage); // add mositure readin

    postStr += "&field2=";

    postStr += String(t); // add tempr readin

    postStr += "&field3=";

    postStr += String(h); // add humidity readin

    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");

    client.print("Host: api.thingspeak.com\n");

    client.print("Connection: close\n");

    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");

    client.print("Content-Type: application/x-www-form-urlencoded\n");

    client.print("Content-Length: ");

    client.print(postStr.length()); // send lenght of the string

    client.print("\n\n");

    client.print(postStr); // send complete string

    Serial.print("Moisture Percentage: ");

    Serial.print(moisturePercentage);

    Serial.print("%. Temperature: ");

    Serial.print(t);

    Serial.print(" C, Humidity: ");

    Serial.print(h);

    Serial.println("%. Sent to Thingspeak.");
  }
}
