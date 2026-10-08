#include <DHT.h>

// ==========================
// PIN CONFIGURATION
// ==========================

// DHT11
#define DHT_PIN 4
#define DHT_TYPE DHT11

// Soil moisture
#define SOIL_PIN 34

// Outputs
#define FAN_PIN   25
#define LIGHT_PIN 26
#define PUMP_PIN  27

DHT dht(DHT_PIN, DHT_TYPE);

// ==========================
// SETUP
// ==========================
void setup() {
  Serial.begin(115200);

  dht.begin();

  pinMode(FAN_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);

  // Start everything OFF
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(LIGHT_PIN, LOW);
  digitalWrite(PUMP_PIN, LOW);

  Serial.println();
  Serial.println("==============================");
  Serial.println(" Smart Greenhouse ESP32");
  Serial.println("==============================");
  Serial.println("Commands:");
  Serial.println("fan on");
  Serial.println("fan off");
  Serial.println("light on");
  Serial.println("light off");
  Serial.println("water on");
  Serial.println("water off");
  Serial.println("==============================");
}

// ==========================
// LOOP
// ==========================
void loop() {

  readSensors();
  handleCommands();

  delay(2000);
}

// ==========================
// SENSOR READING
// ==========================
void readSensors() {

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  int soilRaw = analogRead(SOIL_PIN);

  Serial.println();
  Serial.println("------ SENSOR DATA ------");

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("DHT11 reading failed!");
  } else {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  }

  Serial.print("Soil raw value: ");
  Serial.println(soilRaw);

  Serial.println("-------------------------");
}

// ==========================
// SERIAL COMMANDS
// ==========================
void handleCommands() {

  if (Serial.available()) {

    String command = Serial.readStringUntil('\n');

    command.trim();
    command.toLowerCase();

    if (command == "fan on") {

      digitalWrite(FAN_PIN, HIGH);
      Serial.println("Fan: ON");

    } else if (command == "fan off") {

      digitalWrite(FAN_PIN, LOW);
      Serial.println("Fan: OFF");

    } else if (command == "light on") {

      digitalWrite(LIGHT_PIN, HIGH);
      Serial.println("Light: ON");

    } else if (command == "light off") {

      digitalWrite(LIGHT_PIN, LOW);
      Serial.println("Light: OFF");

    } else if (command == "water on") {

      digitalWrite(PUMP_PIN, HIGH);
      Serial.println("Water Pump: ON");

    } else if (command == "water off") {

      digitalWrite(PUMP_PIN, LOW);
      Serial.println("Water Pump: OFF");

    } else {

      Serial.print("Unknown command: ");
      Serial.println(command);
    }
  }
}