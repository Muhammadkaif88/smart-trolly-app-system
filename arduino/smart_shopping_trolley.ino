// ======================================================
// SMART TROLLEY FINAL V7 (Add/Remove Toggle & Price Display)
// ESP32 + RFID + LCD + FIREBASE + BUZZER
// ======================================================

#include <DNSServer.h>
#include <Firebase_ESP_Client.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <Preferences.h>
#include <SPI.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>


// ======================================================
// FORWARD DECLARATIONS
// ======================================================
void startAPMode();
void handleRoot();
void handleSave();
void handleNotFound();
void checkWiFiConnection();
void updateFirebase();
void showHome();
void successBeep();
void removeBeep();
void errorBeep();
void showItemAction(String actionText, int price);

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
// WIFI PREFERENCES & STATE
// ======================================================
Preferences preferences;
String currentSSID = "";
String currentPassword = "";
unsigned long lastWiFiCheck = 0;
unsigned long lastResetCheck = 0;

const byte DNS_PORT = 53;
DNSServer dnsServer;
WebServer server(80);
bool isOnline = false;
bool isAPMode = false;
bool wasConnected = false;

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
// AP & WIFI CONFIG PORTAL FUNCTIONS
// ======================================================
void startAPMode() {
  WiFi.softAP("Smart_Trolley_WiFi");
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP Address: ");
  Serial.println(IP);

  dnsServer.start(DNS_PORT, "*", IP);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound(handleNotFound);

  server.begin();
  isAPMode = true;
  Serial.println("AP Configuration Web Server Started.");
}

void handleRoot() {
  String html =
      "<!DOCTYPE html><html>"
      "<head><meta name=\"viewport\" content=\"width=device-width, "
      "initial-scale=1\">"
      "<link "
      "href=\"https://fonts.googleapis.com/"
      "css2?family=Poppins:wght@400;600&display=swap\" rel=\"stylesheet\">"
      "<style>"
      "body { font-family: 'Poppins', sans-serif; background-color: #09090F; "
      "color: #FFFFFF; text-align: center; margin: 0; padding: 20px; }"
      ".container { max-width: 400px; margin: 40px auto; background: rgba(255, "
      "255, 255, 0.05); padding: 30px; border-radius: 20px; border: 1px solid "
      "rgba(255, 255, 255, 0.1); box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.3); "
      "backdrop-filter: blur(4px); }"
      "h2 { color: #00E676; margin-bottom: 30px; font-weight: 600; "
      "letter-spacing: 1px; }"
      "input[type=text], input[type=password] { width: 100%; padding: 12px "
      "20px; margin: 8px 0 20px 0; display: inline-block; border: 1px solid "
      "rgba(255, 255, 255, 0.2); border-radius: 12px; box-sizing: border-box; "
      "background: rgba(255, 255, 255, 0.1); color: white; font-size: 16px; }"
      "input[type=text]:focus, input[type=password]:focus { border-color: "
      "#00E676; outline: none; }"
      "button { background-color: #00E676; color: #000000; padding: 14px 20px; "
      "margin: 8px 0; border: none; border-radius: 12px; cursor: pointer; "
      "width: 100%; font-size: 16px; font-weight: 600; letter-spacing: 1px; "
      "transition: all 0.3s ease; }"
      "button:hover { background-color: #00B359; transform: translateY(-2px); }"
      ".footer { margin-top: 30px; font-size: 12px; color: rgba(255, 255, 255, "
      "0.5); }"
      "</style></head>"
      "<body>"
      "<div class=\"container\">"
      "<h2>SMART TROLLEY</h2>"
      "<p style=\"color: rgba(255, 255, 255, 0.7); margin-bottom: 25px;\">WiFi "
      "Configuration Portal</p>"
      "<form action=\"/save\" method=\"post\">"
      "<label style=\"display:block; text-align:left; font-size: 14px; color: "
      "rgba(255, 255, 255, 0.7);\">WiFi Network Name (SSID)</label>"
      "<input type=\"text\" name=\"ssid\" placeholder=\"Enter SSID\" required>"
      "<label style=\"display:block; text-align:left; font-size: 14px; color: "
      "rgba(255, 255, 255, 0.7);\">WiFi Password</label>"
      "<input type=\"password\" name=\"password\" placeholder=\"Enter "
      "Password\">"
      "<button type=\"submit\">CONNECT DEVICE</button>"
      "</form>"
      "<div class=\"footer\">Smart Trolley App System &copy; 2026</div>"
      "</div>"
      "</body></html>";
  server.send(200, "text/html", html);
}

void handleSave() {
  if (server.hasArg("ssid")) {
    String newSSID = server.arg("ssid");
    String newPass = server.hasArg("password") ? server.arg("password") : "";

    preferences.putString("ssid", newSSID);
    preferences.putString("password", newPass);

    String responseHtml =
        "<!DOCTYPE html><html>"
        "<head><meta name=\"viewport\" content=\"width=device-width, "
        "initial-scale=1\">"
        "<link "
        "href=\"https://fonts.googleapis.com/"
        "css2?family=Poppins:wght@400;600&display=swap\" rel=\"stylesheet\">"
        "<style>"
        "body { font-family: 'Poppins', sans-serif; background-color: #09090F; "
        "color: #FFFFFF; text-align: center; margin: 0; padding: 20px; }"
        ".container { max-width: 400px; margin: 40px auto; background: "
        "rgba(255, 255, 255, 0.05); padding: 30px; border-radius: 20px; "
        "border: 1px solid rgba(255, 255, 255, 0.1); box-shadow: 0 8px 32px 0 "
        "rgba(0, 0, 0, 0.3); backdrop-filter: blur(4px); }"
        "h2 { color: #00E676; margin-bottom: 20px; }"
        "p { color: rgba(255, 255, 255, 0.8); line-height: 1.6; }"
        "</style></head>"
        "<body>"
        "<div class=\"container\">"
        "<h2>Credentials Saved!</h2>"
        "<p>The Smart Trolley will now restart and attempt to connect to: "
        "<br><strong>" +
        newSSID +
        "</strong></p>"
        "<p>You can close this page now.</p>"
        "</div>"
        "</body></html>";

    server.send(200, "text/html", responseHtml);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Configured!");
    lcd.setCursor(0, 1);
    lcd.print("Restarting...");

    delay(2000);
    ESP.restart();
  } else {
    server.send(400, "text/plain", "SSID is required");
  }
}

