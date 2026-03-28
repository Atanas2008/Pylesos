#include <HardwareSerial.h>

#define SIM800_RX  26
#define SIM800_TX  27
#define SIM800_RST 33

HardwareSerial sim800(1);

const char* APN        = "internet.a1.bg";
const char* DEVICE_ID  = "bracelet-001";
const char* SERVER_URL = "http://ec2-16-16-142-88.eu-north-1.compute.amazonaws.com:8080/location";
const int   SEND_EVERY = 15000;

unsigned long lastSend = 0;

// ==================================================
String readSIM(int waitMs) {
  delay(waitMs);
  String resp = "";
  while (sim800.available()) resp += (char)sim800.read();
  return resp;
}

// ==================================================
void sendAndPrint(String cmd, int delayTime = 1000) {
  Serial.println("\nTesting: " + cmd);
  sim800.println(cmd);
  delay(delayTime);
  while (sim800.available()) Serial.write(sim800.read());
}

// ==================================================
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
  sim800.println("AT+CCLK?");
  String r = readSIM(2000);
  Serial.println("[TIME] CCLK response: " + r);

  // Response format: +CCLK: "26/03/27,19:52:33+08"
  int start = r.indexOf("\"");
  int end = r.indexOf("\"", start + 1);

  if (start >= 0 && end > start) {
    String time = r.substring(start + 1, end);
    // Parse: YY/MM/DD,HH:MM:SS+TZ
    // Convert to ISO8601: YYYY-MM-DDTHH:MM:SS.000Z

    String yy = time.substring(0, 2);
    String mm = time.substring(3, 5);
    String dd = time.substring(6, 8);
    String hh = time.substring(9, 11);
    String mi = time.substring(12, 14);
    String ss = time.substring(15, 17);

    return "20" + yy + "-" + mm + "-" + dd + "T" + hh + ":" + mi + ":" + ss + ".000Z";
  }

  return "2026-03-27T19:52:33.000Z"; // Fallback
}

// ==================================================
void postLocation() {
  float  lat      = 42.698334;
  float  lng      = 23.319941;
  float  accuracy = 5.0;
  String timestamp = getTimestamp();

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
  r = readSIM(3000);
  Serial.println("[POST] HTTPINIT: " + r);
  if (r.indexOf("OK") < 0) {
    Serial.println("[POST] HTTPINIT failed — reopening bearer...");
    sim800.println("AT+SAPBR=0,1");
    readSIM(3000);
    if (!openBearer()) {
      Serial.println("[POST] Bearer reopen failed — giving up.");
      return;
    }
    sim800.println("AT+HTTPINIT");
    r = readSIM(3000);
    Serial.println("[POST] HTTPINIT retry: " + r);
    if (r.indexOf("OK") < 0) {
      Serial.println("[POST] HTTPINIT retry failed.");
      return;
    }
  }

  sim800.println("AT+HTTPPARA=\"CID\",1");
  r = readSIM(2000);
  Serial.println("[POST] CID: " + r);

  sim800.println("AT+HTTPPARA=\"URL\",\"" + String(SERVER_URL) + "\"");
  r = readSIM(2000);
  Serial.println("[POST] URL: " + r);

  sim800.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
  r = readSIM(2000);
  Serial.println("[POST] CONTENT: " + r);

  sim800.println("AT+HTTPDATA=" + String(payload.length()) + ",10000");
  r = readSIM(5000);
  Serial.println("[POST] HTTPDATA: " + r);

  if (r.indexOf("DOWNLOAD") < 0) {
    Serial.println("[POST] No DOWNLOAD prompt — aborting.");
    sim800.println("AT+HTTPTERM");
    readSIM(1000);
    return;
  }

  sim800.print(payload);
  r = readSIM(3000);
  Serial.println("[POST] After payload: " + r);

  sim800.println("AT+HTTPACTION=1");
  r = readSIM(15000);
  Serial.println("[POST] ACTION: " + r);

  sim800.println("AT+HTTPREAD");
  r = readSIM(5000);
  Serial.println("[POST] RESPONSE: " + r);

  if (r.indexOf("success") >= 0 || r.indexOf("200") >= 0) {
    Serial.println("[POST] ✓ SUCCESS");
  } else {
    Serial.println("[POST] ✗ Unexpected response.");
  }

  sim800.println("AT+HTTPTERM");
  readSIM(1000);
}

// ==================================================
void setup() {
  Serial.begin(115200);
  sim800.begin(9600, SERIAL_8N1, SIM800_RX, SIM800_TX);

  pinMode(SIM800_RST, OUTPUT);
  digitalWrite(SIM800_RST, LOW); delay(100);
  digitalWrite(SIM800_RST, HIGH);

  // Wait minimum boot time then poll for response
  Serial.println("Waiting for SIM800L boot (20s minimum)...");
  delay(20000);

  Serial.println("Polling for response...");
  unsigned long pollStart = millis();
  bool ready = false;
  while (millis() - pollStart < 30000) {
    sim800.println("AT");
    delay(1000);
    String r = "";
    while (sim800.available()) r += (char)sim800.read();
    Serial.println("Poll: [" + r + "]");
    if (r.indexOf("OK")    >= 0 || r.indexOf("RDY")   >= 0 ||
        r.indexOf("READY") >= 0 || r.indexOf("Call")  >= 0) {
      ready = true;
      Serial.println(">>> SIM800L responded!");
      break;
    }
  }

  if (!ready) {
    Serial.println("WARNING: No response after 50s — proceeding anyway.");
  }
  delay(3000); // settle

  Serial.println("--- SIM800L DIAGNOSTIC START ---");
  sendAndPrint("AT");
  sendAndPrint("AT+CSQ");
  sendAndPrint("AT+CPIN?");
  sendAndPrint("AT+CREG?");
  Serial.println("\n--- DIAGNOSTIC END ---");

  Serial.println("Opening GPRS bearer...");
  if (!openBearer()) {
    Serial.println("FATAL: Could not open bearer. Check SIM/signal.");
    while (true);
  }

  Serial.println("Ready. Starting transmissions...");
  lastSend = millis() - SEND_EVERY;
}

// ==================================================
void loop() {
  if (sim800.available()) Serial.write(sim800.read());
  if (Serial.available())  sim800.write(Serial.read());

  if (millis() - lastSend >= SEND_EVERY) {
    lastSend = millis();
    postLocation();
  }
}