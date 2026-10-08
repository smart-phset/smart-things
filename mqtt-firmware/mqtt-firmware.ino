#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <sys/time.h>
#include "config.h"
#include "policy.h"

constexpr int DHT_PIN=4, SOIL_PIN=34;
constexpr int OUTPUT_PINS[]={25,26,27}; // fan, light, pump
DHT dht(DHT_PIN,DHT11);
WiFiClient network;
PubSubClient mqtt(network);
bool outputs[3]={false,false,false};
bool stateDirty=true, wasOnline=false;
String lastCommandId[3];
uint32_t lastWifi=0,lastMqtt=0,lastTelemetry=0,lastState=0,pumpStarted=0;
String commandPrefix=String("smartphset/devices/")+DEVICE_ID+"/commands/";
String stateTopic=String("smartphset/devices/")+DEVICE_ID+"/state";
String telemetryTopic=String("smartphset/devices/")+DEVICE_ID+"/telemetry";

void setOutput(int index,bool on) {
    if(index==2 && on && !outputs[index])pumpStarted=millis();
    outputs[index]=on;
    digitalWrite(OUTPUT_PINS[index],on==bool(ACTUATOR_ACTIVE_HIGH)?HIGH:LOW);
    stateDirty=true;
}
void allOff() {for(int i=0;i<3;i++)setOutput(i,false);}
String capturedAt() {
    timeval tv;gettimeofday(&tv,nullptr);
    if(tv.tv_sec<1700000000)return ""; // Never invent timestamps before NTP sync.
    tm utc;gmtime_r(&tv.tv_sec,&utc);
    char date[32];strftime(date,sizeof(date),"%Y-%m-%dT%H:%M:%S",&utc);
    char stamp[40];snprintf(stamp,sizeof(stamp),"%s.%06ldZ",date,(long)tv.tv_usec);
    return String(stamp);
}
void publishJson(const String& topic,JsonDocument& doc) {
    char buffer[768];size_t size=measureJson(doc);
    if(size>=sizeof(buffer))return;
    serializeJson(doc,buffer,sizeof(buffer));
    if(!mqtt.publish(topic.c_str(),buffer,false))Serial.println("MQTT publish failed");
}
void publishState() {
    String timestamp=capturedAt();if(timestamp.isEmpty())return;
    JsonDocument doc;doc["device_id"]=DEVICE_ID;doc["captured_at"]=timestamp;
    doc["fan"]=outputs[0];doc["light"]=outputs[1];doc["pump"]=outputs[2];
    char buffer[512];serializeJson(doc,buffer,sizeof(buffer));
    if(mqtt.publish(stateTopic.c_str(),buffer,false))stateDirty=false;
}
void command(char* topic,byte* payload,unsigned int length) {
    if(length>384)return;
    String name(topic);if(!name.startsWith(commandPrefix))return;
    int index=actuatorIndex(name.substring(commandPrefix.length()).c_str());
    if(index<0)return;
    JsonDocument doc;
    if(deserializeJson(doc,payload,length) || !doc["on"].is<bool>()
            || !doc["command_id"].is<const char*>() || !doc["expires_at"].is<int64_t>())return;
    const char* id=doc["command_id"];
    if(strlen(id)==0 || strlen(id)>64 || !freshCommand(doc["expires_at"].as<int64_t>(),time(nullptr)))return;
    if(lastCommandId[index]==id)return;
    lastCommandId[index]=id;
    // Exact ON/OFF assignment is idempotent; no toggle commands accepted.
    setOutput(index,doc["on"].as<bool>());
}
void setup() {
    Serial.begin(115200);
    for(int pin:OUTPUT_PINS) {
        digitalWrite(pin,ACTUATOR_ACTIVE_HIGH?LOW:HIGH);
        pinMode(pin,OUTPUT);
    }
    allOff();dht.begin();analogReadResolution(12);
    WiFi.mode(WIFI_STA);WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
    configTime(0,0,NTP_SERVER);
    mqtt.setServer(MQTT_HOST,MQTT_PORT);mqtt.setCallback(command);
    mqtt.setBufferSize(1024);mqtt.setKeepAlive(15);mqtt.setSocketTimeout(1);
    lastWifi=millis();lastMqtt=millis()-5000;
}
void loop() {
    uint32_t now=millis();
    // Enforce pump limit before any bounded network/DHT operations.
    if(outputs[2] && elapsed(now,pumpStarted,MAX_PUMP_ON_MS))setOutput(2,false);
    bool online=WiFi.status()==WL_CONNECTED && mqtt.connected();
    if(wasOnline && !online)allOff(); // Fail-safe on observed Wi-Fi/broker loss.
    wasOnline=online;
    if(WiFi.status()!=WL_CONNECTED) {
        if(elapsed(now,lastWifi,5000)){lastWifi=now;WiFi.reconnect();}
        return;
    }
    if(!mqtt.connected()) {
        if(elapsed(now,lastMqtt,5000)) {
            lastMqtt=now;
            bool connected=mqtt.connect(DEVICE_ID,MQTT_USERNAME,MQTT_PASSWORD);
            if(connected) {
                allOff();
                if(!mqtt.subscribe((commandPrefix+"+").c_str(),0))mqtt.disconnect();
                stateDirty=true;
            }
        }
        return;
    }
    mqtt.loop();
    if(stateDirty || elapsed(now,lastState,5000)) {lastState=now;publishState();}
    if(elapsed(now,lastTelemetry,5000)) {
        lastTelemetry=now;
        float humidity=dht.readHumidity(),temperature=dht.readTemperature();
        float moisture=moisturePercent(analogRead(SOIL_PIN),SOIL_DRY_ADC,SOIL_WET_ADC);
        String timestamp=capturedAt();
        if(!std::isfinite(temperature)||!std::isfinite(humidity)||!std::isfinite(moisture)||timestamp.isEmpty()) {
            Serial.println("Sensor/time unavailable; telemetry skipped");return;
        }
        JsonDocument doc;doc["device_id"]=DEVICE_ID;doc["captured_at"]=timestamp;
        doc["temperature_c"]=temperature;doc["humidity_percent"]=humidity;doc["soil_moisture_percent"]=moisture;
        publishJson(telemetryTopic,doc);
    }
}
