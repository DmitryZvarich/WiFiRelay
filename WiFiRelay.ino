#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <OneButton.h>

const int relayPin = 2;
const int buttonPin = 3;

boolean curStatus = 0;

const long intervalMqttSetup = 300000;
unsigned long previousMqttSetupMillis = 0;


/*
const char* publishTopic = "/kitchen/light/status";
const char* subscribeTopic = "/kitchen/light/changestatus";
const char* clientName = "esp8266-kitchen-light";
*/
/*
const char* publishTopic = "/hall/light/status";
const char* subscribeTopic = "/hall/light/changestatus";
const char* clientName = "esp8266-hall-light";
*/

//washing machine
const char* publishTopic = "/bathroom/washingmachine/status";
const char* subscribeTopic = "/bathroom/washingmachine/changestatus";
const char* clientName = "esp8266-washing-machine";


WiFiClient wifiClient;
PubSubClient mqtt_client;

OneButton btn (buttonPin, true, true);

void mqtt_callback(char* topic, byte* payload, unsigned int len) 
{
  String tmp=topic;
  if(tmp.indexOf(subscribeTopic)>=0)
  {
    changeRelayStatus();
  }
}

void changeRelayStatus()
{
  curStatus = !curStatus;

  setRelayStatus();
}

void setRelayStatus()
{
  if (curStatus)
  {
    digitalWrite(relayPin, HIGH);
  }
  else
  {
    digitalWrite(relayPin, LOW);
  }
  
  EEPROM.begin(1);
  boolean eeprom;
  EEPROM.get(0, eeprom);
  
  if (curStatus != eeprom)
  {  
    EEPROM.put(0, curStatus);
    EEPROM.commit();
  }
  
  mqtt_client.publish(publishTopic, String(curStatus).c_str(), true);
}

void mqtt_setup() {
  if (!mqtt_client.connected()) {
    mqtt_client.setServer(mqtt_server, 14037);
    mqtt_client.setCallback(mqtt_callback);
    mqtt_client.setClient(wifiClient);

    mqtt_client.connect(clientName, mqtt_user, mqtt_password);
    mqtt_client.subscribe(subscribeTopic);
    
    mqtt_client.publish(publishTopic, String(curStatus).c_str(), true);
    }
}

void setup() {
  setRelayStatus();
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  
  EEPROM.begin(1);
  EEPROM.get(0, curStatus);

  setRelayStatus();

  btn.attachClick(changeRelayStatus);
  
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

  mqtt_setup();

  ArduinoOTA.setHostname(clientName);
  ArduinoOTA.begin();
}

void loop() {
  if(millis() - previousMqttSetupMillis >= intervalMqttSetup)
  {
    mqtt_setup();
    previousMqttSetupMillis = millis();
  }

  ArduinoOTA.handle();

  mqtt_client.loop();

  btn.tick();
}
