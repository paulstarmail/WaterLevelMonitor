#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClient.h>

#define TRIG_PIN                                                               \
  23 // ESP32 pin GIOP23 connected to Ultrasonic Sensor's TRIG pin
#define ECHO_PIN                                                               \
  22 // ESP32 pin GIOP22 connected to Ultrasonic Sensor's ECHO pin

// Initiating HTTP server
WebServer server(80);
// Configuring Static IP Address. Modify these according to your needs.
IPAddress local_IP(192, 168, 1, 232);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

// Setting Network Credentials
const char *ssid = "<name of the WiFi connection>";
const char *password = "<password of the WiFi connection>";

// Declaring some variables
float duration_us, distance_cm, percent;

// Storing the HTML design to a variable
String webpageCode = R"***(
<!DOCTYPE html>
<head>
  <title>Tank Level</title>
</head>
<html>
<!----------------------------HTML--------------------------->
<body>
  <div>
    <h1><span style="background-color:white">Tank Level</span></h1>
    <h2>
      Level: <span style="color:red" id="level">Error</span>
      <br>
      <br>
      Developer: <span style="color:green">Amal K Paul</span>
    </h2>
  </div>
<!-------------------------JavaScrip------------------------->
  <script>
    setInterval(function()
    {
      getLevel();
    }, 1000);
    //-------------------------------------------------------
    function getLevel()
    {
      var levelRequest = new XMLHttpRequest();
      levelRequest.onreadystatechange = function()
      {
        if(this.readyState == 4 && this.status == 200)
        {
          document.getElementById("level").innerHTML =
          this.responseText;
        }
      };
      levelRequest.open("GET", "readLevel", true);
      levelRequest.send();
    }
  </script>
</body>
</html>
)***";

// Handler function for root of the web-application
void handleRoot() { server.send(200, "text/html", webpageCode); }

// Handler function of the web application to find the water level
void handleLevel() {
  // Generate 10-microsecond pulse to TRIG pin
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure duration of pulse from ECHO pin
  duration_us = pulseIn(ECHO_PIN, HIGH);

  // Calculate the distance
  distance_cm = 0.017 * duration_us;

  /*
  General formula for the water level percentage is: p = K*(d_max-d)
  where, p = percentage of water level, K = a constant,  d_max = distance from
  sensor to the bottom of the tank, d = distance from sensor to
  water level
  */

  // Tank specific special formula derived from the General formula
  percent = 1.11 * (100 - distance_cm);
  Serial.print("Percentage of Water: "); // Print the value to Serial Monitor
  Serial.print(percent);
  Serial.println("%");

  String level = String(percent) + "%";
  server.send(200, "text/plane", level);
}

void setup() {
  // Begin serial port
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  // Configure the trigger pin to output mode
  pinMode(TRIG_PIN, OUTPUT);
  // Configure the echo pin to input mode
  pinMode(ECHO_PIN, INPUT);

  WiFi.mode(WIFI_STA); // Configuring in Station Mode
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connecting in setup()");
    delay(500);
  }
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Mapping the requests to Handler functions
  server.on("/", handleRoot);
  server.on("/readLevel", handleLevel);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);

  // Checking whether the connectivity is lost
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);
    WiFi.disconnect();
    Serial.println("Connecting to WiFi...");
    WiFi.reconnect();
    delay(1000);
  }

  // Handle the Client
  server.handleClient();

  delay(1);
}
