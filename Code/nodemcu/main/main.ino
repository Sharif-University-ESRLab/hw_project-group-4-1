#include <ESP8266WiFi.h>
#include <WiFiUdp.h>                /// for UDP socket
#include <ESP8266HTTPClient.h>



#define BUILT_IN_LED (2)            /// GPID of built in led
#define WIFI_WAIT_TIME (10000)      /// Wait time for connecting to wifi
#define WIFI_DELAY (500)
#define WIFI_SSID  ("Xiaomi_08C5")/// ("Redmi 8A")      /// Hotspot SSID
#define WIFI_PASS    // ("Aa123QWE")      /// Hotspot Password
#define LOCAL_UDP_PORT (10210)     /// local port to listen on
#define MAX_PERIOD (15)
#define MAX_PACKETS (15)


#define HOST_IP ("185.18.214.189")  /// Server ip
IPAddress hostIP(185, 18, 214, 189);
#define HOST_PORT (9999)            /// Server port
#define HTTP_URL ("http://185.18.214.189:9999")
#define HTTP_DOWNLOAD_PATH ("http://185.18.214.189:9999/dummyFile")
#define HTTP_LATENCY_PATH ("http://185.18.214.189:9999/emptyDummyFile")
#define HTTP_UPLOAD_PATH ("http://185.18.214.189:9999/upload")



#define BUFF_SIZE (2500)             /// Size of buffer for reading from socket
#define ARRAY_SIZE (30)             /// Size of array to store result

/// protocols types
#define TCP (1)
#define UDP (2)
#define HTTP (3)

/// tests types
#define DOWNLOAD (5)
#define UPLOAD (6)
#define LATENCY (7)

#define PROTOCOL (TCP)              /// protocol type
#define TEST (DOWNLOAD)             /// test type


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
    if (i == WIFI_WAIT_TIME / WIFI_DELAY) {
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
      if (client.available()) {
        result_array[(millis() - st) / 1000] += client.read(buff, BUFF_SIZE);
      }
    }
    Serial.printf("TCP download result:");
    printResultArray();
  } else if (TEST == UPLOAD) {
    while (client.connected()) {
      result_array[(millis() - st) / 1000] += client.write(write_data, BUFF_SIZE);
      client.flush();
    }
    Serial.printf("TCP upload result:");
    printResultArray();
  } else {
    for (int i = 0; client.connected(); i++) {
      while (client.connected() && !client.available());
      client.read(buff, BUFF_SIZE);
      client.write(write_data, 1);
      client.flush();
    }
  }
}

void sendSingleChar() {
  udp.beginPacket(hostIP, HOST_PORT);
  udp.print("$");
  udp.endPacket();
}

void udpTest() {
  /// Initialize udp
  st = millis();
  udp.begin(LOCAL_UDP_PORT);
  sendSingleChar();

  if (TEST == DOWNLOAD) {
    while ((millis() - st) / 1000 < MAX_PERIOD) {
      uint16_t packetSize = udp.parsePacket();
      /// result_array[(millis()-st)/1000] += packetSize;
      if (packetSize)
        result_array[(millis() - st) / 1000] += udp.read(buff, packetSize);
    }
    Serial.printf("UDP download result:");
    printResultArray();
  } else if (TEST == UPLOAD) {
    while ((millis() - st) / 1000 < MAX_PERIOD) {
      udp.beginPacket(hostIP, HOST_PORT);
      udp.print(write_data);
      udp.endPacket();
      delay(1); /// to avoid run time error
    }
  } else {
    for (int i = 0; i < MAX_PACKETS; i++) {
      while (!udp.parsePacket());
      sendSingleChar();
    }
  }
}


//these callbacks will be invoked to read and write data to sdcard
//and process response
//and showing progress
int responsef(uint8_t *buffer, int len) {
  Serial.printf("%s\n", buffer);
  return 0;
}

int rdataf(uint8_t *buffer, int len) {
  //read file to upload
  return len;
}

int wdataf(uint8_t *buffer, int len) {
  //write downloaded data to file
  return len;
  // return root.write(buffer, len);
}

void progressf(int percent) {
  Serial.printf("%d\n", percent);
}

void httpTest() {

  st = millis();
  if (TEST == DOWNLOAD) {
    WiFiClient client;
    HTTPClient http;
    http.begin(client, HTTP_DOWNLOAD_PATH);
    int httpCode = http.GET();
    Serial.println(httpCode);
    int len = http.getSize();
    Serial.println(len);
    WiFiClient* stream = &client;
    while (http.connected() && (len > 0 || len == -1) && (millis() - st) / 1000 <= MAX_PERIOD) {
      int c = stream->readBytes(buff, (size_t)min(len, BUFF_SIZE));
      if (!c) {
        Serial.println("read timeout");
      }
      if (len > 0) {
        len -= c;
        result_array[(millis() - st) / 1000] += c;
      }

    }
    Serial.printf("HTTP download result:");
    printResultArray();
  } else if (TEST == LATENCY) {
    WiFiClient client;
    HTTPClient http;
    st = millis();
    http.begin(client, HTTP_LATENCY_PATH);
    Serial.printf("Result latency: %d\n", millis() - st);
    int httpCode = http.GET();
    Serial.println(httpCode);
  } else {
    Serial.println("Upload HTTP test");
    WiFiClient client;
    while (!client.connect(HOST_IP, HOST_PORT));  // connect to server
    Serial.println("Connected to server");
    client.write("POST /upload HTTP/1.1\r\nHost: 185.18.214.189:9999\r\nUser-Agent: curl/7.68.0\r\nAccept: */*\r\nContent-Length: 512000\r\nContent-Type: multipart/form-data; boundary=------------------------75717ac272b4c37a\r\nExpect: 100-continue\r\n\r\n");
    client.flush();
    client.write("------------------------75717ac272b4c37a\r\n");
    client.write("Content-Disposition: form-data; name=\"files\"; filename=\"108585262.jpeg\"\r\nContent-Type: image/jpeg\r\n\r\n\n\n\n");
    client.flush();
    int i = 0;
    while (client.connected() && (millis() - st) / 1000 <= MAX_PERIOD) {
      result_array[(millis() - st) / 1000] += client.write(write_data, BUFF_SIZE);
      client.flush();
    }
    client.write("\r\n------------------------75717ac272b4c37a\r\n");
    client.flush();
    Serial.printf("HTTP upload result:");
    printResultArray();
  }
}

void setup() {
  ESP.eraseConfig(); /// this makes esp faster
  Serial.begin(9600);
  delay(10); /// waits for Serial to begin
  digitalWrite(BUILT_IN_LED, HIGH); /// turn the LED off.
  for (int i = 0; i < BUFF_SIZE; i++) {
    write_data[i] = '$';
  }
  write_data[BUFF_SIZE - 1] = '\0';
  bool ok = true;
  if (setupWifi()) {
    Serial.println("OK");
    digitalWrite(BUILT_IN_LED, LOW); // turn the LED on.
  }


  if (PROTOCOL == TCP)
    tcp();
  else if (PROTOCOL == UDP)
    udpTest();
  else if (PROTOCOL == HTTP)
    httpTest();
  Serial.println("\nDone");
}
void loop() {
}
