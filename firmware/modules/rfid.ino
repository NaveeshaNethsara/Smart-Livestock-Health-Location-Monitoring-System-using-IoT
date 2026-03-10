#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 5
#define RST_PIN 4

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);

  SPI.begin();
  rfid.PCD_Init();

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("Scan RFID Card");
}

void loop() {

  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  String uid = "";

  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i], HEX);
  }

  Serial.println(uid);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Card UID:");
  lcd.setCursor(0,1);
  lcd.print(uid);

  delay(2000);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Scan RFID Card");

  rfid.PICC_HaltA();
}