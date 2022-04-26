
#define SDA_PIN 27
#define SCL_PIN 32
#define POWER_PIN 12
#define POWER_PIN_STATE HIGH 

#include <Wire.h>
#include <MS5611.h>
#include "SparkFun_BNO080_Arduino_Library.h"
#include <kalmanvert.h>
#define volumeDefault 5
#define POSITION_MEASURE_STANDARD_DEVIATION 0.1
#ifdef HAVE_ACCELEROMETER 
#define ACCELERATION_MEASURE_STANDARD_DEVIATION 0.3
#else
#define ACCELERATION_MEASURE_STANDARD_DEVIATION 0.6
#endif //HAVE_ACCELEROMETER 

#define BEEP_FREQ 800

BNO080 myIMU;
MS5611 ms5611;
unsigned long realPressure = 0;
unsigned long alti = 0;
unsigned long lastDisplayTimestamp;
Kalmanvert kalmanvert;

void setup()
{
  Serial.begin(115200);
  delay(5000);
  Serial.println();
  Serial.println("BNO080 Read Example");

  pinMode(POWER_PIN, OUTPUT); 
  digitalWrite(POWER_PIN, POWER_PIN_STATE);     // turn on POWER


  delay(100);
  Wire.flush();
  Wire.begin (SDA_PIN, SCL_PIN);
//  myIMU.begin(0x4A, Wire);

  while(!ms5611.begin(MS5611_ULTRA_HIGH_RES))
  {
    delay(500);
  }
  
  if (myIMU.begin(0x4A, Wire) == false)
  {
    Serial.println("BNO080 not detected at default I2C address. Check your jumpers and the hookup guide. Freezing...");
    while (1);
  }

  Wire.setClock(400000); //Increase I2C data rate to 400kHz

  myIMU.calibrateAll(); //Turn on cal for Accel, Gyro, and Mag
  myIMU.enableLinearAccelerometer(50);
  realPressure = ms5611.readPressure();
  alti = ms5611.getAltitude(realPressure);

   Serial.print(F("Pression : "));
  Serial.print(realPressure);
  Serial.println();
  Serial.print(F("Alti : "));
  Serial.print(alti);
  Serial.println();

    kalmanvert.init(alti,
                  0.0,
                  POSITION_MEASURE_STANDARD_DEVIATION,
                  ACCELERATION_MEASURE_STANDARD_DEVIATION,
                  millis());
                  
}


void loop()
{
  //Look for reports from the IMU
  if (myIMU.dataAvailable() == true)
  {
    float lz = myIMU.getLinAccelZ();
    kalmanvert.update( alti,
                       lz,
                       millis() );

     Serial.print(lz, 2);
      Serial.print(F(","));
       Serial.println();
      Serial.print("Vario : ");
      Serial.println(kalmanvert.getVelocity());
      Serial.println();
      byte linAccuracy = myIMU.getLinAccelAccuracy();
      printAccuracyLevel(linAccuracy);
      Serial.println();
   
    
    
  }
}

//Given a accuracy number, print what it means
void printAccuracyLevel(byte accuracyNumber)
{
  if (accuracyNumber == 0) Serial.print(F("Unreliable"));
  else if (accuracyNumber == 1) Serial.print(F("Low"));
  else if (accuracyNumber == 2) Serial.print(F("Medium"));
  else if (accuracyNumber == 3) Serial.print(F("High"));
}
