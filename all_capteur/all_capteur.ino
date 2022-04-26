#include "I2CWrapper.h"

#include <MS5611.h>

MS5611 ms5611;

#define SDA_PIN 27
#define SCL_PIN 32
#define POWER_PIN 12
#define POWER_PIN_STATE HIGH 

#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h> // Click here to get the library: http://librarymanager/All#SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library

//SFE_MAX1704X lipo; // Defaults to the MAX17043
//SFE_MAX1704X lipo(MAX1704X_MAX17043); // Create a MAX17043
//SFE_MAX1704X lipo(MAX1704X_MAX17044); // Create a MAX17044
SFE_MAX1704X lipo(MAX1704X_MAX17048); // Create a MAX17048
//SFE_MAX1704X lipo(MAX1704X_MAX17049); // Create a MAX17049

double voltage = 0; // Variable to keep track of LiPo voltage
double soc = 0; // Variable to keep track of LiPo state-of-charge (SOC)
bool alert; // Variable to keep track of whether alert has been triggered

//#define BNO_ADAFRUIT   //IMU lib adafruit or lib sparkfun

// Librairie Sparkfun
#include "SparkFun_BNO080_Arduino_Library.h"

BNO080 myIMU;

#define IMU_ADR 0x4A

void setup(void) {
  Serial.begin(115200);

 // Init Alim capteur
  
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, POWER_PIN_STATE);     // turn on POWER

// Init BUS I2C  
  I2CWrapper::init();

// Init Moniteur serie  
  while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens
  Serial.println(F("ALL CAPTEUR"));

// Init MS5611
  // Initialize MS5611 sensor
  // Ultra high resolution: MS5611_ULTRA_HIGH_RES
  // (default) High resolution: MS5611_HIGH_RES
  // Standard: MS5611_STANDARD
  // Low power: MS5611_LOW_POWER
  // Ultra low power: MS5611_ULTRA_LOW_POWER
  while(!ms5611.begin(MS5611_ULTRA_HIGH_RES))
  {
    delay(500);
  }



  Wire.setClock(400000); //Increase I2C data rate to 400kHz


//init gauge 

  // Set up the MAX17043 LiPo fuel gauge:
  if (lipo.begin() == false) // Connect to the MAX17043 using the default wire port
  {
    Serial.println(F("MAX17043 not detected. Please check wiring. Freezing."));
    while (1)
      ;
  }

  // Quick start restarts the MAX17043 in hopes of getting a more accurate
  // guess for the SOC.
  lipo.quickStart();

  // We can set an interrupt to alert when the battery SoC gets too low.
  // We can alert at anywhere between 1% - 32%:
  lipo.setThreshold(20); // Set alert threshold to 20%.  
  setReports();

  Serial.println("Reading events");
  delay(100);
}

// Here is where you define the sensor outputs you want to receive
void setReports(void) {
  Serial.println("Setting desired reports");
	
  lipo.enableDebugging(); // Uncomment this line to enable helpful debug messages on Serial
}


void loop() {
//LECTEUR MPU


//LECTURE TENSION
// lipo.getVoltage() returns a voltage value (e.g. 3.93)
  voltage = lipo.getVoltage();
  // lipo.getSOC() returns the estimated state of charge (e.g. 79%)
  soc = lipo.getSOC();
  // lipo.getAlert() returns a 0 or 1 (0=alert not triggered)
  alert = lipo.getAlert();

  // Print the variables:
  Serial.print("Voltage: ");
  Serial.print(voltage);  // Print the battery voltage
  Serial.println(" V");

  Serial.print("Percentage: ");
  Serial.print(soc); // Print the battery state of charge
  Serial.println(" %");

  Serial.print("Alert: ");
  Serial.println(alert);
  Serial.println();

//LECTURE BARO
  // Read true temperature & Pressure (without compensation)
  double realTemperature = ms5611.readTemperature();
  long realPressure = ms5611.readPressure();
  double realAltitude = ms5611.getAltitude(realPressure);

  // Read true temperature & Pressure (with compensation)
  double realTemperature2 = ms5611.readTemperature(true);
  long realPressure2 = ms5611.readPressure(true);
  double realAltitude2 = ms5611.getAltitude(realPressure2);

  // Output
  Serial.print("Temperature : ");
  Serial.println(realTemperature);
  Serial.print("Temperature compensee :");
  Serial.println(realTemperature2);
  Serial.print("Pression :");
  Serial.println(realPressure);
  Serial.print("Pression compensee :");
  Serial.println(realPressure2);
  Serial.print("Altitude :");
  Serial.println(realAltitude);
  Serial.print("Altitude compensee :");
  Serial.println(realAltitude2);
  Serial.println();
  
  delay(100);
}
