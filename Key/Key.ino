#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Standardwerte für WLAN und MQTT
char ssid[50] = "Setup_Hotspot";  // SSID des Access Points
char password[50] = "SetupPasswort";  // Passwort des Access Points

char mqtt_broker[50] = "192.168.1.100";  // Standard MQTT-Broker
char mqtt_username[50] = "mqtt";  // Standard MQTT Benutzername
char mqtt_password[50] = "mqtt";  // Standard MQTT Passwort
char mqtt_topic_status[50] = "esp32/schluesselschalter/status";  // Status Topic
char mqtt_topic_lastupdate[50] = "esp32/schluesselschalter/lastupdate";  // Last Update Topic
char mqtt_topic_wificonnect[50] = "esp32/schluesselschalter/wificonnect";  // WiFi Connect Topic

// Objekte für WiFi, WebServer und MQTT
WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);

// Speicher für Einstellungen
Preferences preferences;

// NTP-Client für Zeit
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);  // Zeit vom NTP-Server holen

// HTML-Seite für das Captive Portal
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>WLAN und MQTT Setup</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 50px; }
    input { padding: 10px; margin: 10px; width: 80%; }
    button { padding: 10px 20px; background-color: #4CAF50; color: white; border: none; cursor: pointer; }
    button:hover { background-color: #45a049; }
  </style>
</head>
<body>
  <h1>WLAN und MQTT Setup</h1>
  <form action="/set_wifi" method="POST">
    <h2>WLAN Einstellungen</h2>
    <input type="text" name="ssid" placeholder="WLAN SSID" required><br>
    <input type="password" name="wifipassword" placeholder="WLAN Passwort" required><br>
    <h2>MQTT Einstellungen</h2>
    <input type="text" name="mqtt_broker" placeholder="MQTT Broker IP" required><br>
    <input type="text" name="mqtt_username" placeholder="MQTT Benutzername" required><br>
    <input type="password" name="mqtt_password" placeholder="MQTT Passwort" required><br>
    <button type="submit">Speichern</button>
  </form>
</body>
</html>
)rawliteral";

// Setup des Captive Portals
void setupCaptivePortal() {
  // Wenn die Seite "/" aufgerufen wird, wird das HTML-Formular angezeigt
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });

  // Wenn das Formular abgeschickt wird, werden die Daten verarbeitet
  server.on("/set_wifi", HTTP_POST, []() {
    // WLAN-Einstellungen aus dem Formular lesen
    String ssid_input = server.arg("ssid");
    String wifi_password_input = server.arg("wifipassword");
    String mqtt_broker_input = server.arg("mqtt_broker");
    String mqtt_username_input = server.arg("mqtt_username");
    String mqtt_password_input = server.arg("mqtt_password");

    // Speichern der WLAN- und MQTT-Einstellungen
    preferences.putString("ssid", ssid_input);
    preferences.putString("wifi_password", wifi_password_input);
    preferences.putString("mqtt_broker", mqtt_broker_input);
    preferences.putString("mqtt_username", mqtt_username_input);
    preferences.putString("mqtt_password", mqtt_password_input);

    // Feedback an den Benutzer
    server.send(200, "text/html", "<h1>Einstellungen gespeichert!</h1><p>Gerät wird neu gestartet...</p>");
    delay(2000);
    ESP.restart();  // Gerät neu starten
  });

  server.begin();  // Webserver starten
  Serial.println("Webserver gestartet.");
}

// Verbindung zu MQTT herstellen
void connectToMQTT() {
  client.setServer(mqtt_broker, 1883);  // MQTT-Broker und Port
  while (!client.connected()) {
    Serial.println("Verbindung zu MQTT-Broker...");
    if (client.connect("ESP32", mqtt_username, mqtt_password)) {
      Serial.println("Verbunden!");
      client.publish(mqtt_topic_status, "0");  // Schlüsselschalter Status (0 = aus)
      client.publish(mqtt_topic_wificonnect, getCurrentTime().c_str());  // Zeitpunkt der WLAN-Verbindung
    } else {
      Serial.print("Fehler, rc=");
      Serial.println(client.state());  // Detaillierte Fehlerausgabe
      delay(2000);  // Warten, bevor erneut versucht wird, eine Verbindung herzustellen
    }
  }
}

