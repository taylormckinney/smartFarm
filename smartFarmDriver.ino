
#include <WiFiNINA.h>
#include <EduIntro.h>
#include <ArduinoMqttClient.h>
#include <ArduinoECCX08.h>
#include <ArduinoBearSSL.h>

//set up for sensors:
DHT11 dht11(D6);
float tempF;
float tempC;
float humidity;
int id = 0;
int moisture = 0;
int soilPin = A0;
int soilPower = 7;


//set up for mqtt:
const char ssid[]        = "asu-visitor";
const char pass[]        = "";
const char broker[]      = "a1hcvc2t4hd9wg-ats.iot.us-west-2.amazonaws.com";
const char certificate[] = R"(-----BEGIN CERTIFICATE-----
MIICujCCAaKgAwIBAgIURRomi6zU7Hrq64pcmILWHAUecfAwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTE5MDkyNjE4MTUy
NFoXDTQ5MTIzMTIzNTk1OVowSjELMAkGA1UECBMCbmMxDjAMBgNVBAcTBWJvb25l
MRowGAYDVQQKExFBcHBhbGFjaGlhbiBTdGF0ZTEPMA0GA1UEAxMGdGF5bG9yMFkw
EwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEB1AbqBOF/YKNwO8L/YyC0tjtin97Y+ha
lcsQBecjfiWVZawXhmA2Q8nBbcdBrCTGZ9cVASrLu1HtGbAMpQo24aNgMF4wHwYD
VR0jBBgwFoAUqX4VYuVcQhJwJlDhzkgMf7zZJsAwHQYDVR0OBBYEFABdgiDyYSMB
hRj/m3TXapL+g3iJMAwGA1UdEwEB/wQCMAAwDgYDVR0PAQH/BAQDAgeAMA0GCSqG
SIb3DQEBCwUAA4IBAQAVoEtCiRXoN+EUhK++09Kg7ikfWAjEUwgZ0IOorZUpKpQI
zH8EbJNcZq3D7d7+EfNWdzmTWTVlQcZCjQjCYBQUtULFe5lganxWR2nDfiuJzlWS
wrdbfL+eGZ5J0RG6oWMKwa2Vu+D/yejCslRxXMaNT2Thfwi13SWEtgU2bV/fMkKh
0oV9Oa62593HJG/ssh+JHinxHF72LZVZLwhb486hXsgFUhXHzGIkyxbHpC1o8faQ
+39ZmMYMK6kj5QfeZb67ZXXxthk7MukNXqhSzAW64h66/WlB4DX50HYx5ljHR8Jl
HZqeucWR6msCrIcQMbyjus16ONJv7vERExGj6hm6
-----END CERTIFICATE-----
)";

WiFiClient    wifiClient;           
BearSSLClient sslClient(wifiClient);
MqttClient    mqttClient(sslClient);

unsigned long lastMillis = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    while (1);
  }

  ArduinoBearSSL.onGetTime(getTime);
  sslClient.setEccSlot(0, certificate);

  pinMode(soilPower, OUTPUT);
  digitalWrite(soilPower, LOW);

}

void loop() {
   if (WiFi.status() != WL_CONNECTED) 
   {
    connectWiFi();
   }
  if (!mqttClient.connected()) 
  {
    connectMQTT();
  }
  mqttClient.poll();

  dht11.update();
  tempC = dht11.readCelsius(); 
  tempF = dht11.readFahrenheit();
  humidity = dht11.readHumidity();
  moisture = readSoil();
  publishMessage();

delay(30000);//wait 30 seconds to get new values
}


unsigned long getTime() 
{
  return WiFi.getTime();
}
void connectWiFi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.print(ssid);
  Serial.print(" ");

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the network");
  Serial.println();
}

void connectMQTT() 
{
  Serial.print("Attempting to MQTT broker: ");
  Serial.print(broker);
  Serial.println(" ");
  while (!mqttClient.connect(broker, 8883)) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();
  Serial.println("You're connected to the MQTT broker");
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe("arduino/incoming");
}

void publishMessage() {
  Serial.println("Publishing message");

  // send message
  id++;
  mqttClient.beginMessage("arduino/outgoing");
  mqttClient.print("{\"id\": \"");
  mqttClient.print(id);
  mqttClient.print(" \", \n ");
  mqttClient.print("\"tempF\": \"");
  mqttClient.print(tempF);
  mqttClient.print( "\", \n");
  mqttClient.print("\"tempC\": \"");
  mqttClient.print(tempC);
  mqttClient.print( "\", \n");
  mqttClient.print("\"humidity\": \"");
  mqttClient.print(humidity);
  mqttClient.print( "\", \n");
  mqttClient.print("\"moisture\": \"");
  mqttClient.print(moisture);
  mqttClient.print("\"}");

  mqttClient.endMessage();
 
}

//This is a function used to get the soil moisture content
int readSoil()
{

    digitalWrite(soilPower, HIGH);//turn D7 "On"
    delay(10);//wait 10 milliseconds 
    moisture = analogRead(soilPin);//Read the SIG value form sensor 
    digitalWrite(soilPower, LOW);//turn D7 "Off"
    return moisture;//send current moisture value
}
