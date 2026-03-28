#include <HardwareSerial.h>
#include <TinyGPS++.h>

#define SIM800_RX  26
#define SIM800_TX  27
#define SIM800_RST 33
#define GPS_RX     16
#define GPS_TX     17

HardwareSerial sim800(1);
HardwareSerial gpsSerial(2);
TinyGPSPlus    gps;

const char* APN        = "internet.a1.bg";
const char* DEVICE_ID  = "bracelet-001";
const char* SERVER_URL = "http://ec2-16-16-142-88.eu-north-1.compute.amazonaws.com:8080/location";
const int   SEND_EVERY = 15000;

unsigned long lastSend = 0;

void safeDelay(uint32_t ms) {
  uint32_t start = millis();
  while (millis() - start < ms) {
    while (gpsSerial.available()) gps.encode(gpsSerial.read());
    delay(10);
  }
}

String readSIM(int waitMs) {
  safeDelay(waitMs);
  String resp = "";
  while (sim800.available()) resp += (char)sim800.read();
  return resp;
}

String readSIMSmart(int timeoutMs) {
  String resp = "";
  unsigned long start   = millis();
  unsigned long lastChar = millis();
  while (millis() - start < (unsigned long)timeoutMs) {
    while (gpsSerial.available()) gps.encode(gpsSerial.read());
    while (sim800.available()) {
      resp += (char)sim800.read();
      lastChar = millis();
    }
    if (resp.length() > 0 && millis() - lastChar > 600) break;
    delay(10);
  }
  return resp;
}

void sendAndPrint(String cmd, int delayTime = 1000) {
  Serial.println("\nTesting: " + cmd);
  sim800.println(cmd);
  safeDelay(delayTime);
  while (sim800.available()) Serial.write(sim800.read());
}

bool openBearer() {
  String r;

  sim800.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  r = readSIM(2000);
  Serial.println("[GPRS] Contype: " + r);

  sim800.println("AT+SAPBR=3,1,\"APN\",\"" + String(APN) + "\"");
  r = readSIM(2000);
  Serial.println("[GPRS] APN: " + r);

  sim800.println("AT+SAPBR=1,1");
  r = readSIM(8000);
  Serial.println("[GPRS] Open: " + r);

  sim800.println("AT+SAPBR=2,1");
  r = readSIM(2000);
  Serial.println("[GPRS] IP: " + r);

  if (r.indexOf("0.0.0.0") >= 0 || r.indexOf("1,1,\"") < 0) {
    Serial.println("[GPRS] No valid IP — bearer failed.");
    return false;
  }
  Serial.println("[GPRS] Bearer OK!");
  return true;
}

String getTimestamp() {
  if (gps.date.isValid() && gps.time.isValid() && gps.date.year() > 2020) {
    char buf[30];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d.000Z",
      gps.date.year(), gps.date.month(), gps.date.day(),
      gps.time.hour(), gps.time.minute(), gps.time.second());
    return String(buf);
  }

  sim800.println("AT+CCLK?");
  String r = readSIM(2000);
  int start = r.indexOf("\"");
  int end   = r.indexOf("\"", start + 1);
  if (start >= 0 && end > start) {
    String t  = r.substring(start + 1, end);
    String yy = t.substring(0, 2);
    String mm = t.substring(3, 5);
    String dd = t.substring(6, 8);
    String hh = t.substring(9, 11);
    String mi = t.substring(12, 14);
    String ss = t.substring(15, 17);
    if (yy.toInt() >= 24) {
      return "20" + yy + "-" + mm + "-" + dd + "T" + hh + ":" + mi + ":" + ss + ".000Z";
    }
  }


  return "2026-03-28T00:00:00.000Z";
}

