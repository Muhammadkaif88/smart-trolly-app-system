// ======================================================
// SMART TROLLEY FINAL V7 (Add/Remove Toggle & Price Display)
// ESP32 + RFID + LCD + FIREBASE + BUZZER
// ======================================================

#include <Firebase_ESP_Client.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <Preferences.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>

// ======================================================
// RFID PINS (ESP32 38-Pin Default VSPI)
// ======================================================
#define SS_PIN 5
#define RST_PIN 27
MFRC522 rfid(SS_PIN, RST_PIN);

// ======================================================
// LCD PINS
// ======================================================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ======================================================
// BUZZER PIN
// ======================================================
#define BUZZER_PIN 26

// ======================================================
// WIFI
// ======================================================
#define WIFI_SSID "RAIHSOFT2"
#define WIFI_PASSWORD "RaihSoft@230525"

// ======================================================
// FIREBASE
// ======================================================
#define API_KEY "AIzaSyChn1UUDdKP8UNDMNRsApCngJZNoqJdF1s"
#define DATABASE_URL                                                           \
  "https://"                                                                   \
  "smart-trolley-d480f-default-rtdb.asia-southeast1.firebasedatabase.app/"

// ======================================================
// FIREBASE OBJECTS
// ======================================================
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

// ======================================================
// WIFI PREFERENCES
// ======================================================
Preferences preferences;
String currentSSID = "";
String currentPassword = "";
unsigned long lastWiFiCheck = 0;
unsigned long lastResetCheck = 0;

// ======================================================
// RFID UID
// ======================================================
String milkUID = "F3EC6A05";
String biscuitUID = "72116E05";
String soapUID = "EADB6A05";

// ======================================================
// PRODUCT DATA
// ======================================================
int milkPrice = 40;
int biscuitPrice = 20;
int soapPrice = 30;

int milkQty = 0;
int biscuitQty = 0;
int soapQty = 0;

float subtotal = 0;
float gst = 0;
float grandTotal = 0;

unsigned long lastScan = 0;

// ======================================================
// SETUP
// ======================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\nStarting Smart Trolley...");

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SMART TROLLEY");
  delay(1000);

  SPI.begin();
  rfid.PCD_Init();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  Serial.println("Preparing WiFi...");

  preferences.begin("wifi_creds", false);
  currentSSID = preferences.getString("ssid", WIFI_SSID);
  currentPassword = preferences.getString("password", WIFI_PASSWORD);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);

  Serial.print("Connecting to: ");
  Serial.println(currentSSID);

  WiFi.begin(currentSSID.c_str(), currentPassword.c_str());

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");

    if (attempts == 0)
      lcd.setCursor(0, 1);
    lcd.print(".");
    if (attempts > 15) {
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      attempts = 0;
    }
    attempts++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected");
  } else {
    Serial.println("WiFi Failed! Restarting...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed!");
    lcd.setCursor(0, 1);
    lcd.print("Check Creds");
    delay(3000);
    ESP.restart();
  }

  delay(1000);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase SignUp OK");
    signupOK = true;
  } else {
    Serial.printf("Firebase SignUp Error: %s\n",
                  config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting DB...");
  delay(3000);

  if (Firebase.ready() && signupOK) {
    Firebase.RTDB.deleteNode(&fbdo, "/TROLLEY-1/items");
    Firebase.RTDB.setFloat(&fbdo, "/TROLLEY-1/subtotal", 0);
    Firebase.RTDB.setFloat(&fbdo, "/TROLLEY-1/gst", 0);
    Firebase.RTDB.setFloat(&fbdo, "/TROLLEY-1/grandTotal", 0);
    Serial.println("Database Reset OK");
  }

  Serial.println("System Ready");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  successBeep();
  delay(1500);

  showHome();
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) {
    // Check Firebase for reset flag every 3 seconds
    if (millis() - lastResetCheck > 3000) {
      lastResetCheck = millis();
      if (Firebase.ready() && signupOK) {
        if (Firebase.RTDB.getBool(&fbdo, "/TROLLEY-1/reset")) {
          if (fbdo.boolData() == true) {
            Serial.println("Payment Done! Resetting Trolley...");
            milkQty = 0; biscuitQty = 0; soapQty = 0;
            subtotal = 0; gst = 0; grandTotal = 0;
            Firebase.RTDB.setBool(&fbdo, "/TROLLEY-1/reset", false);
            updateFirebase();
            
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Payment Done!");
            lcd.setCursor(0,1);
            lcd.print("Trolley Ready");
            
            successBeep(); delay(300); successBeep();
            delay(2000);
            showHome();
          }
        }
      }
    }

    // Check Firebase for new WiFi credentials every 10 seconds
    if (millis() - lastWiFiCheck > 10000) {
      lastWiFiCheck = millis();
      if (Firebase.ready() && signupOK) {
        if (Firebase.RTDB.getString(&fbdo, "/TROLLEY-1/wifi/ssid")) {
          String newSSID = fbdo.stringData();
          if (Firebase.RTDB.getString(&fbdo, "/TROLLEY-1/wifi/password")) {
            String newPass = fbdo.stringData();
            
            if (newSSID != "" && newSSID != "null" && (newSSID != currentSSID || newPass != currentPassword)) {
              Serial.println("New WiFi credentials found in Firebase!");
              preferences.putString("ssid", newSSID);
              preferences.putString("password", newPass);
              
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("WiFi Updated!");
              lcd.setCursor(0,1);
              lcd.print("Restarting...");
              delay(2000);
              ESP.restart();
            }
          }
        }
      }
    }
    return;
  }
  if (!rfid.PICC_ReadCardSerial())
    return;

  if (millis() - lastScan < 2000) {
    rfid.PICC_HaltA();
    return;
  }
  lastScan = millis();

  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10)
      uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  Serial.print("Scanned UID: ");
  Serial.println(uid);

  // ======================================================
  // MILK LOGIC
  // ======================================================
  if (uid == milkUID) {
    if (milkQty == 0) {
      milkQty = 1; // Add
      successBeep();
      showItemAction("Milk Added", milkPrice);
    } else {
      milkQty = 0; // Remove
      removeBeep();
      showItemAction("Milk Removed", milkPrice);
    }
    updateFirebase();
  }
  // ======================================================
  // BISCUIT LOGIC
  // ======================================================
  else if (uid == biscuitUID) {
    if (biscuitQty == 0) {
      biscuitQty = 1; // Add
      successBeep();
      showItemAction("Biscuit Added", biscuitPrice);
    } else {
      biscuitQty = 0; // Remove
      removeBeep();
      showItemAction("Biscuit Removed", biscuitPrice);
    }
    updateFirebase();
  }
  // ======================================================
  // SOAP LOGIC
  // ======================================================
  else if (uid == soapUID) {
    if (soapQty == 0) {
      soapQty = 1; // Add
      successBeep();
      showItemAction("Soap Added", soapPrice);
    } else {
      soapQty = 0; // Remove
      removeBeep();
      showItemAction("Soap Removed", soapPrice);
    }
    updateFirebase();
  } else {
    errorBeep();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Unknown Card");
    Serial.println("Unknown Card Scanned!");
  }

  delay(2000);
  showHome();
  rfid.PICC_HaltA();
}

