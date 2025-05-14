#include "config.h"
#include "buttons.h"
#include "ntp.h"
#include "sensors.h"
#include "nixiedisplay.h"
#include <WiFi.h>
#include <WebServer.h>

// The working WebServer instance for your ESP32:
WebServer server(80);

extern NixieDisplay nixieDisplay;
bool clockUpdateEnabled = true;
bool leftDotEnabled  = false;
bool rightDotEnabled = false;


// ---------------------------------------------------------------------
// Returns inline CSS for a dark theme, glowing orange text, larger fonts,
// flex layout for GPIO buttons, and styled links as buttons.
String getCSS() {
  String css = "<style>"
    "body { background-color: #000; color: #FFA500; font-family: Arial, sans-serif; margin: 0; padding: 0; }"
    ".container { max-width: 800px; margin: 0 auto; padding: 20px; }"
    "h1, h2 { text-align: center; }"
    "p { text-align: center; }"
    "a.button { display: inline-block; background-color: #222; border: 1px solid #FFA500; color: #FFA500; padding: 10px 20px; margin: 10px 5px; text-decoration: none; cursor: pointer; }"
    "a.button:hover { background-color: #FFA500; color: #000; }"
    "button { background-color: #222; border: 1px solid #FFA500; color: #FFA500; padding: 10px 20px; margin: 5px; cursor: pointer; }"
    "button.on { background-color: #FFA500; color: #000; }"
    "button.off { background-color: #222; color: #FFA500; }"
    ".gpio-container { display: flex; flex-wrap: wrap; justify-content: center; gap: 10px; margin-bottom: 20px; }"
    "@media (max-width:600px) { .container { padding: 10px; } button { width: 100%; box-sizing: border-box; } }"
    "</style>";
  return css;
}

// ---------------------------------------------------------------------
// Main Status Page: Displays current time, sensor data, and a button to access Debug Tools.
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Nixie Clock</title>";
  html += getCSS();
  html += "</head><body><div class='container'>";
  html += "<h1>Nixie Clock Status</h1>";
  
  // Display current time.
  html += "<p><strong>Current Time:</strong> " + getFormattedTime() + "</p>";
  
  // Display sensor data.
  SensorData data = readSensors();
  html += "<h2>Sensor Data</h2>";
  html += "<p>AHT Temperature: " + String(data.temperatureAHT) + " °C<br>"
               "Humidity: " + String(data.humidity) + " %<br>"
               "BMP Temperature: " + String(data.temperatureBMP) + " °C<br>"
               "Pressure: " + String(data.pressure) + " hPa</p>";
  
  // Navigation: button linking to Debug Tools page.
  html += "<p><a class='button' href='/debug'>Go to Debug Tools</a></p>";
  
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