void postLocation() {
  while (gpsSerial.available()) gps.encode(gpsSerial.read());

  float lat      = 42.698334;
  float lng      = 23.319941;
  float accuracy = 99.9;

  if (gps.location.isValid() && gps.location.age() < 5000) {
    lat      = gps.location.lat();
    lng      = gps.location.lng();
    accuracy = gps.hdop.isValid() ? gps.hdop.hdop() * 5.0f : 9.9f;
    Serial.printf("[GPS] Fix: %.6f, %.6f  HDOP: %.1f  Sats: %d\n",
      lat, lng, gps.hdop.hdop(), gps.satellites.value());
  } else {
    Serial.println("[GPS] No fix — using fallback coordinates.");
    Serial.printf("[GPS] Chars: %lu  Sentences: %lu  Fails: %lu\n",
      gps.charsProcessed(), gps.sentencesWithFix(), gps.failedChecksum());
  }

  String timestamp = getTimestamp();
  Serial.println("[TIME] " + timestamp);

  String payload = "{";
  payload += "\"deviceId\":\"" + String(DEVICE_ID) + "\",";
  payload += "\"lat\":"        + String(lat, 6)    + ",";
  payload += "\"lng\":"        + String(lng, 6)    + ",";
  payload += "\"timestamp\":\"" + timestamp        + "\",";
  payload += "\"accuracy\":"   + String(accuracy, 1);
  payload += "}";

  Serial.println("\n[POST] Payload: " + payload);

  String r;

  sim800.println("AT+HTTPTERM");
  readSIM(1000);

  sim800.println("AT+HTTPINIT");
  r = readSIMSmart(5000);
  Serial.println("[POST] HTTPINIT: " + r);
  if (r.indexOf("OK") < 0) {
    Serial.println("[POST] HTTPINIT failed — reopening bearer...");
    sim800.println("AT+SAPBR=0,1");
    readSIM(3000);
    if (!openBearer()) {
      Serial.println("[POST] Bearer reopen failed.");
      return;
    }
    sim800.println("AT+HTTPINIT");
    r = readSIMSmart(5000);
    if (r.indexOf("OK") < 0) {
      Serial.println("[POST] HTTPINIT retry failed.");
      return;
    }
  }

  sim800.println("AT+HTTPPARA=\"CID\",1");
  r = readSIMSmart(3000);
  Serial.println("[POST] CID: " + r);

  sim800.println("AT+HTTPPARA=\"URL\",\"" + String(SERVER_URL) + "\"");
  r = readSIMSmart(3000);
  Serial.println("[POST] URL: " + r);

  sim800.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
  r = readSIMSmart(3000);
  Serial.println("[POST] CONTENT: " + r);

  sim800.println("AT+HTTPPARA=\"USERDATA\",\"X-Device-Key: dev-key-bracelet-001-secret\"");
  r = readSIMSmart(3000);
  Serial.println("[POST] KEY: " + r);

  sim800.println("AT+HTTPDATA=" + String(payload.length()) + ",10000");
  r = readSIMSmart(8000);
  Serial.println("[POST] HTTPDATA: " + r);

  if (r.indexOf("DOWNLOAD") < 0) {
    Serial.println("[POST] No DOWNLOAD prompt.");
    sim800.println("AT+HTTPTERM");
    readSIM(1000);
    return;
  }

  sim800.print(payload);
  r = readSIMSmart(5000);
  Serial.println("[POST] After payload: " + r);

  sim800.println("AT+HTTPACTION=1");
  r = readSIMSmart(20000); // up to 20s for server response
  Serial.println("[POST] ACTION: " + r);

  sim800.println("AT+HTTPREAD");
  r = readSIMSmart(8000);
  Serial.println("[POST] RESPONSE: " + r);

  if (r.indexOf("success") >= 0 || r.indexOf("200") >= 0) {
    Serial.println("[POST] ✓ SUCCESS");
  } else {
    Serial.println("[POST] ✗ Unexpected response.");
  }

  sim800.println("AT+HTTPTERM");
  readSIM(1000);
}

void setup() {
  Serial.begin(115200);

  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.println("[GPS] UART started on GPIO16/17");

  sim800.begin(9600, SERIAL_8N1, SIM800_RX, SIM800_TX);

  pinMode(SIM800_RST, OUTPUT);
  digitalWrite(SIM800_RST, LOW); delay(100);
  digitalWrite(SIM800_RST, HIGH);

  Serial.println("Waiting for SIM800L boot (20s)...");
  safeDelay(20000);

  Serial.println("Polling...");
  unsigned long pollStart = millis();
  bool ready = false;
  while (millis() - pollStart < 30000) {
    sim800.println("AT");
    safeDelay(3000);
    String r = "";
    while (sim800.available()) r += (char)sim800.read();
    Serial.println("Poll: [" + r + "]");
    if (r.indexOf("OK")    >= 0 || r.indexOf("RDY")  >= 0 ||
        r.indexOf("READY") >= 0 || r.indexOf("Call") >= 0) {
      ready = true;
      Serial.println(">>> SIM800L ready!");
      safeDelay(15000);
      break;
    }
  }

  if (!ready) Serial.println("WARNING: No response — proceeding anyway.");

  Serial.println("--- DIAGNOSTIC ---");
  sendAndPrint("AT");
  sendAndPrint("AT+CSQ");
  sendAndPrint("AT+CPIN?");
  sendAndPrint("AT+CREG?");
  Serial.println("--- DIAGNOSTIC END ---");

  Serial.println("Opening bearer...");
  int attempts = 0;
  while (!openBearer() && attempts < 3) {
    Serial.println("Retrying in 5s...");
    safeDelay(5000);
    attempts++;
  }
  if (attempts == 3) {
    Serial.println("FATAL: Bearer failed.");
    while (true) { delay(1000); }
  }

  Serial.println("=== Ready — GPS parsing active ===");
  lastSend = millis() - SEND_EVERY;
}

void loop() {
  while (gpsSerial.available()) gps.encode(gpsSerial.read());
  if (sim800.available()) Serial.write(sim800.read());
  if (Serial.available())  sim800.write(Serial.read());

  if (millis() - lastSend >= SEND_EVERY) {
    lastSend = millis();
    postLocation();
  }
}