// ======================================================
// HELPER FUNCTIONS
// ======================================================
void showItemAction(String actionText, int price) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(actionText);
  lcd.setCursor(0, 1);
  lcd.print("Price: Rs.");
  lcd.print(price);
}

void showHome() {
  subtotal = (milkQty * milkPrice) + (biscuitQty * biscuitPrice) +
             (soapQty * soapPrice);
  gst = subtotal * 0.05;
  grandTotal = subtotal + gst;
  int totalItems = milkQty + biscuitQty + soapQty;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TOTAL: Rs.");
  lcd.print(grandTotal, 1);
  lcd.setCursor(0, 1);
  lcd.print("ITEMS: ");
  lcd.print(totalItems);
}

void successBeep() {
  tone(BUZZER_PIN, 1500);
  delay(150);
  noTone(BUZZER_PIN);
}

void removeBeep() {
  // Two quick beeps to indicate removal
  tone(BUZZER_PIN, 1000);
  delay(100);
  noTone(BUZZER_PIN);
  delay(100);
  tone(BUZZER_PIN, 1000);
  delay(100);
  noTone(BUZZER_PIN);
}

void errorBeep() {
  tone(BUZZER_PIN, 300);
  delay(400);
  noTone(BUZZER_PIN);
}

void updateFirebase() {
  // Recalculate totals before pushing to Firebase!
  subtotal = (milkQty * milkPrice) + (biscuitQty * biscuitPrice) +
             (soapQty * soapPrice);
  gst = subtotal * 0.05;
  grandTotal = subtotal + gst;

  if (Firebase.ready() && signupOK) {
    if (milkQty > 0) {
      Firebase.RTDB.setInt(&fbdo, "/TROLLEY-1/items/Milk/price", milkPrice);
      Firebase.RTDB.setInt(&fbdo, "/TROLLEY-1/items/Milk/qty", milkQty);
      Firebase.RTDB.setInt(&fbdo, "/TROLLEY-1/items/Milk/total",
                           milkQty * milkPrice);
    } else
      Firebase.RTDB.deleteNode(&fbdo, "/TROLLEY-1/items/Milk");

    if (biscuitQty > 0) {
      Firebase.RTDB.setInt(&fbdo, "/TROLLEY-1/items/Biscuit/price",
                           biscuitPrice);
      Firebase.RTDB.setInt(&fbdo, "/TROLLEY-1/items/Biscuit/qty", biscuitQty);
      Firebase.RTDB.setInt(&fbdo, "/TROLLEY-1/items/Biscuit/total",
                           biscuitQty * biscuitPrice);
    } else
      Firebase.RTDB.deleteNode(&fbdo, "/TROLLEY-1/items/Biscuit");

    if (soapQty > 0) {
      Firebase.RTDB.setInt(&fbdo, "/TROLLEY-1/items/Soap/price", soapPrice);
      Firebase.RTDB.setInt(&fbdo, "/TROLLEY-1/items/Soap/qty", soapQty);
      Firebase.RTDB.setInt(&fbdo, "/TROLLEY-1/items/Soap/total",
                           soapQty * soapPrice);
    } else
      Firebase.RTDB.deleteNode(&fbdo, "/TROLLEY-1/items/Soap");

    Firebase.RTDB.setFloat(&fbdo, "/TROLLEY-1/subtotal", subtotal);
    Firebase.RTDB.setFloat(&fbdo, "/TROLLEY-1/gst", gst);
    Firebase.RTDB.setFloat(&fbdo, "/TROLLEY-1/grandTotal", grandTotal);

    Serial.println("Firebase Updated Successfully");
  } else {
    Serial.println("Firebase Not Ready");
  }
}
