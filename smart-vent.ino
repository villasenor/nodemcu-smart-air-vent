// Simple WiFi-controlled Smart Vent on NodeMCU V1 and L9110 motor drive board
// Adapted from https://www.hackster.io/alankrantas/simple-nodemcu-wifi-controlled-car-esp8266-c5491e
// Adapted from https://www.mischianti.org/2020/06/30/how-to-create-a-rest-server-on-esp8266-or-esp32-post-put-patch-delete-part-3/
// Designed to work with Home Assistant RESTful Switch (https://www.home-assistant.io/integrations/switch.rest/)

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

// WiFi settings
#define WIFI_MODE           2                     // 1 = AP mode, 2 = STA mode
#define SSID_AP             "NodeMCU_WiFi_Smart_Vent"    // for AP mode
#define PASSWORD_AP         "12345678"            // for AP mode
#define SSID_STA            "SSID"      // for STA mode
#define PASSWORD_STA        "PASSWORD"  // for STA mode

// motor settings
#define VENT_MOTOR_PIN1    4                     // pin 1 of VENT motor (D2)
#define VENT_MOTOR_PIN2    5                     // pin 2 of VENT motor (D1)
#define VENT_MOTOR_SPEED   1023                 //speed for VENT motor (0-1023)

IPAddress local_ip(192, 168, 1, 200); //IP for AP mode
IPAddress gateway(192, 168, 1, 1); //IP for AP mode
IPAddress subnet(255, 255, 255, 0); //IP for AP mode
ESP8266WebServer server(80);
bool status_val = false;

// initialize
void setup() {
    Serial.begin(9600);
    Serial.println("NodeMCU Wifi Smart Vent");
    pinMode(VENT_MOTOR_PIN1, OUTPUT);
    pinMode(VENT_MOTOR_PIN2, OUTPUT);

    if (WIFI_MODE == 1) { // AP mode
        WiFi.softAP(SSID_AP, PASSWORD_AP);
        WiFi.softAPConfig(local_ip, gateway, subnet);
    } else { // STA mode
        WiFi.begin(SSID_STA, PASSWORD_STA);
        Serial.print("Connecting to WiFi...");
        while (WiFi.status() != WL_CONNECTED) {
            delay(100);
            Serial.print(".");
        }
        Serial.println("");
        Serial.print("Connected! IP: ");
        Serial.println(WiFi.localIP()); //the IP is needed for connection in STA mode
    }

    // setup web server to handle specific HTTP requests
    server.on("/", HTTP_GET, handle_OnConnect);
    server.on("/status", HTTP_GET, handle_status);
    server.on("/control", HTTP_POST, handle_post);
    server.on("/stop", HTTP_GET, handle_estop);
    server.onNotFound(handle_NotFound);

    //start server
    server.begin();
    Serial.println("NodeMCU web server started.");
}

void loop() {
    server.handleClient();
}

void handle_post() {
    String postBody = server.arg("plain");
    Serial.println(postBody);

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, postBody);
    if (error) {
        // if the file didn't open, print an error:
        Serial.print(F("Error parsing JSON "));
        Serial.println(error.c_str());

        String msg = error.c_str();

        server.send(400, F("text/html"),
                    "Error parsing JSON body due to " + msg);

    } else {
        JsonObject postObj = doc.as<JsonObject>();

        if (server.method() == HTTP_POST) {
            if (postObj.containsKey("active")) {


                if (postObj["active"] == "true") {
                    handle_close();
                } else if (postObj["active"] == "false") {
                    handle_open();
                } else {
                    DynamicJsonDocument doc(512);
                    doc["status"] = "ERROR";
                    doc["message"] = F("Invalid state for key");

                    String buf;
                    serializeJson(doc, buf);

                    server.send(400, F("application/json"), buf);
                }


                DynamicJsonDocument doc(512);
                doc["status"] = "OK";

                String buf;
                serializeJson(doc, buf);

                server.send(201, F("application/json"), buf);

            } else {
                DynamicJsonDocument doc(512);
                doc["status"] = "ERROR";
                doc["message"] = F("Invalid Key");

                String buf;
                serializeJson(doc, buf);

                server.send(400, F("application/json"), buf);
            }
        }
    }
}


void handle_OnConnect() {
    server.send(200, "text/html", SendHTML());
}

void handle_estop() {
    digitalWrite(VENT_MOTOR_PIN1, LOW);
    digitalWrite(VENT_MOTOR_PIN2, LOW);
    Serial.println("Stopped");
    server.send(200, "text/html", SendHTML());
}

void handle_status() {
    DynamicJsonDocument doc(512);
    doc["is_active"] = status_val;

    String buf;
    serializeJson(doc, buf);

    server.send(200, F("application/json"), buf);
}

void handle_open() {
    analogWrite(VENT_MOTOR_PIN1, VENT_MOTOR_SPEED);
    analogWrite(VENT_MOTOR_PIN2, 0);
    delay(500);
    digitalWrite(VENT_MOTOR_PIN1, LOW);
    digitalWrite(VENT_MOTOR_PIN2, LOW);
    status_val = false;
}

void handle_close() {
    analogWrite(VENT_MOTOR_PIN2, VENT_MOTOR_SPEED);
    analogWrite(VENT_MOTOR_PIN1, 0);
    delay(500);
    digitalWrite(VENT_MOTOR_PIN1, LOW);
    digitalWrite(VENT_MOTOR_PIN2, LOW);
    status_val = true;
}

void handle_NotFound() {
    Serial.println("Page error");
    server.send(404, "text/plain", "Not found");
}

// output HTML web page for user
String SendHTML() {
    String html = "<!DOCTYPE html>\n";
    html += "<html>\n";
    html += "<head>\n";
    html += "<title>NodeMCU Smart Vent</title>\n";
    html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
    html += "</head>\n";
    html += "<body>\n";
    html += "<div align=\"center\">\n";
    html += "<h1>NodeMCU Smart Vent</h1>\n";
    html += "<br>\n";
    html += "<form method=\"GET\">\n";
    html += "<input type=\"button\" value=\"Status\" onclick=\"window.location.href='/status'\">\n";
    html += "<br><br>\n";
    html += "<input type=\"button\" value=\"ESTOP\" onclick=\"window.location.href='/stop'\">\n";
    html += "</form>\n";
    html += "</div>\n";
    html += "</body>\n";
    html += "</html>\n";
    return html;
}
