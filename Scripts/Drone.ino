#include <SoftwareSerial.h>
#include <QMC5883LCompass.h>
#include <TinyGPSPlus.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include <Wire.h>
#include <sSense-BMx280I2C.h>
#include <BMx280_EnvCalc.h>
#include <Adafruit_SGP30.h>

const char APN[] = "net";
const char SERVER_IP[] = "karlmarx.home.ro";
const char SERVER_PORT[] = "25565";

const int SIM800_RX_PIN = 4;
const int SIM800_TX_PIN = 5;
const int GAS_SENSOR_MQ5_PIN = A1;
const int GAS_SENSOR_MQ7_PIN = A2;
const int GPS_RX_PIN = 3;
const int GPS_TX_PIN = 2;
const uint32_t GPS_BAUD = 9600;
const float ACCEL_SCALE_FACTOR = 16384.0;
const float GRAVITY_MS2 = 9.81;

SoftwareSerial sim800l(SIM800_RX_PIN, SIM800_TX_PIN);
QMC5883LCompass compass;
TinyGPSPlus gps;
SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);
MPU6050 mpu;
BMx280I2C bme;
Adafruit_SGP30 sgp;

char compassDirection[4];
int gasAnalogValue;
int coAnalogValue;
float gpsLatitude = 0.0;
float gpsLongitude = 0.0;
float gpsAltitude = 0.0;
float gpsSpeed = 0.0;
uint32_t sats = 0;
float accelX, accelY, accelZ;
float temperature, humidity, pressure;
uint16_t tvocLevel, co2Level;

unsigned long lastSendTime = 0;
const long sendInterval = 5000;

void setup() {
    Serial.begin(9600);
    sim800l.begin(9600);
    gpsSerial.begin(GPS_BAUD);

    Wire.begin();
    Wire.setClock(400000);

    Serial.println(F("--- Universal Sensor & GPRS Initializer ---"));
    delay(1000);

    initializeSensors();
    initializeGPRS();

    Serial.println(F("\n--- Setup complete. Starting main loop. ---\n"));
    lastSendTime = millis();
}

void loop() {
    gpsSerial.listen();
    
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }

    if (millis() - lastSendTime >= sendInterval) {
        lastSendTime = millis();

        Serial.println(F("\n========================================"));
        Serial.println(F("Interval reached. Reading sensors and sending data..."));

        readAllSensors();
        sendData();

        Serial.println(F("========================================"));
    }
}

void readAllSensors() {
    Serial.println(F("--- Reading all sensor data... ---"));
    if (gps.location.isUpdated()) {
        gpsLatitude = gps.location.lat();
        gpsLongitude = gps.location.lng();
    }
    if (gps.altitude.isUpdated()) gpsAltitude = gps.altitude.meters();
    if (gps.speed.isUpdated()) gpsSpeed = gps.speed.kmph();
    if (gps.satellites.isUpdated()) sats = gps.satellites.value();

    compass.read();
    compass.getDirection(compassDirection, compass.getAzimuth());

    gasAnalogValue = analogRead(GAS_SENSOR_MQ5_PIN);
    coAnalogValue = analogRead(GAS_SENSOR_MQ7_PIN);

    int16_t raw_ax, raw_ay, raw_az;
    mpu.getAcceleration(&raw_ax, &raw_ay, &raw_az);
    accelX = (raw_ax / ACCEL_SCALE_FACTOR) * GRAVITY_MS2;
    accelY = (raw_ay / ACCEL_SCALE_FACTOR) * GRAVITY_MS2;
    accelZ = (raw_az / ACCEL_SCALE_FACTOR) * GRAVITY_MS2;

    bme.read(pressure, temperature, humidity, BME280::TempUnit_Celsius, BME280::PresUnit_Pa);
    if (humidity > 0) sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));
    if (sgp.IAQmeasure()) {
        tvocLevel = sgp.TVOC;
        co2Level = sgp.eCO2;
    }
    printData();
}

void sendData() {
    sendHttpPost();
}

void initializeSensors() {
    Serial.println(F("--- Initializing Sensors ---"));
    Serial.print(F("QMC5883L... "));
    compass.init();
    Serial.println(F("Done."));
    Serial.print(F("MPU6050... "));
    mpu.initialize();
    if (mpu.testConnection()) {
        Serial.println(F("OK. Setting offsets..."));
        mpu.setXAccelOffset(1690); mpu.setYAccelOffset(870); mpu.setZAccelOffset(4640);
        mpu.setXGyroOffset(-46); mpu.setYGyroOffset(68); mpu.setZGyroOffset(88);
    } else {
        Serial.println(F("Failed."));
    }
    Serial.print(F("BME280... "));
    if (!bme.begin()) Serial.println(F("Failed."));
    else Serial.println(F("Done."));
    Serial.print(F("SGP30... "));
    if (!sgp.begin()) Serial.println(F("Failed."));
    else Serial.println(F("Done."));
    Serial.println(F("--- Sensor Initialization Complete ---"));
}