// Verbindung zum WLAN herstellen
void connectToWiFi() {
  WiFi.begin(ssid, password);  // Mit dem konfigurierten WLAN verbinden
  Serial.print("Verbinde mit WLAN: ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("Mit WLAN verbunden!");
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());
  client.publish(mqtt_topic_wificonnect, getCurrentTime().c_str());  // Zeitpunkt der WLAN-Verbindung
}

// Holen der aktuellen Zeit (Datum und Uhrzeit)
String getCurrentTime() {
  timeClient.update();
  String time = timeClient.getFormattedTime();
  return time;
}

// Setup-Funktion
void setup() {
  Serial.begin(115200);

  // Speicher initialisieren
  preferences.begin("mqtt_wifi", false);

  // WLAN und MQTT-Daten aus dem Speicher laden
  String saved_ssid = preferences.getString("ssid", "");
  String saved_wifi_password = preferences.getString("wifi_password", "");
  String saved_mqtt_broker = preferences.getString("mqtt_broker", "");
  String saved_mqtt_username = preferences.getString("mqtt_username", "");
  String saved_mqtt_password = preferences.getString("mqtt_password", "");

  // Wenn gespeicherte Werte vorhanden sind, verwenden
  if (saved_ssid.length() > 0) {
    saved_ssid.toCharArray(ssid, 50);
    saved_wifi_password.toCharArray(password, 50);
    saved_mqtt_broker.toCharArray(mqtt_broker, 50);
    saved_mqtt_username.toCharArray(mqtt_username, 50);
    saved_mqtt_password.toCharArray(mqtt_password, 50);
    Serial.println("WLAN- und MQTT-Daten aus Speicher geladen.");
  } else {
    Serial.println("Keine gespeicherten WLAN- oder MQTT-Daten gefunden.");
  }

  // Wenn keine WLAN- und MQTT-Daten gespeichert sind, starte den Access Point und warte auf die Konfiguration
  if (saved_ssid.length() == 0 || saved_mqtt_broker.length() == 0) {
    WiFi.softAP(ssid, password);  // Access Point starten
    Serial.println("Access Point gestartet.");
    Serial.print("IP-Adresse: ");
    Serial.println(WiFi.softAPIP());

    // Captive Portal einrichten
    setupCaptivePortal();
    while (true) {
      server.handleClient();  // Warte auf Benutzeranfragen
    }
  } else {
    // Wenn die Konfiguration gespeichert wurde, verbinde dich mit dem WLAN und dem MQTT-Broker
    connectToWiFi();
    connectToMQTT();
  }

  // Schlüsselschalter (GPIO 21) als Input mit Pull-Up-Widerstand konfigurieren
  pinMode(21, INPUT_PULLUP);  // GPIO 21 als Eingang mit internem Pull-Up-Widerstand
}

// Hauptloop
void loop() {
  // Überprüfe, ob die WLAN-Verbindung noch besteht
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WLAN-Verbindung verloren. Versuche erneut...");
    connectToWiFi();  // WLAN erneut verbinden
  }

  // Überprüfe, ob die MQTT-Verbindung besteht, und stelle sie bei Bedarf wieder her
  if (!client.connected()) {
    connectToMQTT();  // Versuche erneut, eine Verbindung herzustellen
  }

  // MQTT-Loop ausführen, um Nachrichten zu empfangen
  client.loop();

  // Status des Schlüssels (GPIO 21) lesen und an MQTT senden
  int schluesselschalterStatus = digitalRead(21);  // GPIO 21 als Schalter
  client.publish(mqtt_topic_status, schluesselschalterStatus == LOW ? "1" : "0");  // LOW = Schalter geschlossen (1), HIGH = Schalter offen (0)

  // Sende das Datum und die Uhrzeit der letzten Aktualisierung
  client.publish(mqtt_topic_lastupdate, getCurrentTime().c_str());

  delay(1000);  // 1 Sekunde warten
}
