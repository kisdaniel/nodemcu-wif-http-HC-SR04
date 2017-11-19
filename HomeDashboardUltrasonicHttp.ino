
#include <Wire.h>
#include <ESP8266WiFi.h>

// Replace with your network details
const char* ssid = "SSID";
const char* password = "pw";

const int trigPin = D4;
const int echoPin = D3;

// long duration;
// int d;
char distanceStr[6];

long duration;
int distance;
int tmp;
int avg;
int avgCnt;

// Web Server on port 80
WiFiServer server(80);

void flashLed() {
  digitalWrite(LED_BUILTIN, LOW);
  delay(400);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
}

// only runs once on boot
void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  flashLed();
  Serial.begin(115200);
  flashLed();  
  delay(10);
  // Wire.begin(D3, D4);
  // Wire.setClock(100000);
  // Connecting to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);  
    delay(1000);    
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Starting the web server
  server.begin();
  
  Serial.println("Web server running. Waiting for the ESP IP...");
  delay(10000);

  // Printing the ESP IP address
  Serial.println(WiFi.localIP());
  
  digitalWrite(LED_BUILTIN, HIGH);  

  // digitalWrite(trigPin, LOW);
}

int measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH, 100000);
  delayMicroseconds(20);
  // Calculating the distance
  return duration*0.034/2;
}

void clearAvg() {
  avg = 0;
  avgCnt = 0;
}

void addToAvgDistance() {  
  tmp = measureDistance();
  if (tmp > 0 && tmp < 100000) {
    avg += tmp;
    avgCnt++;
  }
}

void serveWifiClients() {
    // Listenning for new clients
   WiFiClient client = server.available();
 
   if (client) {
     
     // bolean to locate when the http request ends
     boolean blank_line = true;
     while (client.connected()) {
       Serial.println("Client connected");
       if (client.available()) {
         char c = client.read();
 
         if (c == '\n' && blank_line) {
           // digitalWrite(LED_BUILTIN, LOW);
           client.println("HTTP/1.1 200 OK");
           client.println("Content-Type: application/json");
           client.println("Connection: close");
           client.println();
           // your actual web page that displays temperature
           client.println("{");          
           client.println("\"sensors\": [");
           if (distance > 0 /*!isnan(d)*/) {
             client.println("{\"type\": \"distance\",");
             client.println("\"measure\": \"cm\",");
             client.print("\"value\": ");
             client.println(distance);
             client.println("}");
           }
           client.println("]");
           client.println("}");
           // digitalWrite(LED_BUILTIN, HIGH);
           break;
         }
         if (c == '\n') {
           // when starts reading a new line
           blank_line = true;
         }
         else if (c != '\r') {
           // when finds a character on the current line
           blank_line = false;
         }
       }
     }
     // closing the client connection
     client.stop();
   }
}

// runs over and over again
void loop() {
  serveWifiClients();

  clearAvg();
  addToAvgDistance();
  addToAvgDistance();
  addToAvgDistance();
  addToAvgDistance();
  if (avgCnt > 0) {
    distance = avg / avgCnt;
    if (distance > 60 && distance < 80) {
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      digitalWrite(LED_BUILTIN, LOW);
    }
    Serial.print("Distance ");
    Serial.println(distance);
  } else {
    distance = -1;
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("No distance");
  }

  
}
