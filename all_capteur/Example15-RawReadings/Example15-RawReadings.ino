/*
  Using the BNO080 IMU
  By: Nathan Seidle
  SparkFun Electronics
  Date: December 21st, 2017
  SparkFun code, firmware, and software is released under the MIT License.
	Please see LICENSE.md for further details.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14586

  This example shows how to output accelerometer values

  Hardware Connections:
  Attach the Qwiic Shield to your Arduino/Photon/ESP32 or other
  Plug the sensor onto the shield
  Serial.print it out at 115200 baud to serial monitor.
*/

#define SDA_PIN 27
#define SCL_PIN 32
#define POWER_PIN 12
#define POWER_PIN_STATE HIGH 

#include <Wire.h>

#include "SparkFun_BNO080_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_BNO080
BNO080 myIMU;

#include <MPUOrientation.h>
pCompassContext CNTX = createContext(); // Cria contexto de variaveis
uint32_t timer; // Calculo de tempo em uS

void setup()
{
  Serial.begin(115200);
  delay(5000);

  Serial.println();
  Serial.println("BNO080 Read Example");

  pinMode(POWER_PIN, OUTPUT); 
  digitalWrite(POWER_PIN, POWER_PIN_STATE);     // turn on POWER

//    Wire.begin();

  delay(100);
  Wire.flush();
  Wire.begin (SDA_PIN, SCL_PIN);

//    myIMU.begin();

  if (myIMU.begin(0x4A, Wire) == false)
  {
    Serial.println("BNO080 not detected at default I2C address. Check your jumpers and the hookup guide. Freezing...");
    while (1);
  }

  Wire.setClock(400000); //Increase I2C data rate to 400kHz

  myIMU.enableAccelerometer(50);    //We must enable the accel in order to get MEMS readings even if we don't read the reports.
  myIMU.enableRawAccelerometer(50); //Send data update every 50ms
  myIMU.enableGyro(50);
  myIMU.enableRawGyro(50);
  myIMU.enableMagnetometer(50);
  myIMU.enableRawMagnetometer(50);

  Serial.println(F("Raw MEMS readings enabled"));
  Serial.println(F("Output is: (accel) x y z (gyro) x y z (mag) x y z"));
  delay(2000);

  timer = micros();
}

uint32_t cont = 0;

void loop()
{

  double dt = (double)(micros() - timer) / 1000000; // Calculate delta time
  timer = micros();
 
  //Look for reports from the IMU
  if (myIMU.dataAvailable() == true)
  {
    int x = myIMU.getRawAccelX();
    int y = myIMU.getRawAccelY();
    int z = myIMU.getRawAccelZ();

    Serial.print(x);
    Serial.print("\t");
    Serial.print(y);
    Serial.print("\t");
    Serial.print(z);
    Serial.print("\t");

    int gx = myIMU.getRawGyroX();
    int gy = myIMU.getRawGyroY();
    int gz = myIMU.getRawGyroZ();

    Serial.print(gx);
    Serial.print("\t");
    Serial.print(gy);
    Serial.print("\t");
    Serial.print(gz);
    Serial.print("\t");

    int mx = myIMU.getRawMagX();
    int my = myIMU.getRawMagY();
    int mz = myIMU.getRawMagZ();

    Serial.print(mx);
    Serial.print("\t");
    Serial.print(my);
    Serial.print("\t");
    Serial.print(mz);
    Serial.print("\t");

    Serial.println();

    IMUFullFusion SENS;
    SENS.ACCEL.x = x;
    SENS.ACCEL.y = y;
    SENS.ACCEL.z = z;
  
    SENS.GYRO.x = gx;
    SENS.GYRO.y = gy;
    SENS.GYRO.z = gz;
  
    SENS.MAG.x = mx;
    SENS.MAG.y = my;
    SENS.MAG.z = mz;
  
    IMUOrientation ori = getFullOrientation(CNTX, ALGO_KALMAN_V1, SENS, dt);
  
    if(cont++ % 30 == 29){ // 125 a intervalo de 2ms equivalente a 0.25 seg
      Serial.printf( "\t%f\t%f\t%f\n", ori.pitch, ori.roll, ori.yaw );
      cont = 0;
    }  
  } 
  delay(2);  
}
