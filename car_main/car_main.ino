#include "esp_camera.h"
#include <WiFi.h>
#include "credentials.h"
#include <WebServer.h>
#include <PubSubClient.h>

// Pin definitions for ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    22
#define XCLK_GPIO_NUM     0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM       5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     1

// LED diagnostic pin (pin 4 for this ESP32-CAM board)
#define LED_PIN 4

// MQTT Configuration
#define MQTT_BROKER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_TOPIC "ajlisy/robotlogs"
#define MQTT_CLIENT_ID "ESP32CAM_Car"

WebServer server(80);
WiFiClient espClient;
PubSubClient mqtt(espClient);

// Function to send log messages via MQTT
void sendMqttLog(const String& message) {
  if (mqtt.connected()) {
    mqtt.publish(MQTT_TOPIC, message.c_str());
    Serial.println("MQTT Log sent: " + message);
  } else {
    Serial.println("MQTT not connected, log not sent: " + message);
  }
}

// Function to connect to MQTT broker
void connectMqtt() {
  while (!mqtt.connected()) {
    Serial.println("Connecting to MQTT broker...");
    sendMqttLog("Connecting to MQTT broker...");
    
    if (mqtt.connect(MQTT_CLIENT_ID)) {
      Serial.println("MQTT connected");
      sendMqttLog("MQTT connected successfully");
    } else {
      Serial.print("MQTT connection failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" retrying in 5 seconds");
      sendMqttLog("MQTT connection failed, retrying in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  // Initialize LED for diagnostics
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Diagnostic blink 1: Setup starting
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println("ESP32-CAM Car Setup Starting...");
  sendMqttLog("ESP32-CAM Car Setup Starting...");

  // Configure MQTT
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  
  // ESP32-CAM has a PS RAM (High Speed PSRAM)
  if(psramFound()){
    Serial.println("PSRAM found - using high quality settings");
    sendMqttLog("PSRAM found - using high quality settings");
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    Serial.println("No PSRAM - using limited settings");
    sendMqttLog("No PSRAM - using limited settings");
    // Limit the frame size when PSRAM is not available
    config.frame_size = FRAMESIZE_SVGA;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  // camera init
  Serial.println("Initializing camera...");
  sendMqttLog("Initializing camera...");
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    String errorMsg = "Camera init failed with error 0x" + String(err, HEX);
    Serial.println(errorMsg);
    sendMqttLog(errorMsg);
    // Diagnostic blink: Camera init failed
    for(int i = 0; i < 5; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
    return;
  }
  
  // Diagnostic blink 2: Camera initialized successfully
  for(int i = 0; i < 2; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(300);
    digitalWrite(LED_PIN, LOW);
    delay(300);
  }
  Serial.println("Camera initialized successfully");
  sendMqttLog("Camera initialized successfully");

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s) {
    Serial.println("Configuring camera sensor settings...");
    sendMqttLog("Configuring camera sensor settings...");
    s->set_brightness(s, 0);     // -2 to 2
    s->set_contrast(s, 0);       // -2 to 2
    s->set_saturation(s, 0);     // -2 to 2
    s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
    s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
    s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
    s->set_aec2(s, 0);           // 0 = disable , 1 = enable
    s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
    s->set_agc_gain(s, 0);       // 0 to 30
    s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
    s->set_bpc(s, 0);            // 0 = disable , 1 = enable
    s->set_wpc(s, 1);            // 0 = disable , 1 = enable
    s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
    s->set_lenc(s, 1);           // 0 = disable , 1 = enable
    s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
    s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
    s->set_dcw(s, 1);            // 0 = disable , 1 = enable
    s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
  }

  // Diagnostic blink 3: Starting WiFi connection
  for(int i = 0; i < 4; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(150);
    digitalWrite(LED_PIN, LOW);
    delay(150);
  }
  Serial.println("Starting WiFi connection...");
  sendMqttLog("Starting WiFi connection...");
  
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    // Send MQTT log every 10 seconds during WiFi connection
    static int wifiAttempts = 0;
    wifiAttempts++;
    if (wifiAttempts % 20 == 0) { // Every 10 seconds (20 * 500ms)
      sendMqttLog("WiFi connection attempt " + String(wifiAttempts/2) + "...");
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  sendMqttLog("WiFi connected");
  
  // Diagnostic blink 4: WiFi connected successfully
  for(int i = 0; i < 2; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
  }
  sendMqttLog("WiFi connected successfully");

  // Connect to MQTT broker
  Serial.println("Connecting to MQTT broker...");
  connectMqtt();

  // Start web server
  server.on("/", handleRoot);
  server.on("/capture", handleCapture);
  server.begin();
  Serial.println("HTTP server started");
  sendMqttLog("HTTP server started");

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
  sendMqttLog("Camera Ready! Use 'http://" + WiFi.localIP().toString() + "' to connect");
  
  // Diagnostic blink 5: Setup complete - continuous slow blink
  Serial.println("Setup complete - LED will now blink slowly to indicate system is running");
  sendMqttLog("Setup complete - LED will now blink slowly to indicate system is running");
}

void handleRoot() {
  String html = "<html><head><title>ESP32-CAM</title></head><body>";
  html += "<h1>ESP32-CAM Camera</h1>";
  html += "<p><a href='/capture'>Take Photo</a></p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleCapture() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }
  
  server.sendHeader("Content-Disposition", "inline; filename=capture.jpg");
  server.send_P(200, "image/jpeg", (const char*)fb->buf, fb->len);
  esp_camera_fb_return(fb);
}

void loop() {
  server.handleClient();
  
  // Continuous slow blink to indicate system is running
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  
  if (millis() - lastBlink >= 2000) {  // Blink every 2 seconds
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    lastBlink = millis();
  }

  // MQTT loop
  if (!mqtt.connected()) {
    connectMqtt();
  }
  mqtt.loop();
  
  // Send periodic status updates via MQTT
  static unsigned long lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate >= 30000) { // Every 30 seconds
    String statusMsg = "System running - Uptime: " + String(millis()/1000) + "s, WiFi: " + 
                      (WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected") + 
                      ", IP: " + WiFi.localIP().toString();
    sendMqttLog(statusMsg);
    lastStatusUpdate = millis();
  }
}
