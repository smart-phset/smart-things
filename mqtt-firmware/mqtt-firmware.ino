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
bool loggedWifiConnected=false, loggedMqttConnected=false, loggedTimeReady=false;
String lastCommandId[3];
uint32_t lastWifi=0,lastMqtt=0,lastTelemetry=0,lastState=0,pumpStarted=0;
String commandPrefix=String("smartphset/devices/")+DEVICE_ID+"/commands/";
String stateTopic=String("smartphset/devices/")+DEVICE_ID+"/state";
String telemetryTopic=String("smartphset/devices/")+DEVICE_ID+"/telemetry";

void setOutput(int index,bool on) {
    if(index==2 && on && !outputs[index])pumpStarted=millis();
    outputs[index]=on;
    int level=on==bool(ACTUATOR_ACTIVE_HIGH)?HIGH:LOW;
    digitalWrite(OUTPUT_PINS[index],level);
    const char* names[]={"FAN","LIGHT","PUMP"};
    Serial.printf("%s -> %s | GPIO%d -> %s\n",
        names[index],on?"ON":"OFF",OUTPUT_PINS[index],level==HIGH?"HIGH":"LOW");
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
    if(length>384){Serial.println("Command rejected: payload too large");return;}
    String name(topic);if(!name.startsWith(commandPrefix))return;
    String actuator=name.substring(commandPrefix.length());
    int index=actuatorIndex(actuator.c_str());
    if(index<0){Serial.println("Command rejected: invalid actuator");return;}
    JsonDocument doc;
    if(deserializeJson(doc,payload,length)){Serial.println("Command rejected: invalid JSON");return;}
    if(!doc["on"].is<bool>()){Serial.println("Command rejected: invalid on field");return;}
    if(!doc["command_id"].is<const char*>()){Serial.println("Command rejected: invalid command_id");return;}
    if(!doc["expires_at"].is<int64_t>()){Serial.println("Command rejected: invalid expires_at");return;}
    const char* id=doc["command_id"];
    if(strlen(id)==0 || strlen(id)>64){Serial.println("Command rejected: invalid command_id");return;}
    if(!freshCommand(doc["expires_at"].as<int64_t>(),time(nullptr))) {
        Serial.println("Command rejected: expiry outside allowed window or device time invalid");return;
    }
    if(lastCommandId[index]==id){Serial.println("Command ignored: duplicate command_id");return;}
    lastCommandId[index]=id;
    // Exact ON/OFF assignment is idempotent; no toggle commands accepted.
    const char* names[]={"FAN","LIGHT","PUMP"};
    Serial.printf("Command accepted: %s -> %s\n",names[index],doc["on"].as<bool>()?"ON":"OFF");
    setOutput(index,doc["on"].as<bool>());
}
void setup() {
    Serial.begin(115200);
    for(int pin:OUTPUT_PINS) {
        digitalWrite(pin,ACTUATOR_ACTIVE_HIGH?LOW:HIGH);
        pinMode(pin,OUTPUT);
    }
    allOff();dht.begin();analogReadResolution(12);
    WiFi.mode(WIFI_STA);
    Serial.printf("WiFi: connecting to %s...\n",WIFI_SSID);
    WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
    configTime(0,0,NTP_SERVER);
    Serial.println("Time/NTP: synchronization requested");
    loggedTimeReady=time(nullptr)>=1700000000;
    Serial.println(loggedTimeReady?"Time/NTP: READY":"Time/NTP: NOT READY");
    if(loggedTimeReady)Serial.printf("Time UTC: %s\n",capturedAt().c_str());
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
    bool wifiConnected=WiFi.status()==WL_CONNECTED;
    if(wifiConnected!=loggedWifiConnected) {
        loggedWifiConnected=wifiConnected;
        if(wifiConnected) {
            Serial.println("WiFi: connected");
            Serial.printf("WiFi: IP = %s\n",WiFi.localIP().toString().c_str());
            Serial.printf("WiFi: RSSI = %ld dBm\n",(long)WiFi.RSSI());
        } else Serial.println("WiFi: disconnected");
    }
    if(loggedMqttConnected && !online) {
        loggedMqttConnected=false;
        Serial.printf("MQTT: disconnected | state = %d\n",mqtt.state());
    }
    bool timeReady=time(nullptr)>=1700000000;
    if(timeReady!=loggedTimeReady) {
        loggedTimeReady=timeReady;
        Serial.println(timeReady?"Time/NTP: READY":"Time/NTP: NOT READY");
        if(timeReady)Serial.printf("Time UTC: %s\n",capturedAt().c_str());
    }
    if(WiFi.status()!=WL_CONNECTED) {
        if(elapsed(now,lastWifi,5000)) {
            lastWifi=now;
            Serial.printf("WiFi: reconnecting to %s...\n",WIFI_SSID);
            WiFi.reconnect();
        }
        return;
    }
    if(!mqtt.connected()) {
        if(elapsed(now,lastMqtt,5000)) {
            lastMqtt=now;
            Serial.printf("MQTT: connecting to %s:%u...\n",MQTT_HOST,(unsigned int)MQTT_PORT);
            bool connected=mqtt.connect(DEVICE_ID,MQTT_USERNAME,MQTT_PASSWORD);
            if(connected) {
                Serial.println("MQTT: connected");
                loggedMqttConnected=true;
                allOff();
                String subscription=commandPrefix+"+";
                if(!mqtt.subscribe(subscription.c_str(),0)) {
                    Serial.printf("MQTT: subscription failed: %s\n",subscription.c_str());
                    mqtt.disconnect();
                    loggedMqttConnected=false;
                    Serial.printf("MQTT: disconnected | state = %d\n",mqtt.state());
                } else Serial.printf("MQTT: subscribed to %s\n",subscription.c_str());
                stateDirty=true;
            } else Serial.printf("MQTT: connection failed | state = %d\n",mqtt.state());
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
            Serial.println("Telemetry skipped:");
            if(!std::isfinite(temperature))Serial.println(" - temperature invalid");
            if(!std::isfinite(humidity))Serial.println(" - humidity invalid");
            if(!std::isfinite(moisture))Serial.println(" - soil mapped value invalid");
            if(timestamp.isEmpty())Serial.println(" - time not synchronized");
            return;
        }
        JsonDocument doc;doc["device_id"]=DEVICE_ID;doc["captured_at"]=timestamp;
        doc["temperature_c"]=temperature;doc["humidity_percent"]=humidity;doc["soil_moisture_percent"]=moisture;
        publishJson(telemetryTopic,doc);
    }
}
