#include <ESP8266WiFi.h>
#include <WiFiUdp.h>



#define BUILT_IN_LED (2)            /// GPID of built in led
#define WIFI_WAIT_TIME (10000)      /// Wait time for connecting to wifi
#define WIFI_DELAY (500)
#define WIFI_SSID ("Redmi 8A")      /// Hotspot SSID
#define WIFI_PASS ("Aa123QWE")      /// Hotspot Password

#define HOST_IP ("185.18.214.189")  /// Server ip
#define HOST_PORT (9999)            /// Server port

#define BUFF_SIZE (512)             /// Size of buffer for reading from socket
#define ARRAY_SIZE (30)             /// Size of array to store result

/// protocols types
#define TCP (1)
#define UDP (2)
#define HTTP (3)
#define QUIC (4)
/// tests types
#define DOWNLOAD (5)
#define UPLOAD (6)
#define LATENCY (7)

#define PROTOCOL (TCP)              /// protocol type
#define TEST (DOWNLOAD)             /// test type


char* buff = (char*)malloc(BUFF_SIZE);
unsigned long st;
int result_array[ARRAY_SIZE];

/// Connect to wifi
bool setupWifi() {
  Serial.printf("connecting to %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(WIFI_DELAY);
    Serial.print(++i); Serial.print(' ');
    if (i == WIFI_WAIT_TIME/WIFI_DELAY){
      Serial.println("Failed to connect");
      WiFi.disconnect();
      return false;
    }
    Serial.println("retry");
  }
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP()); 
  return true;
}

void printResultArray() {
  for (int i = 0; i < ARRAY_SIZE && result_array[i]; i++) {
    Serial.printf("%d, ", result_array[i]);
  }
}

void tcp() {
  /// Init variables
  st = millis();
  
  /// Connecting to server
  WiFiClient client;
  if (client.connect(HOST_IP, HOST_PORT)){  // connect to server
    Serial.println("Connected to server");
  }else{
    return;
  }

  /// Download test
  if (TEST == DOWNLOAD) {
    while (client.connected()) { 
      if(client.available()) {
        result_array[(millis()-st)/1000] += client.read(buff, BUFF_SIZE);
      }
    }
    Serial.printf("TCP Download result:");
    printResultArray();
  }
}

void setup() {
  ESP.eraseConfig(); /// this make esp faster
  Serial.begin(9600);
  delay(10); /// waits for Serial to begin
  digitalWrite(BUILT_IN_LED, HIGH); /// turn the LED off.
  bool ok = true;
  if (setupWifi()) {
    Serial.println("OK");
    digitalWrite(BUILT_IN_LED, LOW); // turn the LED on.
  }
  if (PROTOCOL == TCP)
    tcp();
  Serial.println("\nDone");
}
void loop() {
}
