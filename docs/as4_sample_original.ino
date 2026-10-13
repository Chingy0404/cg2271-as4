/*
--------------------------------------------------------------------------------
-- {c} Low Tjun Lym, NUS
-- License terms:
-- You are free to use this code under the following conditions:
--    (i) use it only for educational purposes;
--    (ii) accept the responsibility to ensure that your implementation does not violate any intellectual property of any entity.
--    (iii) accept that the program is provided "as is" without warranty of any kind or assurance regarding its suitability for any particular purpose;
--------------------------------------------------------------------------------
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>


const char* ssid = ""; // Fill in your own hospot SSID here
const char* password = ""; // Fill in your own hotspot password here.

// Your Domain name with URL path or IP address with path
//String openWeatherMapApiKey = "REPLACE_WITH_YOUR_OPEN_WEATHER_MAP_API_KEY";
// Example:
String openWeatherMapApiKey = ""; // Replade with your own Open Weather Map API key.

// Replace with your country code and city
String city = "Singapore";
String countryCode = "SG";

// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
// The limit for this key is 60 calls/minute, 1000000 calls per month. For this example, we delay 10 seconds to avoid getting blocked.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 10 seconds (10000)
unsigned long timerDelay = 10000;

String jsonBuffer;

void setup() {
  Serial.begin(115200);
  Serial.print("Connecting to SSID:");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.print("\nConnected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.print("Timer set to "); 
  Serial.print(timerDelay);
  Serial.println(" ms, which delays API calls.");
}

void loop() {
  // Send an HTTP GET request
  if ((millis() - lastTime) > timerDelay) {
    // Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;
      
      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);
      JSONVar myObject = JSON.parse(jsonBuffer);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
    
      Serial.print("JSON object = ");
      Serial.println(myObject);
      Serial.print("Temperature: ");
      Serial.println(myObject["main"]["temp"]);
      Serial.print("Pressure: ");
      Serial.println(myObject["main"]["pressure"]);
      Serial.print("Humidity: ");
      Serial.println(myObject["main"]["humidity"]);
      Serial.print("Wind Speed: ");
      Serial.println(myObject["wind"]["speed"]);
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}
