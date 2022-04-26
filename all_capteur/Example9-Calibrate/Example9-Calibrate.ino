
/*
  Using the BNO080 IMU
  By: Nathan Seidle
  SparkFun Electronics
  Date: December 21st, 2017
  SparkFun code, firmware, and software is released under the MIT License.
	Please see LICENSE.md for further details.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14586

  This example shows how to calibrate the sensor. See document 1000-4044.

  It takes about 1ms at 400kHz I2C to read a record from the sensor, but we are polling the sensor continually
  between updates from the sensor. Use the interrupt pin on the BNO080 breakout to avoid polling.

  Hardware Connections:
  Attach the Qwiic Shield to your Arduino/Photon/ESP32 or other
  Plug the sensor onto the shield
  Serial.print it out at 115200 baud to serial monitor.
*/

#include <Wire.h>

#define SDA_PIN 27
#define SCL_PIN 32
#define POWER_PIN 12
#define POWER_PIN_STATE HIGH 

#include <MS5611.h>


MS5611 ms5611;

#include "SparkFun_BNO080_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_BNO080
BNO080 myIMU;

float quat[4];

void setup()
{
  Serial.begin(115200);

  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, POWER_PIN_STATE);     // turn on POWER
   
  Serial.println();
  Serial.println("BNO080 Read Example");

//  Wire.begin();

// Init BUS I2C  
  Wire.flush();
  Wire.begin (SDA_PIN, SCL_PIN);

// Init Moniteur serie  
  while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens

//  myIMU.begin();

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

  //Enable dynamic calibration for accel, gyro, and mag
  myIMU.calibrateAll(); //Turn on cal for Accel, Gyro, and Mag

  myIMU.enableLinearAccelerometer(100);
  //Enable Game Rotation Vector output
  myIMU.enableGameRotationVector(100); //Send data update every 100ms

  myIMU.enableAccelerometer(100);
  myIMU.enableMagnetometer(100);
  
  //Once magnetic field is 2 or 3, run the Save DCD Now command
  Serial.println(F("Calibrating. Press 's' to save to flash"));
  Serial.println(F("Output in form x, y, z, in uTesla"));
}

void loop()
{
  if(Serial.available())
  {
    byte incoming = Serial.read();

    if(incoming == 's')
    {
      myIMU.saveCalibration(); //Saves the current dynamic calibration data (DCD) to memory
      myIMU.requestCalibrationStatus(); //Sends command to get the latest calibration status

      //Wait for calibration response, timeout if no response
      int counter = 100;
      while(1)
      {
        if(--counter == 0) break;
        if(myIMU.dataAvailable() == true)
        {
          //The IMU can report many different things. We must wait
          //for the ME Calibration Response Status byte to go to zero
          if(myIMU.calibrationComplete() == true)
          {
            Serial.println("Calibration data successfully stored");
            delay(1000);
            break;
          }
        }

        delay(1);
      }
    }
  }

//Look for reports from the IMU
  if (myIMU.dataAvailable() == true)
  {
    
    
      /* Serial.print(cumulativeTime);
      Serial.print(F(","));
      float x = myIMU.getMagX();
      float y = myIMU.getMagY();
      float z = myIMU.getMagZ();
      byte accuracy = myIMU.getMagAccuracy();
  
      float quatI = myIMU.getQuatI();
      float quatJ = myIMU.getQuatJ();
      float quatK = myIMU.getQuatK();
      float quatReal = myIMU.getQuatReal();
      byte sensorAccuracy = myIMU.getQuatAccuracy();
  
      Serial.print(x, 2);
      Serial.print(F(","));
      Serial.print(y, 2);
      Serial.print(F(","));
      Serial.print(z, 2);
      Serial.print(F(","));
      printAccuracyLevel(accuracy);
      Serial.print(F(","));
  
      Serial.print("\t");
      */
      float lx = myIMU.getAccelX();
      float ly = myIMU.getAccelY();
      float lz = myIMU.getAccelZ();
      //byte linAccuracy = myIMU.getLinAccelAccuracy();
  
		
  
  
  
  
  
      Serial.print(lx, 2);
      Serial.print(F(","));
      Serial.print(ly, 2);
      Serial.print(F(","));
      Serial.print(lz, 2);
      //Serial.print(F(","));
      /*printAccuracyLevel(linAccuracy);*/
      Serial.println();

    
       


    
  }
}

float imu_GravityCompensatedAccel(float ax, float ay, float az, volatile float* q)
{
  float za;
  za = 2.0*(q[1]*q[3] - q[0]*q[2])*ax + 2.0*(q[0]*q[1] + q[2]*q[3])*ay + (q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3])*az - 1.0;

  return (abs(za) < 0.1) ? 0 : za * 9.81f;
}

//Given a accuracy number, print what it means
void printAccuracyLevel(byte accuracyNumber)
{
  if (accuracyNumber == 0) Serial.print(F("Unreliable"));
  else if (accuracyNumber == 1) Serial.print(F("Low"));
  else if (accuracyNumber == 2) Serial.print(F("Medium"));
  else if (accuracyNumber == 3) Serial.print(F("High"));
}
