#include <HardwareSerial.h>

#define SIM800_RX 26
#define SIM800_TX 27
#define SIM800_RST 33

HardwareSerial sim800(1);
const char* APN = "internet.a1.bg";

void sendAndPrint(String cmd, int delayTime = 1000) {
  Serial.println("\nTesting: " + cmd);
  sim800.println(cmd);
  delay(delayTime);
  while (sim800.available()) {
    Serial.write(sim800.read());
  }
}

void setup() {
  Serial.begin(115200);
  sim800.begin(9600, SERIAL_8N1, SIM800_RX, SIM800_TX);
  
  // Hardware Reset
  pinMode(SIM800_RST, OUTPUT);
  digitalWrite(SIM800_RST, LOW); delay(100);
  digitalWrite(SIM800_RST, HIGH);
  delay(35000); 

  Serial.println("--- SIM800L DIAGNOSTIC START ---");

  // HOP 1: ESP to SIM (Serial Link)
  sendAndPrint("AT"); 

  // HOP 2: SIM to A1 (Cellular Link)
  sendAndPrint("AT+CSQ");       // Signal quality (should be > 10)
  sendAndPrint("AT+CPIN?");    // SIM Status (should be READY)
  sendAndPrint("AT+CREG?");    // Network Reg (should be 0,1 or 0,5)

  // HOP 3: SIM to Internet/Backend (GPRS Link)
  sendAndPrint("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  sendAndPrint("AT+SAPBR=3,1,\"APN\",\"" + String(APN) + "\"");
  sendAndPrint("AT+SAPBR=1,1", 4000); // Open Bearer
  sendAndPrint("AT+SAPBR=2,1");       // Check IP Address (The "Ping" to Backend)

  Serial.println("\n--- DIAGNOSTIC END ---");
  Serial.println("You can now type manual AT commands in the Serial Monitor.");

  sim800.println("AT+CMGF=1"); 
delay(1000);

sim800.println("AT+CMGS=\"+359882163123\"");
delay(1000);

sim800.print("Hello from SIM800L");
delay(500);

sim800.write(26); 

}

void loop() {
  // Serial Passthrough for manual debugging
  if (sim800.available()) {
    Serial.write(sim800.read());
  }
  if (Serial.available()) {
    sim800.write(Serial.read());
  }
}