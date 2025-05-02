

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebSrv.h>

AsyncWebServer server(80);

//Wi-Fi credentials
const char* ssid = "UmarKhalifah";
const char* password = "ajike250700";

//Initialize MQ sensors

const int MQ2_PIN = 34;
const int MQ7_PIN = 36;
const int MQ135_PIN = 35;

float MQ2R0 = 0.0;
float MQ7R0 = 0.0;
float MQ135R0 = 0.0;

// Variables for storing sensor voltages
float MQ2sensor_volt = 0.0;
float MQ7sensor_volt = 0.0;
float MQ135sensor_volt = 0.0;


void notFound(AsyncWebServerRequest *request) 
{
request->send(404, "text/plain", "Not found");
}


// Function to fetch real-time data from sensors and update webpage


void setup() 

{
  
Serial.begin(115200);
WiFi.mode(WIFI_STA);
WiFi.begin(ssid, password);
if (WiFi.waitForConnectResult() != WL_CONNECTED)
{
  
Serial.printf("WiFi Failed!\n");
return;
     
}

Serial.print("IP Address: ");
Serial.println(WiFi.localIP());

//server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
//{
//  
//request->send(200, "text/plain", "Hello, world");
//}
//);


server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) 
  
 
{
  
String html = "<!DOCTYPE html><html><head><title>Vehicle Emission Monitoring System</title>";
html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
html += "<script>";
html += "function fetchData() {";
html += "  fetch('/realtime-data').then(response => response.json()).then(data => {";
html += "    document.getElementById('mq2').innerHTML = 'MQ2 R0: ' + data.mq2;";
html += "    document.getElementById('mq7').innerHTML = 'MQ7 R0: ' + data.mq7;";
html += "    document.getElementById('mq135').innerHTML = 'MQ135 R0: ' + data.mq135;";
html += "  });";
html += "}";
html += "setInterval(fetchData, 2000);"; // Fetch data every 2 seconds
html += "fetchData();"; // Fetch data when the page loads initially
html += "</script></head><body>";
html += "<h1>Vehicle Emission Monitoring System</h1>";
html += "<h2>Gas Sensor Readings:</h2>";
html += "<ul>";
html += "<li id='mq2'>MQ2 R0: -</li>";
html += "<li id='mq7'>MQ7 R0: -</li>";
html += "<li id='mq135'>MQ135 R0: -</li>";
html += "</ul>";
html += "</body></html>";

request->send(200, "text/html", html);
  
}

  
);


server.onNotFound(notFound);

server.begin();

}




void loop() {

//server.begin();

server.on("/realtime-data", HTTP_GET, [](AsyncWebServerRequest *request) 
{
  
fetchData(); // Fetch real-time data
String data = "{\"mq2\":" + String(MQ2R0) + ", \"mq7\":" + String(MQ7R0) + ", \"mq135\":" + String(MQ135R0) + "}";
request->send(200, "application/json", data);

}
  
);

  // Print sensor values and parameters


delay(2000);

}




void fetchData() 
{
  
int MQ2sensorValue = 0;
int MQ7sensorValue = 0;
int MQ135sensorValue = 0;

// Get an average data by testing 100 times

  for (int x = 0; x < 100; x++) 
{
  
MQ2sensorValue = MQ2sensorValue + analogRead(MQ2_PIN);
MQ7sensorValue = MQ7sensorValue + analogRead(MQ7_PIN);
MQ135sensorValue = MQ135sensorValue + analogRead(MQ135_PIN);

}

MQ2sensor_volt = MQ2sensorValue / 100.0 / 1024.0 * 5.0;
MQ7sensor_volt = MQ7sensorValue / 100.0 / 1024.0 * 5.0;
MQ135sensor_volt = MQ135sensorValue / 100.0 / 1024.0 * 5.0;

float MQ2RS_air = (5.0 - MQ2sensor_volt) / MQ2sensor_volt; // omit *RL
float MQ7RS_air = (5.0 - MQ7sensor_volt) / MQ7sensor_volt;
float MQ135RS_air = (5.0 - MQ135sensor_volt) / MQ135sensor_volt;

// Check for division by zero and set default values if necessary
MQ2R0 = (MQ2RS_air == 0) ? 0 : MQ2RS_air / 10.0;
MQ7R0 = (MQ7RS_air == 0) ? 0 : MQ7RS_air / 10.0;
MQ135R0 = (MQ135RS_air == 0) ? 0 : MQ135RS_air / 10.0;


Serial.print("MQ2sensor_volt = ");
Serial.print(MQ2sensor_volt);

Serial.print("MQ7sensor_volt = ");
Serial.print(MQ7sensor_volt);

Serial.print("MQ135sensor_volt = ");
Serial.print(MQ135sensor_volt);

Serial.print("MQ2R0 = ");
Serial.println(MQ2R0);

Serial.print("MQ7R0 = ");
Serial.println(MQ7R0);

Serial.print("MQ135R0 = ");
Serial.println(MQ135R0);
delay(300);
} //fe