void handleNotFound() {
  String host = server.hostHeader();
  if (host != WiFi.softAPIP().toString()) {
    server.sendHeader(
        "Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
    server.send(302, "text/plain", "");
  } else {
    server.send(404, "text/plain", "Not Found");
  }
}

void checkWiFiConnection() {
  bool currentConnected = (WiFi.status() == WL_CONNECTED);

  if (currentConnected && !wasConnected) {
    Serial.println("\nWiFi Connected (or reconnected)!");
    isOnline = true;

    if (isAPMode) {
      dnsServer.stop();
      server.stop();
      WiFi.softAPdisconnect(true);
      isAPMode = false;
      Serial.println("AP Mode Stopped");
    }

    WiFi.mode(WIFI_STA);

    if (!signupOK) {
      Serial.println("Attempting Firebase Sign Up...");
      if (Firebase.signUp(&config, &auth, "", "")) {
        Serial.println("Firebase SignUp OK");
        signupOK = true;
      } else {
        Serial.printf("Firebase SignUp Error: %s\n",
                      config.signer.signupError.message.c_str());
      }
    }

    if (signupOK) {
      Firebase.begin(&config, &auth);
      Firebase.reconnectWiFi(true);
      updateFirebase();
      Serial.println("Firebase Synced after reconnect");
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected");
    lcd.setCursor(0, 1);
    lcd.print("Firebase Ready");
    delay(2000);
    showHome();
  } else if (!currentConnected && wasConnected) {
    Serial.println("\nWiFi Connection Lost!");
    isOnline = false;

    WiFi.mode(WIFI_AP_STA);

    if (!isAPMode) {
      startAPMode();
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Offline");
    lcd.setCursor(0, 1);
    lcd.print("Running Local");
    delay(2000);
    showHome();
  }

  wasConnected = currentConnected;
}

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

  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect(true);
  delay(100);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);

  Serial.print("Connecting to: ");
  Serial.println(currentSSID);

  WiFi.begin(currentSSID.c_str(), currentPassword.c_str());

  int attempts = 0;
  // Try to connect to WiFi for maximum 8 seconds (16 attempts) or until any
  // RFID card is scanned to skip
  while (WiFi.status() != WL_CONNECTED && attempts < 16) {
    delay(500);
    Serial.print(".");

    // Check if any card is tapped on the RFID reader to skip WiFi connection
    // and go offline instantly
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      Serial.println("\nRFID Card tapped! Skipping WiFi connection...");
      successBeep();
      rfid.PICC_HaltA(); // halt the card so it can be scanned again in loop
      break;
    }

    if (attempts == 0)
      lcd.setCursor(0, 1);
    lcd.print(".");
    if (attempts > 12) {
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
    isOnline = true;
    wasConnected = true;
    WiFi.mode(WIFI_STA); // Disable AP since we connected successfully
  } else {
    Serial.println("WiFi Connection Skipped/Failed. Starting Offline Mode...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Offline Mode");
    lcd.setCursor(0, 1);
    lcd.print("AP: Trolley_WiFi");
    isOnline = false;
    wasConnected = false;
    startAPMode();
    delay(3000);
  }

  delay(1000);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (isOnline) {
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
  } else {
    Serial.println("Firebase Initialization Skipped (Offline Mode)");
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
  // Handle background WiFi tasks
  checkWiFiConnection();
  if (isAPMode) {
    dnsServer.processNextRequest();
    server.handleClient();
  }

  if (!rfid.PICC_IsNewCardPresent()) {
    // Check Firebase for reset flag every 3 seconds
    if (millis() - lastResetCheck > 3000) {
      lastResetCheck = millis();
      if (Firebase.ready() && signupOK) {
        if (Firebase.RTDB.getBool(&fbdo, "/TROLLEY-1/reset")) {
          if (fbdo.boolData() == true) {
            Serial.println("Payment Done! Resetting Trolley...");
            milkQty = 0;
            biscuitQty = 0;
            soapQty = 0;
            subtotal = 0;
            gst = 0;
            grandTotal = 0;
            Firebase.RTDB.setBool(&fbdo, "/TROLLEY-1/reset", false);
            updateFirebase();

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Payment Done!");
            lcd.setCursor(0, 1);
            lcd.print("Trolley Ready");

            successBeep();
            delay(300);
            successBeep();
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

            if (newSSID != "" && newSSID != "null" &&
                (newSSID != currentSSID || newPass != currentPassword)) {
              Serial.println("New WiFi credentials found in Firebase!");
              preferences.putString("ssid", newSSID);
              preferences.putString("password", newPass);

              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("WiFi Updated!");
              lcd.setCursor(0, 1);
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

  // Show online/offline status on the bottom right corner
  lcd.setCursor(11, 1);
  if (isOnline) {
    lcd.print("[ ON]");
  } else {
    lcd.print("[OFF]");
  }
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

  if (isOnline && Firebase.ready() && signupOK) {
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
