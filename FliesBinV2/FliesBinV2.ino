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
// Include DHT11 library
#include "DHTesp.h" // Click here to get the library: http://librarymanager/All#DHTesp
#include <Ticker.h>
// Define ESP board
#ifndef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP32 ONLY!)
#error Select ESP32 board.
#endif

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 5;
const int LOADCELL_SCK_PIN = 4;
HX711 scale;                      // Init HX711

// HCSR04 circuit wiring
const int TRIG_PIN = 26;
const int ECHO_PIN = 25;
HCSR04 hc(TRIG_PIN, ECHO_PIN);    // Init HCSR04

// DHT Init
DHTesp dht;

// WiFi Identification
const char ssid[] = "OPPO F7 Youth";
const char pass[] = "piranha44";

// Blynk auth token
char auth[] = "88pR-FEtTFiuJ9pA8dJWNdlLbIMymGAx";

// Function to connect to WiFi

// Use Virtual pin 5 for uptime display
#define PIN_WEIGHT V5
#define PIN_DIST V4
#define PIN_TEMP V6
#define PIN_HUMD V7

// This function tells Arduino what to do if there is a Widget
// which is requesting data for Virtual Pin (5)
BLYNK_READ(PIN_WEIGHT)
{
  // This command writes Arduino's uptime in seconds to Virtual Pin (5)
  Blynk.virtualWrite(PIN_WEIGHT, scale.get_units());
  Blynk.virtualWrite(PIN_DIST, hc.dist());
  Blynk.virtualWrite(PIN_TEMP, dht.getTempAndHumidity().temperature);
  Blynk.virtualWrite(PIN_HUMD, dht.getTempAndHumidity().humidity);
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

void tempTask(void *pvParameters);
bool getTemperature();
void triggerGetTemp();

/** Task handle for the light value read task */
TaskHandle_t tempTaskHandle = NULL;
/** Ticker for temperature reading */
Ticker tempTicker;
/** Comfort profile */
ComfortState cf;
/** Flag if task should run */
bool tasksEnabled = false;
/** Pin number for DHT11 data pin */
int dhtPin = 15;

/**
 * initTemp
 * Setup DHT library
 * Setup task and timer for repeated measurement
 * @return bool
 *    true if task and timer are started
 *    false if task or timer couldn't be started
 */
bool initTemp() {
  byte resultValue = 0;
  // Initialize temperature sensor
  dht.setup(dhtPin, DHTesp::DHT11);
  Serial.println("DHT initiated");

  // Start task to get temperature
  xTaskCreatePinnedToCore(
      tempTask,                       /* Function to implement the task */
      "tempTask ",                    /* Name of the task */
      4000,                           /* Stack size in words */
      NULL,                           /* Task input parameter */
      5,                              /* Priority of the task */
      &tempTaskHandle,                /* Task handle. */
      1);                             /* Core where the task should run */

  if (tempTaskHandle == NULL) {
    Serial.println("Failed to start task for temperature update");
    return false;
  } else {
    // Start update of environment data every 20 seconds
    tempTicker.attach(20, triggerGetTemp);
  }
  return true;
}

/**
 * triggerGetTemp
 * Sets flag dhtUpdated to true for handling in loop()
 * called by Ticker getTempTimer
 */
void triggerGetTemp() {
  if (tempTaskHandle != NULL) {
     xTaskResumeFromISR(tempTaskHandle);
  }
}

/**
 * Task to reads temperature from DHT11 sensor
 * @param pvParameters
 *    pointer to task parameters
 */
void tempTask(void *pvParameters) {
  Serial.println("tempTask loop started");
  while (1) // tempTask loop
  {
    if (tasksEnabled) {
      // Get temperature values
      getTemperature();
    }
    // Got sleep again
    vTaskSuspend(NULL);
  }
}

/**
 * getTemperature
 * Reads temperature from DHT11 sensor
 * @return bool
 *    true if temperature could be aquired
 *    false if aquisition failed
*/
bool getTemperature() {
  // Reading temperature for humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  TempAndHumidity newValues = dht.getTempAndHumidity();
  // Check if any reads failed and exit early (to try again).
  if (dht.getStatus() != 0) {
    Serial.println("DHT11 error status: " + String(dht.getStatusString()));
    return false;
  }

  float heatIndex = dht.computeHeatIndex(newValues.temperature, newValues.humidity);
  float dewPoint = dht.computeDewPoint(newValues.temperature, newValues.humidity);
  float cr = dht.getComfortRatio(cf, newValues.temperature, newValues.humidity);

  String comfortStatus;
  switch(cf) {
    case Comfort_OK:
      comfortStatus = "Comfort_OK";
      break;
    case Comfort_TooHot:
      comfortStatus = "Comfort_TooHot";
      break;
    case Comfort_TooCold:
      comfortStatus = "Comfort_TooCold";
      break;
    case Comfort_TooDry:
      comfortStatus = "Comfort_TooDry";
      break;
    case Comfort_TooHumid:
      comfortStatus = "Comfort_TooHumid";
      break;
    case Comfort_HotAndHumid:
      comfortStatus = "Comfort_HotAndHumid";
      break;
    case Comfort_HotAndDry:
      comfortStatus = "Comfort_HotAndDry";
      break;
    case Comfort_ColdAndHumid:
      comfortStatus = "Comfort_ColdAndHumid";
      break;
    case Comfort_ColdAndDry:
      comfortStatus = "Comfort_ColdAndDry";
      break;
    default:
      comfortStatus = "Unknown:";
      break;
  };

  Serial.println(" T:" + String(newValues.temperature) + " H:" + String(newValues.humidity) + " I:" + String(heatIndex) + " D:" + String(dewPoint) + " " + comfortStatus);
  return true;
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

  // Serial.println("Readings:");
  Serial.println();
  Serial.println("DHT ESP32 example with tasks");
  initTemp();
  // Signal end of setup() to tasks
  tasksEnabled = true;
  // Begin Blynk
  Blynk.begin(auth, ssid, pass);
}

void loop() {
  /* 
   *  // Uncomment if you want to read weight in serial
  Serial.print("one reading:\t");
  Serial.print(scale.get_units(), 1);
  Serial.print("\t| average:\t");
  Serial.println(scale.get_units(10), 1);

  scale.power_down();                           // put the ADC in sleep mode
  delay(5000);
  scale.power_up(); */
  /* 
   *  // Uncomment if you want to read dist in serial
  Serial.println(hc.dist()); 
  */
  
  if (!tasksEnabled) {
    // Wait 2 seconds to let system settle down
    delay(2000);
    // Enable task that will read values from the DHT sensor
    tasksEnabled = true;
    if (tempTaskHandle != NULL) {
      vTaskResume(tempTaskHandle);
    }
  }
  yield();
  
  Blynk.run();
}
