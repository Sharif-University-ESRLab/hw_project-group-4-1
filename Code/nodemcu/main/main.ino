#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h> /// for UDP socket

#define BUILT_IN_LED (2) /// GPID of built in led.

#define WIFI_INIT_READY_DALY (10000) /// Wait time for connecting to wifi.
#define WIFI_CONN_CHK_INTERVAL (500) /// Interval for checking wifi connection.
#define WIFI_CONN_TRY_COUNT (20)     /// Number of tries to connect to wifi.
#define WIFI_SSID ("Redmi 8A")       /// Hotspot SSID.
#define WIFI_PASS ("Aa123QWE")       /// Hotspot Password.

#define LOCAL_UDP_PORT (10210) /// Local port to listen on.
#define CONN_TIME_OUT (15)     /// Maximum time each test would take.
#define UDP_DELAY_TIME 10      /// time between two UDP packet transfer.
#define MAX_PACKETS (10)       /// Maximum number of packet sent/received.

IPAddress hostIP(185, 18, 214, 189); /// Server IP.
#define HOST_IP ("185.18.214.189")   /// Server IP.
#define HOST_PORT (9999)             /// Server port.

#define HTTP_URL ("http://185.18.214.189:9999")
#define HTTP_DOWNLOAD_PATH ("http://185.18.214.189:9999/dummyFile")
#define HTTP_LATENCY_PATH ("http://185.18.214.189:9999/emptyDummyFile")
#define HTTP_UPLOAD_PATH ("http://185.18.214.189:9999/upload")

#define BUFF_SIZE (512) /// Size of buffer for reading from socket
#define TEST_SIZE (30)  /// Number of samples

/// Protocols types
enum Protocol {
  TCP = 1,
  UDP = 2,
  HTTP = 3,
};

/// Tests types
enum TestType {
  DOWNLOAD = 1,
  UPLOAD = 2,
  LATENCY = 3,
};

int PROTOCOL = -1;
int TEST = -1;

int result_array[TEST_SIZE];

