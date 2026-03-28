#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <sys/time.h>
#include <ArduinoJson.h>

// --- Network & Backend Configuration ---
const char* ssid       = "A1_829B";
const char* password   = "58966679";
const char* serverUrl  = "http://ec2-16-16-142-88.eu-north-1.compute.amazonaws.com:8080/medication";

// --- Device Credentials ---
const char* DEVICE_ID  = "medbox-001";
const char* DEVICE_KEY = "dev-key-medbox-001-secret";

// --- Hardware Pins ---
const int LOCK_PIN   = 25; 
const int BUTTON_PIN = 14; 

// --- Schedule (Set to 05:55 AM for your test) ---
const int UNLOCK_HOUR   = 5; 
const int UNLOCK_MINUTE = 55;

// --- State Variables ---
bool isWaitingForButton = false;
int lastUnlockDay       = -1; 
int lastPrintedMinute   = -1;

void setup() {
  Serial.begin(115200);

  pinMode(LOCK_PIN, OUTPUT);
  digitalWrite(LOCK_PIN, HIGH); // Locked by default
  
  pinMode(BUTTON_PIN, INPUT_PULLUP); 

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // Bulgaria Timezone: EET-2EEST,M3.5.0/3,M10.5.0/4
  configTzTime("EET-2EEST,M3.5.0/3,M10.5.0/4", "pool.ntp.org", "time.google.com");
  
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Syncing time...");
    delay(1000);
  }
  Serial.println("Time Synchronized. Lock will open at 05:55 AM.");
}

void loop() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  // Log status once per minute
  if (timeinfo.tm_min != lastPrintedMinute) {
    lastPrintedMinute = timeinfo.tm_min;
    Serial.printf("[%02d:%02d] Box is %s\n", 
                  timeinfo.tm_hour, timeinfo.tm_min, 
                  isWaitingForButton ? "UNLOCKED - Waiting for button" : "LOCKED");
  }

  // 1. Check if it's 05:55
  if (!isWaitingForButton && timeinfo.tm_hour == UNLOCK_HOUR && timeinfo.tm_min == UNLOCK_MINUTE) {
    if (lastUnlockDay != timeinfo.tm_mday) {
      digitalWrite(LOCK_PIN, LOW); // UNLOCK
      isWaitingForButton = true;
      lastUnlockDay = timeinfo.tm_mday;
      Serial.println(">>> 05:55 AM: UNLOCKING PILLBOX NOW.");
    }
  }

  // 2. Check for button press to relock and send data
  if (isWaitingForButton && digitalRead(BUTTON_PIN) == LOW) {
    delay(50); // Debounce
    if (digitalRead(BUTTON_PIN) == LOW) {
      digitalWrite(LOCK_PIN, HIGH); // RELOCK
      isWaitingForButton = false;
      Serial.println(">>> Button Pressed. Relocking and sending event to AWS...");
      sendMedicationEvent();
    }
  }
  
  delay(1000); 
}

void sendMedicationEvent() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(serverUrl);
  
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-Device-Key", DEVICE_KEY);

  // ISO 8601 UTC Timestamp
  struct timeval tv;
  gettimeofday(&tv, NULL);
  struct tm* utc = gmtime(&tv.tv_sec);
  char ts[35];
  snprintf(ts, sizeof(ts), "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
           utc->tm_year + 1900, utc->tm_mon + 1, utc->tm_mday,
           utc->tm_hour, utc->tm_min, utc->tm_sec, (int)(tv.tv_usec / 1000));

  StaticJsonDocument<256> doc;
  doc["deviceId"] = DEVICE_ID;
  doc["compartment"] = 1;
  doc["event"] = "TAKEN";
  doc["timestamp"] = ts;

  String body;
  serializeJson(doc, body);

  int httpCode = http.POST(body);
  
  if (httpCode > 0) {
    Serial.printf("Server Response: %d\n", httpCode);
    Serial.println(http.getString());
  } else {
    Serial.printf("HTTP Post Failed: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();
}