void initializeGPRS() {
    sim800l.listen();
    Serial.println(F("--- Initializing SIM800L Module ---"));
    delay(3000);
    if (!checkModule()) {
        Serial.println(F("FATAL: SIM800L module not responding. Halting."));
        while (1);
    }
    sendATCommand(F("AT+HTTPTERM"), F("OK"), 5000);
    sendATCommand(F("AT+SAPBR=0,1"), F("OK"), 5000);
    setupGPRS();
}

unsigned int countDigits(int number) {
  if (number < 0) return countDigits(-number) + 1;
  if (number < 10) return 1;
  if (number < 100) return 2;
  if (number < 1000) return 3;
  if (number < 10000) return 4;
  return 5;
}

bool expectResponse(const __FlashStringHelper* ack, unsigned int timeout) {
    String response = "";
    response.reserve(64);
    long int startTime = millis();
    bool ackFound = false;
    while ((millis() - startTime) < timeout) {
        while (sim800l.available()) {
            response += (char)sim800l.read();
        }
        if (response.indexOf(ack) != -1) {
            ackFound = true;
            break;
        }
    }
    Serial.print(F("<<< Response: "));
    Serial.println(response);
    return ackFound;
}

bool sendATCommand(const __FlashStringHelper* cmd, const __FlashStringHelper* ack, unsigned int timeout) {
    Serial.print(F(">>> AT: "));
    Serial.println(cmd);
    sim800l.println(cmd);
    return expectResponse(ack, timeout);
}

bool checkModule() {
    return sendATCommand(F("AT"), F("OK"), 2000);
}

