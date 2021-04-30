// FliesBin
// Test WiFi connection and weight logging

// Include Load Cell Amp Library
#include "HX711.h"
// Include Ultrasonic Sensor Library
#include <HCSR04.h>
// Include WiFi library
#include <WiFi.h>
#include <WiFiClient.h>
// Include Blynk library
#include <BlynkSimpleEsp32.h>

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 25;
const int LOADCELL_SCK_PIN = 26;
HX711 scale;                      // Init HX711

// HCSR04 circuit wiring
const int TRIG_PIN = 4;
const int ECHO_PIN = 5;
HCSR04 hc(TRIG_PIN, ECHO_PIN);    // Init HCSR04

// WiFi Identification
const char ssid[] = "OPPO F7 Youth";
const char pass[] = "piranha44";

// Blynk auth token
char auth[] = "88pR-FEtTFiuJ9pA8dJWNdlLbIMymGAx";

// Function to connect to WiFi

// Use Virtual pin 5 for uptime display
#define PIN_WEIGHT V5
#define PIN_DIST V4

// This function tells Arduino what to do if there is a Widget
// which is requesting data for Virtual Pin (5)
BLYNK_READ(PIN_WEIGHT)
{
  // This command writes Arduino's uptime in seconds to Virtual Pin (5)
  Blynk.virtualWrite(PIN_WEIGHT, scale.get_units());
  Blynk.virtualWrite(PIN_DIST, hc.dist());
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("");
  Serial.println(WiFi.localIP());
}

// Function to check available WiFi
void ScanWifi() {
  Serial.println("Scanning start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
      Serial.println("No networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      delay(10);
    }
  }
  Serial.println("");
}


void setup() {
  // Begin serial communication
  Serial.begin(115200);
  // Scanning for available network
  ScanWifi();
  // Initializing wifi
  initWiFi();
  Serial.println("Setup done");
  // Weight Sensor Demo
  Serial.println("HX711 Demo");

  Serial.println("Initializing the scale");

  // Initialize library with data output pin, clock input pin and gain factor.
  // Channel selection is made by passing the appropriate gain:
  // - With a gain factor of 64 or 128, channel A is selected
  // - With a gain factor of 32, channel B is selected
  // By omitting the gain factor parameter, the library
  // default "128" (Channel A) is used here.
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());               // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));     // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));         // print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);      // print the average of 5 readings from the ADC minus tare weight (not set) divided
            // by the SCAL0000000hdzE parameter (not set yet)

  scale.set_scale(2280.f);                    // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();                               // reset the scale to 0

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());               // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));     // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));         // print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);      // print the average of 5 readings from the ADC minus tare weight, divided
            // by the SCALE parameter set with set_scale

  Serial.println("Readings:");
  // Begin Blynk
  Blynk.begin(auth, ssid, pass);
}

void loop() {
  /* // Uncomment if you want to read weight in serial
  Serial.print("one reading:\t");
  Serial.print(scale.get_units(), 1);
  Serial.print("\t| average:\t");
  Serial.println(scale.get_units(10), 1);

  scale.power_down();                           // put the ADC in sleep mode
  delay(5000);
  scale.power_up(); */
  /* // Uncomment if you want to read dist in serial
  Serial.println(hc.dist()); */
  Blynk.run();
}
