#include <ESP8266WiFi.h>
#include <WiFiUdp.h>



#define BUILT_IN_LED (2)            /// GPID of built in led
#define WIFI_WAIT_TIME (10000)      /// Wait time for connecting to wifi
#define WIFI_DELAY (500)
#define WIFI_SSID ("Redmi 8A")      /// Hotspot SSID
#define WIFI_PASS ("Aa123QWE")      /// Hotspot Password

#define HOST_IP ("185.18.214.189")  /// Server ip
IPAddress hostIP(185, 18, 214, 189);
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

#define PROTOCOL (UDP)              /// protocol type
#define TEST (LATENCY)             /// test type


WiFiUDP udp;
char* buff = (char*)malloc(BUFF_SIZE);
char* write_data = (char*)malloc(BUFF_SIZE);
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
  while (!client.connect(HOST_IP, HOST_PORT));  // connect to server
  Serial.println("Connected to server");

  if (TEST == DOWNLOAD) {
    while (client.connected()) { 
      if(client.available()) {
        result_array[(millis()-st)/1000] += client.read(buff, BUFF_SIZE);
      }
    }
    Serial.printf("TCP download result:");
    printResultArray();
  }else if(TEST == UPLOAD) {
    while (client.connected()) { 
      result_array[(millis()-st)/1000] += client.write(write_data, BUFF_SIZE);
      client.flush();
    }
    Serial.printf("TCP upload result:");
    printResultArray();
  }else {
    for (int i = 0; client.connected(); i++) {
      while (client.connected() && !client.available());
      client.read(buff, BUFF_SIZE);
      client.write(write_data, 1);
      client.flush();
    }
  }
}

void udpTest() {
  udp.beginPacket(hostIP, HOST_PORT);
  for (int i = 0; i < 10; i++){
    uint16_t packetSize = udp.parsePacket();              // Eingehende UDP-Pakete empfangen.
    Serial.println(packetSize);
    if (packetSize) {                                     // Prüfen ob UDP-Pakete empfangen wurden.
      udp.read(buff, BUFF_SIZE);       // Einlesen des UDP Paket in den Bufffer.
      buff[packetSize] = 0;                       // String Ende hinzufügen.
      Serial.println(buff);                       // Visualisierung des empfangenen Packets.
    }
    delay(1000);
  }
  udp.endPacket();
}



void setup() {
  ESP.eraseConfig(); /// this make esp faster
  Serial.begin(9600);
  delay(10); /// waits for Serial to begin
  digitalWrite(BUILT_IN_LED, HIGH); /// turn the LED off.
  for (int i = 0; i < BUFF_SIZE; i++) {
    write_data[i] = '$';
  }
  write_data[BUFF_SIZE-1] = '\0';
  bool ok = true;
  if (setupWifi()) {
    Serial.println("OK");
    digitalWrite(BUILT_IN_LED, LOW); // turn the LED on.
  }
  if (PROTOCOL == TCP)
    tcp();
  else if (PROTOCOL == UDP)
    udpTest();
  Serial.println("\nDone");
}
void loop() {
}