void setupGPRS() {
    Serial.println(F("--- Configuring GPRS ---"));
    sendATCommand(F("AT+CREG?"), F("+CREG: 0,1"), 10000);
    sendATCommand(F("AT+CGATT=1"), F("OK"), 5000);
    sim800l.print(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n"));
    expectResponse(F("OK"), 2000);
    sim800l.print(F("AT+SAPBR=3,1,\"APN\",\""));
    sim800l.print(APN);
    sim800l.print(F("\"\r\n"));
    expectResponse(F("OK"), 2000);
    sendATCommand(F("AT+SAPBR=1,1"), F("OK"), 20000);
    sendATCommand(F("AT+SAPBR=2,1"), F("+SAPBR:"), 5000);
    Serial.println(F("--- GPRS Configuration Complete ---"));
}

void sendHttpPost() {
    sim800l.listen();
    Serial.println(F("--- Starting HTTP POST ---"));
    if (!sendATCommand(F("AT+HTTPINIT"), F("OK"), 5000)) return;

    sim800l.print(F("AT+HTTPPARA=\"URL\",\"http://"));
    sim800l.print(SERVER_IP);
    sim800l.print(F(":"));
    sim800l.print(SERVER_PORT);
    sim800l.print(F("/data\"\r\n"));
    if (!expectResponse(F("OK"), 2000)) return;

    sendATCommand(F("AT+HTTPPARA=\"CONTENT\",\"application/json\""), F("OK"), 2000);

    char tempBuffer[12];
    int jsonLength = 0;
    jsonLength += strlen_P(PSTR("{\"path\":1,\"timestamp\":\"2025-01-01 00:00:00\",\"longitude\":,\"latitude\":,\"temperature\":,\"humidity\":,\"pressure\":,\"CO2level\":,\"TVOClevel\":,\"imuXaccel\":,\"imuYaccel\":,\"imuZaccel\":,\"COlevel\":,\"Gaslevel\":,\"altitude\":,\"speed\":,\"sats\":,\"direction\":\"\"}"));
    jsonLength += strlen(dtostrf(gpsLongitude, 0, 6, tempBuffer));
    jsonLength += strlen(dtostrf(gpsLatitude, 0, 6, tempBuffer));
    jsonLength += strlen(dtostrf(temperature, 0, 2, tempBuffer));
    jsonLength += strlen(dtostrf(humidity, 0, 2, tempBuffer));
    jsonLength += strlen(dtostrf(pressure, 0, 0, tempBuffer));
    jsonLength += countDigits(co2Level);
    jsonLength += countDigits(tvocLevel);
    jsonLength += strlen(dtostrf(accelX, 0, 2, tempBuffer));
    jsonLength += strlen(dtostrf(accelY, 0, 2, tempBuffer));
    jsonLength += strlen(dtostrf(accelZ, 0, 2, tempBuffer));
    jsonLength += countDigits(coAnalogValue);
    jsonLength += countDigits(gasAnalogValue);
    jsonLength += strlen(dtostrf(gpsAltitude, 0, 2, tempBuffer));
    jsonLength += strlen(dtostrf(gpsSpeed, 0, 2, tempBuffer));
    jsonLength += countDigits(sats);
    jsonLength += strlen(compassDirection);

    sim800l.print(F("AT+HTTPDATA="));
    sim800l.print(jsonLength);
    sim800l.println(F(",10000"));

    if (expectResponse(F("DOWNLOAD"), 5000)) {
        Serial.println(F(">>> Sending payload piece by piece..."));
        sim800l.print(F("{\"path\":1,\"timestamp\":\"2025-01-01 00:00:00\""));
        sim800l.print(F(",\"longitude\":")); sim800l.print(dtostrf(gpsLongitude, 0, 6, tempBuffer));
        sim800l.print(F(",\"latitude\":")); sim800l.print(dtostrf(gpsLatitude, 0, 6, tempBuffer));
        sim800l.print(F(",\"temperature\":")); sim800l.print(dtostrf(temperature, 0, 2, tempBuffer));
        sim800l.print(F(",\"humidity\":")); sim800l.print(dtostrf(humidity, 0, 2, tempBuffer));
        sim800l.print(F(",\"pressure\":")); sim800l.print(dtostrf(pressure, 0, 0, tempBuffer));
        sim800l.print(F(",\"CO2level\":")); sim800l.print(co2Level);
        sim800l.print(F(",\"TVOClevel\":")); sim800l.print(tvocLevel);
        sim800l.print(F(",\"imuXaccel\":")); sim800l.print(dtostrf(accelX, 0, 2, tempBuffer));
        sim800l.print(F(",\"imuYaccel\":")); sim800l.print(dtostrf(accelY, 0, 2, tempBuffer));
        sim800l.print(F(",\"imuZaccel\":")); sim800l.print(dtostrf(accelZ, 0, 2, tempBuffer));
        sim800l.print(F(",\"COlevel\":")); sim800l.print(coAnalogValue);
        sim800l.print(F(",\"Gaslevel\":")); sim800l.print(gasAnalogValue);
        sim800l.print(F(",\"altitude\":")); sim800l.print(dtostrf(gpsAltitude, 0, 2, tempBuffer));
        sim800l.print(F(",\"speed\":")); sim800l.print(dtostrf(gpsSpeed, 0, 2, tempBuffer));
        sim800l.print(F(",\"sats\":")); sim800l.print(sats);
        sim800l.print(F(",\"direction\":\"")); sim800l.print(compassDirection); sim800l.print(F("\""));
        sim800l.print(F("}"));
        sim800l.println();

        if (!expectResponse(F("OK"), 10000)) {
            Serial.println(F("Error: Payload send failed."));
        }
    } else {
        Serial.println(F("Error setting HTTP data."));
    }

    if (sendATCommand(F("AT+HTTPACTION=1"), F("+HTTPACTION: 1,200"), 20000)) {
        Serial.println(F("Success: POST sent, server returned 200 OK."));
    } else {
        Serial.println(F("Error: HTTP POST failed."));
    }

    sendATCommand(F("AT+HTTPREAD"), F("OK"), 5000);
    sendATCommand(F("AT+HTTPTERM"), F("OK"), 5000);
    Serial.println(F("--- HTTP POST Finished ---"));
}

void printData() {
    Serial.println(F("-------------------- SENSOR DATA --------------------"));
    Serial.print(F("Compass: ")); Serial.print(compassDirection);
    Serial.print(F("\t| Gas (MQ-5): ")); Serial.print(gasAnalogValue);
    Serial.print(F("\t| CO (MQ-7): ")); Serial.println(coAnalogValue);
    Serial.print(F("GPS: ")); Serial.print(gpsLatitude, 6); Serial.print(F(", ")); Serial.print(gpsLongitude, 6);
    Serial.print(F(" | Alt: ")); Serial.print(gpsAltitude);
    Serial.print(F(" m | Spd: ")); Serial.print(gpsSpeed);
    Serial.print(F(" km/h | Sats: ")); Serial.println(sats);
    Serial.print(F("IMU Accel (X/Y/Z): ")); Serial.print(accelX, 2); Serial.print(F(" / ")); Serial.print(accelY, 2); Serial.print(F(" / ")); Serial.print(accelZ, 2); Serial.println(F(" m/s^2"));
    Serial.print(F("Env: ")); Serial.print(temperature, 2); Serial.print(F(" C | ")); Serial.print(humidity, 2); Serial.print(F(" % | ")); Serial.print(pressure, 0); Serial.println(F(" Pa"));
    Serial.print(F("Air Quality: TVOC ")); Serial.print(tvocLevel); Serial.print(F(" ppb | eCO2 ")); Serial.print(co2Level); Serial.println(F(" ppm"));
    Serial.println(F("-----------------------------------------------------\n"));
}

uint32_t getAbsoluteHumidity(float temp, float hum) {
    const float absoluteHumidity = 216.7f * ((hum / 100.0f) * 6.112f * exp((17.62f * temp) / (243.12f + temp)) / (273.15f + temp));
    return static_cast<uint32_t>(1000.0f * absoluteHumidity);
}