/// Connect to wifi
bool setup_wifi() {
  Serial.printf("connecting to %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait for the Wi-Fi to connect
  int tries_left = WIFI_CONN_TRY_COUNT;
  while (WiFi.status() != WL_CONNECTED && tries_left) {
    delay(WIFI_CONN_CHK_INTERVAL);
    Serial.printf("retrying [%d]\n", WIFI_CONN_TRY_COUNT - tries_left);
    tries_left--;
  }
  // Didn't make the connection.
  if (!tries_left) {
    Serial.println("Failed to connect");
    WiFi.disconnect();
    return false;
  }

  // Configure wifi connection.
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  return true;
}

void printResultArray() {
  for (int i = 0; i < TEST_SIZE; i++) {
    Serial.printf("%d, ", result_array[i]);
  }
}

void tcp_test() {

  char upload_buffer[BUFF_SIZE];
  char download_buffer[BUFF_SIZE];
  ;

  // Fill upload_buffer with random data.
  generate_upload_data(upload_buffer, BUFF_SIZE);

  // Capture start time
  unsigned long start_time_ms = millis();

  /// Connecting to server
  WiFiClient client;
  while (!client.connect(HOST_IP, HOST_PORT))
    ;
  Serial.println("Connected to server");

  switch (TEST) {
  /// In order to test TCP connection in Download mode
  /// we simply need to wait for new packet and read
  /// all the bytes, sent by the server.
  /// We measure the number of bytes read in each second
  /// and report the result in a maximum range of 30 seconds.
  /// Note: server might close the connection before 30 seconds
  /// and thus, some indices in the resulting array might be zero.
  case DOWNLOAD:
    while (client.connected()) {
      while (client.connected() && !client.available())
        ;
      int bytes_read = client.read(download_buffer, BUFF_SIZE);
      unsigned long end_time_ms = millis();
      result_array[(end_time_ms - start_time_ms) / 1000] += bytes_read;
    }
    Serial.printf("TCP download result:");
    printResultArray();
    break;
  /// Similar to the DOWNLOAD case, all we have to do is to
  /// send buffers of loaded data through an already established
  /// TCP connection and measure the timing. It is also worthy to
  /// mention the `client.flush()` function which grantees that
  /// each call to `client.write()` would result in a Network IO
  /// and thus, grantees the sanity of our experiment.
  case UPLOAD:
    while (client.connected()) {
      int bytes_read = client.write(upload_buffer, BUFF_SIZE);
      client.flush();
      unsigned long end_time_ms = millis();
      result_array[(end_time_ms - start_time_ms) / 1000] += bytes_read;
    }
    Serial.printf("TCP upload result:");
    printResultArray();
    break;
  /// For testing the LATENCY, as long as we have the hold of the
  /// connection, we repeat sending, and receiving packets and
  /// measure the RTT for its latency.
  case LATENCY:
    while (client.connected()) {
      while (client.connected() && !client.available())
        ;
      client.read(download_buffer, BUFF_SIZE);
      client.write(upload_buffer, 1);
      client.flush();
    }
    break;
  default:
    break;
  }
}

/// Sends a single '$' character trough the UDP handle
/// passed down to the function.
void udp_send_single_char(WiFiUDP *udp) {
  udp->beginPacket(hostIP, HOST_PORT);
  udp->print("$");
  udp->endPacket();
}

/// Sends the provided buffer as a UDP packet via the
/// UDP handle passed down to the function.
void udp_send_packet(WiFiUDP *udp, char *buffer) {
  udp->beginPacket(hostIP, HOST_PORT);
  udp->print(buffer);
  udp->endPacket();
}

void udp_test() {
  char *upload_buffer = (char *)malloc(BUFF_SIZE);
  char *download_buffer = (char *)malloc(BUFF_SIZE);

  // Fill upload_buffer with random data.
  generate_upload_data(upload_buffer, BUFF_SIZE);

  // Handler for UDP connection.
  WiFiUDP udp;

  // Capture start time
  unsigned long start_time_ms = millis();

  // Create UDP socket from server.
  udp.begin(LOCAL_UDP_PORT);

  // Sign server that we are ready to start the experiment.
  // we do this by sending a single $ character.
  udp_send_single_char(&udp);

  switch (TEST) {
  /// In this case, we send as much packet as we can in a
  /// CONN_TIME_OUT window and store the amount of bytes
  /// transfered in each second in the result_array for
  /// further analyzes.
  case DOWNLOAD:
    while ((millis() - start_time_ms) / 1000 < CONN_TIME_OUT) {

      uint16_t packetSize = udp.parsePacket();
      unsigned long end_time_ms = millis();

      if (packetSize) {
        int bytes_read =
            udp.read(download_buffer, min(packetSize, (uint16_t)BUFF_SIZE));
        result_array[(end_time_ms - start_time_ms) / 1000] += bytes_read;
      }
    }
    Serial.printf("UDP download result:");
    printResultArray();
    break;

  /// Unlike TCP, we can not meassure transfered bytes in
  /// the client side of the connection. This is due to the
  /// fact that UDP packet loss is not trackable from client
  /// side of the connection. So what we do instead is that
  /// we simple send the packets as much as we can and trust
  /// the server with the calculation and analytics, since
  /// server has all the data it needs.
  case UPLOAD:
    while ((millis() - start_time_ms) / 1000 < CONN_TIME_OUT) {
      udp_send_packet(&udp, upload_buffer);
      delay(UDP_DELAY_TIME); /// to avoid run time error
    }
    break;
  /// Similar to the UPLOAD case, we can only send MAX_PACKETS
  /// number of packets and leave all the analytics to the server
  /// side code.
  case LATENCY:
    for (int i = 0; i < MAX_PACKETS; i++) {
      while (!udp.parsePacket())
        ;
      udp_send_single_char(&udp);
    }
    break;
  default:
    break;
  }

  // clean up
  free(upload_buffer);
  free(download_buffer);
}

void http_test() {
  char *upload_buffer = (char *)malloc(BUFF_SIZE);
  char *download_buffer = (char *)malloc(BUFF_SIZE);

  // Fill upload_buffer with random data.
  generate_upload_data(upload_buffer, BUFF_SIZE);

  unsigned long start_time_ms = millis();
  switch (TEST) {
  case DOWNLOAD: {
    WiFiClient client;
    HTTPClient http;
    http.begin(client, HTTP_DOWNLOAD_PATH);
    int http_code = http.GET();
    Serial.println(http_code);
    int len = http.getSize();
    Serial.println(len);
    WiFiClient *stream = &client;
    while (http.connected() && (len > 0 || len == -1) &&
           (millis() - start_time_ms) / 1000 <= CONN_TIME_OUT) {
      int c = stream->readBytes(download_buffer, (size_t)min(len, BUFF_SIZE));
      if (!c) {
        Serial.println("read timeout");
      }
      if (len > 0) {
        len -= c;
        result_array[(millis() - start_time_ms) / 1000] += c;
      }
    }
    Serial.printf("HTTP download result:");
    printResultArray();

  } break;
  case UPLOAD: {
    Serial.println("Upload HTTP test");
    WiFiClient client;
    while (!client.connect(HOST_IP, HOST_PORT))
      ; // connect to server
    Serial.println("Connected to server");
    client.write(
        "POST /upload HTTP/1.1\r\nHost: 185.18.214.189:9999\r\nUser-Agent: "
        "curl/7.68.0\r\nAccept: */*\r\nContent-Length: 512000\r\nContent-Type: "
        "multipart/form-data; "
        "boundary=------------------------75717ac272b4c37a\r\nExpect: "
        "100-continue\r\n\r\n");
    client.flush();
    client.write("------------------------75717ac272b4c37a\r\n");
    client.write("Content-Disposition: form-data; name=\"files\"; "
                 "filename=\"108585262.jpeg\"\r\nContent-Type: "
                 "image/jpeg\r\n\r\n\n\n\n");
    client.flush();
    int i = 0;
    while (client.connected() &&
           (millis() - start_time_ms) / 1000 <= CONN_TIME_OUT) {
      result_array[(millis() - start_time_ms) / 1000] +=
          client.write(upload_buffer, BUFF_SIZE);
      client.flush();
    }
    client.write("\r\n------------------------75717ac272b4c37a\r\n");
    client.flush();
    Serial.printf("HTTP upload result:");
    printResultArray();

  } break;
  case LATENCY:
    for (int i = 0; i < 10; i++) {
      WiFiClient client;
      HTTPClient http;
      start_time_ms = millis();
      http.begin(client, HTTP_LATENCY_PATH);
      int http_code = http.GET();
      result_array[i] = millis() - start_time_ms;
      Serial.printf("Result latency: %d\n", millis() - start_time_ms);

      Serial.println(http_code);
    }
    printResultArray();
    break;
  default:
    break;
  }

  // clean up
  free(upload_buffer);
  free(download_buffer);
}

/// Read 1-digit integer from input
int next() {
  while (true) {
    if (Serial.available()) {
      int res = Serial.read();
      if ('0' <= res && res <= '9')
        return res - '0';
    }
  }
}

void generate_upload_data(char *buffer, int buffer_len) {

  memset(buffer, '$', buffer_len - 2);
  // NULL-Terminate the buffer.
  buffer[buffer_len - 1] = '\0';
}

void setup() {
  // Make esp work Faster.
  ESP.eraseConfig();

  // Starting Serial input for user comminiucation.
  Serial.begin(9600);
  // Waits for Serial to begin.
  delay(10);

  // Turn the LED off.
  pinMode(BUILT_IN_LED, OUTPUT);
  digitalWrite(BUILT_IN_LED, HIGH);

  if (setup_wifi()) {
    Serial.println("Wifi connection is setup!");
    // Turn the LED on.
    // Show that the System is up and running.
    digitalWrite(BUILT_IN_LED, LOW);
  }
}
void show_menu() {
  Serial.println("1: TCP");
  Serial.println("2: UDP");
  Serial.println("3: HTTP");
  while (Serial.available() == 0)
    ;
  PROTOCOL = next();
  Serial.println("1: Download");
  Serial.println("2: Upload");
  Serial.println("3: Latency");
  while (Serial.available() == 0)
    ;
  TEST = next();
  Serial.printf("P: %d T: %d\n", PROTOCOL, TEST);
}
void loop() {
  memset(result_array, 0, TEST_SIZE);
  show_menu();

  switch (PROTOCOL) {
  case TCP:
    tcp_test();
    break;
  case UDP:
    udp_test();
    break;
  case HTTP:
    http_test();
    break;
  default:
    Serial.println("protocol not supported! pleas try again");
    break;
  }
  Serial.println("\nDone");
}
