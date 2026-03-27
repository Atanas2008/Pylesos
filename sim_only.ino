#include <HardwareSerial.h>

#define SIM800_RX  26
#define SIM800_TX  27
#define SIM800_RST 33

HardwareSerial sim800(1);

const char* APN       = "internet.a1.bg";
const char* DEVICE_ID = "THE_ONE";
const char* SERVER_URL = "http://ec2-16-16-142-88.eu-north-1.compute.amazonaws.com:8080/location";
const int   SEND_EVERY = 15000;

unsigned long lastSend = 0;

// ==================================================
String atCmd(String cmd, int waitMs) {
  sim800.println(cmd);
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
  while (sim800.available()) {
    Serial.write(sim800.read());
  }
}

// ==================================================
void postLocation() {
  float lat      = 42.698334;
  float lng      = 23.319941;
  float accuracy = 5.0;
  String timestamp = "2026-03-27T19:52:33.000Z";

  String payload = "{";
  payload += "\"deviceId\":\"" + String(DEVICE_ID) + "\",";
  payload += "\"lat\":"        + String(lat, 6)    + ",";
  payload += "\"lng\":"        + String(lng, 6)    + ",";
  payload += "\"timestamp\":\"" + timestamp        + "\",";
  payload += "\"accuracy\":"   + String(accuracy, 1);
  payload += "}";

  Serial.println("\n[POST] Payload: " + payload);

  atCmd("AT+HTTPTERM", 500);
  Serial.println("[POST] HTTPINIT: "  + atCmd("AT+HTTPINIT", 1000));
  Serial.println("[POST] CID: "       + atCmd("AT+HTTPPARA=\"CID\",1", 1000));
  Serial.println("[POST] URL: "       + atCmd("AT+HTTPPARA=\"URL\",\"" + String(SERVER_URL) + "\"", 1000));
  Serial.println("[POST] CONTENT: "   + atCmd("AT+HTTPPARA=\"CONTENT\",\"application/json\"", 1000));

  String dataCmd = "AT+HTTPDATA=" + String(payload.length()) + ",5000";
  String r = atCmd(dataCmd, 2000);
  Serial.println("[POST] HTTPDATA: " + r);

  if (r.indexOf("DOWNLOAD") >= 0) {
    sim800.print(payload);
    delay(1500);

    r = atCmd("AT+HTTPACTION=1", 10000);
    Serial.println("[POST] ACTION: " + r);

    r = atCmd("AT+HTTPREAD", 3000);
    Serial.println("[POST] RESPONSE: " + r);

    if (r.indexOf("success") >= 0 || r.indexOf("200") >= 0) {
      Serial.println("[POST] SUCCESS");
    } else {
      Serial.println("[POST] Unexpected response.");
    }
  } else {
    Serial.println("[POST] DOWNLOAD prompt missing — reopening bearer...");
    atCmd("AT+SAPBR=0,1", 2000);
    atCmd("AT+SAPBR=1,1", 4000);
  }

  atCmd("AT+HTTPTERM", 500);
}

// ==================================================
void setup() {
  Serial.begin(115200);
  sim800.begin(9600, SERIAL_8N1, SIM800_RX, SIM800_TX);

  pinMode(SIM800_RST, OUTPUT);
  digitalWrite(SIM800_RST, LOW); delay(100);
  digitalWrite(SIM800_RST, HIGH);
  delay(35000);

  Serial.println("--- SIM800L DIAGNOSTIC START ---");
  sendAndPrint("AT");
  sendAndPrint("AT+CSQ");
  sendAndPrint("AT+CPIN?");
  sendAndPrint("AT+CREG?");
  sendAndPrint("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  sendAndPrint("AT+SAPBR=3,1,\"APN\",\"" + String(APN) + "\"");
  sendAndPrint("AT+SAPBR=1,1", 4000);
  sendAndPrint("AT+SAPBR=2,1");
  Serial.println("\n--- DIAGNOSTIC END ---");
  Serial.println("You can now type manual AT commands in the Serial Monitor.");

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