// ---------------------------------------------------------------------
// Debug Page: Contains controls for toggling GPIO outputs,
// shows the physical button status, and provides an input for setting the display number.
void handleDebug() {
  // Start HTML + head
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Debug Tools</title>";
  html += getCSS();

  // Single <script> block with ALL of our AJAX functions:
  html += "<script>"
            // 1) Toggle a GPIO pin
            "function toggleGPIO(btn, pin) {"
              "let curr = btn.getAttribute('data-state');"
              "let nxt  = (curr === 'HIGH') ? 'LOW' : 'HIGH';"
              "fetch('/toggle?pin='+pin+'&state='+nxt)"
                ".then(r=>r.text())"
                ".then(state=>{"
                  "btn.innerHTML = 'GPIO '+pin+' ('+state+')<br>'+btn.getAttribute('data-pinname');"
                  "btn.setAttribute('data-state', state);"
                  "btn.classList.toggle('on',  state==='HIGH');"
                  "btn.classList.toggle('off', state==='LOW');"
                "});"
            "}"
            // 2) Set a manual display number
            "function setDisplayNumber() {"
              "let num = document.getElementById('numInput').value;"
              "fetch('/setnumber?number='+num)"
                ".then(r=>r.text())"
                ".then(msg=>{ document.getElementById('setNumberMsg').innerText = msg; });"
            "}"
            // 3) Toggle the left/right dots
            "function toggleDot(btn, which) {"
              "let curr = btn.getAttribute('data-state');"
              "let nxt  = (curr === 'ON') ? 'OFF' : 'ON';"
              "fetch('/toggleDot?which='+which+'&state='+nxt)"
                ".then(r=>r.text())"
                ".then(state=>{"
                  "btn.setAttribute('data-state', state);"
                  "btn.innerText = (which==='left'?'Left Dot ':'Right Dot ')+ '('+state+')';"
                  "btn.classList.toggle('on',  state==='ON');"
                  "btn.classList.toggle('off', state==='OFF');"
                "});"
            "}"
            // 4) Enable/disable auto‐update
            "function toggleClock() {"
              "let want = document.getElementById('clockBtn').innerText==='Enable'?'true':'false';"
              "fetch('/autoupdate?enable='+want)"
                ".then(r=>r.text())"
                ".then(state=>{"
                  "let btn = document.getElementById('clockBtn');"
                  "let en  = (state==='ENABLED');"
                  "btn.innerText = en?'Disable':'Enable';"
                  "btn.classList.toggle('on',  en);"
                  "btn.classList.toggle('off', !en);"
                "});"
            "}"
          "</script>";

  // Close head, open body
  html += "</head><body><div class='container'>";
  html += "<h1>Debug Tools</h1>";

  // --- GPIO toggle section (unchanged) ---
  html += "<h2>Toggle GPIO Outputs</h2><div class='gpio-container'>";
  int togglePins[]  = { SR_CLK_PIN, SR_DIN_PIN, SR_LE_PIN, HV_ENABLE_PIN };
  String toggleNames[] = { "CLK", "DIN", "LE", "HVEN" };
  for (int i = 0; i < 4; i++) {
    int pin = togglePins[i];
    pinMode(pin, OUTPUT);
    String st  = (digitalRead(pin)==HIGH) ? "HIGH" : "LOW";
    String cls = (st=="HIGH") ? "on" : "off";
    html += "<button data-state='" + st + "' data-pinname='" + toggleNames[i] +
            "' class='" + cls + "' onclick='toggleGPIO(this," + String(pin) + ")'>"
            "GPIO " + String(pin) + " (" + st + ")<br>" + toggleNames[i] + "</button>";
  }
  html += "</div>";

  // --- Dot Control section ---
  html += "<h2>Dot Control</h2><div class='gpio-container'>";
    html += "<button id='leftDotBtn' data-state='"  + String(leftDotEnabled  ? "ON":"OFF") +
            "' class='" + String(leftDotEnabled  ? "on":"off") +
            "' onclick=\"toggleDot(this,'left')\">Left Dot (" +
            String(leftDotEnabled  ? "ON":"OFF") + ")</button>";
    html += "<button id='rightDotBtn' data-state='" + String(rightDotEnabled ? "ON":"OFF") +
            "' class='" + String(rightDotEnabled ? "on":"off") +
            "' onclick=\"toggleDot(this,'right')\">Right Dot (" +
            String(rightDotEnabled ? "ON":"OFF") + ")</button>";
  html += "</div>";

  // --- Rest of Debug page (buttons, sensor readouts, etc.) ---
  html += "<h2>Physical Button Status</h2><p>"
          "Button 1 (GPIO " + String(BUTTON1_PIN) + "): " + (isButtonPressed(BUTTON1_PIN) ? "Pressed" : "Released") + "<br>"
          "Button 2 (GPIO " + String(BUTTON2_PIN) + "): " + (isButtonPressed(BUTTON2_PIN) ? "Pressed" : "Released") + "<br>"
          "Button 3 (GPIO " + String(BUTTON3_PIN) + "): " + (isButtonPressed(BUTTON3_PIN) ? "Pressed" : "Released") +
        "</p>"
        "<h2>Set Display Number</h2><p>"
          "<input id='numInput' type='text' maxlength='6' pattern='[0-9]{1,6}' placeholder='Enter number'> "
          "<button onclick='setDisplayNumber()'>Set Number</button>"
        "</p><p id='setNumberMsg'></p>"
        "<h2>Clock Auto-Update</h2><p>"
          "<button id='clockBtn' class='" + String(clockUpdateEnabled ? "on":"off") +
          "' onclick='toggleClock()'>" + (clockUpdateEnabled ? "Disable":"Enable") + " Clock</button>"
        "</p>"
        "<p><a class='button' href='/'>Return to Home</a></p>";

  // Finish and send
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

// ---------------------------------------------------------------------
// AJAX Endpoint: Toggle the specified GPIO output.
void handleToggle() {
  if (server.hasArg("pin") && server.hasArg("state")) {
    int pin = server.arg("pin").toInt();
    String desiredState = server.arg("state");
    int state = (desiredState == "HIGH") ? HIGH : LOW;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, state);
    // Confirm new state.
    int newState = digitalRead(pin);
    String response = (newState == HIGH) ? "HIGH" : "LOW";
    server.send(200, "text/plain", response);
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

// new AJAX handler to flip individual dots
void handleToggleDot() {
  if (server.hasArg("which") && server.hasArg("state")) {
    String which = server.arg("which");
    bool   on    = (server.arg("state") == "ON");

    if (which == "left") {
      leftDotEnabled = on;
      if (on)  nixieDisplay.enableSegment(leftDot);
      else     nixieDisplay.disableSegment(leftDot);
    }
    else if (which == "right") {
      rightDotEnabled = on;
      if (on)  nixieDisplay.enableSegment(rightDot);
      else     nixieDisplay.disableSegment(rightDot);
    }

    nixieDisplay.updateDisplay();
    server.send(200, "text/plain", on ? "ON" : "OFF");
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

// ---------------------------------------------------------------------
// AJAX Endpoint: Set the display number.
void handleSetNumber() {
  if (server.hasArg("number")) {
    uint32_t number = server.arg("number").toInt();
    nixieDisplay.showNumber(number);
    nixieDisplay.updateDisplay();
    server.send(200, "text/plain", "Display updated with number: " + String(number));
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

// AJAX Endpoint: Enable or disable automatic clock updates
void handleAutoUpdate() {
  if (server.hasArg("enable")) {
    clockUpdateEnabled = (server.arg("enable") == "true");
    server.send(200, "text/plain", clockUpdateEnabled ? "ENABLED" : "DISABLED");
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

// ---------------------------------------------------------------------
// Initialize the web server: set routes and start the server.
void initWebServer() {
  server.on("/", handleRoot);
  server.on("/debug", handleDebug);
  server.on("/toggle", handleToggle);
  server.on("/setnumber", handleSetNumber);
  server.on("/toggleDot", handleToggleDot);
  server.on("/autoupdate", handleAutoUpdate);
  
  server.begin();
  Serial.println("Web server started");
}

// ---------------------------------------------------------------------
// Call from your main loop to handle incoming requests.
void handleWebRequests() {
  server.handleClient();
}
