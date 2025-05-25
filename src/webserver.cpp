#include "config.h"
#include "buttons.h"
#include "ntp.h"
#include "sensors.h"
#include "nixiedisplay.h"
#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

extern NixieDisplay nixieDisplay;
bool clockUpdateEnabled = true;
bool leftDotEnabled  = false;
bool rightDotEnabled = false;

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

void handleRoot() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Nixie Clock</title>";
    html += getCSS();
    html += "</head><body><div class='container'>";
    html += "<h1>Nixie Clock Status</h1>";

    html += "<p><strong>Current Time:</strong> " + getFormattedTime() + "</p>";

    SensorData data = readSensors();
    html += "<h2>Sensor Data</h2>";
    html += "<p>AHT Temperature: " + String(data.temperatureAHT) + " °C<br>"
                 "Humidity: " + String(data.humidity) + " %<br>"
                 "BMP Temperature: " + String(data.temperatureBMP) + " °C<br>"
                 "Pressure: " + String(data.pressure) + " hPa</p>";

    html += "<p><a class='button' href='/debug'>Go to Debug Tools</a></p>";
    html += "</div></body></html>";
    server.send(200, "text/html", html);
}

void handleDebug() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Debug Tools</title>";
    html += getCSS();
    html += "<script>"
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
        "function setDisplayNumber() {"
            "let num = document.getElementById('numInput').value;"
            "fetch('/setnumber?number='+num)"
              ".then(r=>r.text())"
              ".then(msg=>{ document.getElementById('setNumberMsg').innerText = msg; });"
        "}"
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

    html += "</head><body><div class='container'>";
    html += "<h1>Debug Tools</h1>";

    html += "<h2>Toggle GPIO Outputs</h2><div class='gpio-container'>";
    int togglePins[]  = { SR_CLK_PIN, SR_DIN_PIN, SR_LE_PIN, HV_ENABLE_PIN };
    String toggleNames[] = { "CLK", "DIN", "LE", "HVEN" };
    for (int i = 0; i < 4; i++) {
        int pin = togglePins[i];
        pinMode(pin, OUTPUT);
        String st  = (digitalRead(pin) == HIGH) ? "HIGH" : "LOW";
        String cls = (st == "HIGH") ? "on" : "off";
        html += "<button data-state='" + st + "' data-pinname='" + toggleNames[i] +
                "' class='" + cls + "' onclick='toggleGPIO(this," + String(pin) + ")'>"
                "GPIO " + String(pin) + " (" + st + ")<br>" + toggleNames[i] + "</button>";
    }
    html += "</div>";

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

    html += "<h2>Physical Button Status</h2><p>";
    html += "Button 1: " + String(isButtonPressed(BUTTON_1) ? "Pressed" : "Released") + "<br>";
    html += "Button 2: " + String(isButtonPressed(BUTTON_2) ? "Pressed" : "Released") + "<br>";
    html += "Button 3: " + String(isButtonPressed(BUTTON_3) ? "Pressed" : "Released") + "</p>";


    html += "<h2>Set Display Number</h2><p>"
            "<input id='numInput' type='text' maxlength='6' pattern='[0-9]{1,6}' placeholder='Enter number'> "
            "<button onclick='setDisplayNumber()'>Set Number</button>"
            "</p><p id='setNumberMsg'></p>";

    html += "<h2>Clock Auto-Update</h2><p>"
            "<button id='clockBtn' class='" + String(clockUpdateEnabled ? "on":"off") +
            "' onclick='toggleClock()'>" + (clockUpdateEnabled ? "Disable":"Enable") + " Clock</button>"
            "</p>";

    html += "<p><a class='button' href='/'>Return to Home</a></p>";
    html += "</div></body></html>";
    server.send(200, "text/html", html);
}

void handleToggle() {
    if (server.hasArg("pin") && server.hasArg("state")) {
        int pin = server.arg("pin").toInt();
        String desiredState = server.arg("state");
        int state = (desiredState == "HIGH") ? HIGH : LOW;
        pinMode(pin, OUTPUT);
        digitalWrite(pin, state);
        int newState = digitalRead(pin);
        String response = (newState == HIGH) ? "HIGH" : "LOW";
        server.send(200, "text/plain", response);
    } else {
        server.send(400, "text/plain", "Bad Request");
    }
}

void handleToggleDot() {
    if (server.hasArg("which") && server.hasArg("state")) {
        String which = server.arg("which");
        bool on = (server.arg("state") == "ON");

        if (which == "left") {
            leftDotEnabled = on;
            on ? nixieDisplay.enableSegment(leftDot) : nixieDisplay.disableSegment(leftDot);
        } else if (which == "right") {
            rightDotEnabled = on;
            on ? nixieDisplay.enableSegment(rightDot) : nixieDisplay.disableSegment(rightDot);
        }

        nixieDisplay.updateDisplay();
        server.send(200, "text/plain", on ? "ON" : "OFF");
    } else {
        server.send(400, "text/plain", "Bad Request");
    }
}

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

void handleAutoUpdate() {
    if (server.hasArg("enable")) {
        clockUpdateEnabled = (server.arg("enable") == "true");
        server.send(200, "text/plain", clockUpdateEnabled ? "ENABLED" : "DISABLED");
    } else {
        server.send(400, "text/plain", "Bad Request");
    }
}

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

void handleWebRequests() {
    server.handleClient();
}
