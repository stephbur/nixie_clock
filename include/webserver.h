#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <WiFi.h>
#include <WebServer.h>

// The header declares the functions used by the main application.

void initWebServer();
void handleWebRequests();

// Page Handlers:
void handleRoot();    // Main Status page
void handleDebug();   // Debug page with IO toggling, button status, and number setting

// API endpoints (AJAX):
void handleToggle();
void handleSetNumber();

#endif // WEBSERVER_H
