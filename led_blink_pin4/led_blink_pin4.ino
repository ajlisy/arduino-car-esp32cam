// ESP32-CAM LED Blink using correct pin 4
// This sketch will continuously blink the LED

#define LED_PIN 4  // Correct LED pin for this ESP32-CAM board

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("ESP32-CAM LED Blink Starting...");
  Serial.println("Using LED pin 4");
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("LED initialized on pin 4");
}

void loop() {
  Serial.println("LED ON");
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  
  Serial.println("LED OFF");
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}

