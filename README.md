# FliesBin

Smart IoT waste bin.

## FliesBin v1 
Sensors: 
1. Load Sensor + HX711
  Per | ESP32
  Vcc = 3V3 
  Dat = D25
  Clk = D26
  Gnd = Gnd
  
3. HC-SR04
  Per | ESP32
  Vcc = Vin 
  Trig = D4
  Echo = D5
  Gnd = Gnd
  
## FliesBin v2
Sensors:
1. Load Sensor + HX711
2. HC-SR04
3. DHT11
  Per | ESP32
  Vcc = 3-5 V
  Gnd = Gnd
  Dat = D15

Actuator:
1. Fan
  Fan | Per
  Vpp = OUT1
  Vnn = OUT2
2. L298N
  Per | ESP32
  Gnd = Gnd 
  IN1 = D33
  IN2 = D32
  EN1 = D35 (optional) s
