#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>

const int relayPin = 2;

const char* publishTopic = "/hall/light/status";
const char* subscribeTopic = "/hall/light/changestatus";
const char* clientName = "esp8266-hall-light";

/*
//washing machine
const char* publishTopic = "/bathroom/washingmachine/status";
const char* subscribeTopic = "/bathroom/washingmachine/changestatus";
const char* clientName = "esp8266-washing-machine";
*/

WiFiClient wifiClient;
PubSubClient mqtt_client;

void mqtt_callback(char* topic, byte* payload, unsigned int len) 
{
  String tmp=topic;
  if(tmp.indexOf(subscribeTopic)>=0)
  {
    setRelayStatus((char)payload[0]);
  }
}

void setRelayStatus(char relayStatus)
{
  if (relayStatus != '0' && relayStatus != '1')
  {
    return;
  }

  if (relayStatus == '0')
  {
    digitalWrite(relayPin, LOW);
  }
  else
  {
    digitalWrite(relayPin, HIGH);
  }
  
  EEPROM.begin(1);
  char eeprom;
  EEPROM.get(0, eeprom);
  
  if (relayStatus != eeprom)
  {  
    EEPROM.put(0, relayStatus);
    EEPROM.commit();
  }
  
  mqtt_client.publish(publishTopic, String(relayStatus).c_str(), true);
}

void mqtt_setup() {
  if (!mqtt_client.connected()) {
    mqtt_client.setServer(mqtt_server, 14037);
    mqtt_client.setCallback(mqtt_callback);
    mqtt_client.setClient(wifiClient);

    mqtt_client.connect(clientName, mqtt_user, mqtt_password);
    mqtt_client.subscribe(subscribeTopic);
        
    char relayStatus;
    EEPROM.begin(1);
    EEPROM.get(0, relayStatus);
    setRelayStatus(relayStatus);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  WiFi.begin(wifi_ssid, wifi_password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }  
  //
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.setHostname(clientName);
  ArduinoOTA.begin();
}

void loop() {
  mqtt_setup();

  ArduinoOTA.handle();

  mqtt_client.loop();
}
