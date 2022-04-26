/*
 * Código de exemplo para FusionSense com Giroscópio e Acelerômetro.
 * Cálculo de Pitch e Roll
 *
 * Biblioteca para MPU-9250/9255 aqui: https://github.com/jeimison3/MPU9250
 *
 * Core para ESP32: https://github.com/stickbreaker/arduino-esp32/tree/patch-2
 */


#include <MPUOrientation.h>
pCompassContext CNTX_KAL = createContext();
pCompassContext CNTX_MAD = createContext();

uint32_t timer; // Calculo de tempo em uS

#include "MPU9250.h"


#include <Wire.h>
#define i2cSDA 25
#define i2cSCL 33
TwoWire i2c;

MPU9250 IMU(i2c,0x69); // Conectada com ADO = 1
int statt;


void setup() {
  Serial.begin(115200);
  while(!Serial) {}

  // start communication with IMU
  statt = IMU.begin(i2cSDA, i2cSCL); // Adaptação do código, pois estou usando uma NodeMCU 1.0
  Serial.print("Iniciando.");

  if (statt < 0) {
    Serial.println("IMU initialization unsuccessful");
    Serial.println("Check IMU wiring or try cycling power");
    Serial.print("Status: ");
    Serial.println(statt);
    while(1) {}
  }

  // setting the accelerometer full scale range to +/-8G
  IMU.setAccelRange(MPU9250::ACCEL_RANGE_8G);
  // setting the gyroscope full scale range to +/-500 deg/s
  IMU.setGyroRange(MPU9250::GYRO_RANGE_500DPS);
  // setting DLPF bandwidth to 20 Hz
  IMU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);
  // setting SRD to 19 for a 50 Hz update rate
  IMU.setSrd(19);

  /*IMU.calibrateAccel();
  IMU.calibrateGyro();
  IMU.calibrateMag();*/

  // Calibragem realizada para o ambiente:


  IMU.setAccelCalX(0.0, 1.0);
  IMU.setAccelCalY(0.0, 1.0);
  IMU.setAccelCalZ(0.0, 1.0);

  //Giroscopio:
  IMU.setGyroBiasX_rads(0.009010);
  IMU.setGyroBiasY_rads(-0.027020);
  IMU.setGyroBiasZ_rads(0.015803);

  // Lembre-se de calibrar o magnetômetro para o seu local.

  IMU.setMagCalX(-5.263671, 1.663117);
  IMU.setMagCalY(-4.459239, 1.180570);
  IMU.setMagCalZ(-68.734070, 0.644467);

  delay(100); // Tempo para sensores estabilizarem

  timer = micros();
}

void printBiases(){

  Serial.print("//    Acelerometro:\n");
  Serial.printf(
    "  IMU.setAccelCalX(%f, %f);\n  IMU.setAccelCalY(%f, %f);\n  IMU.setAccelCalZ(%f, %f);\n",
    IMU.getAccelBiasX_mss(), IMU.getAccelScaleFactorX(),
    IMU.getAccelBiasY_mss(), IMU.getAccelScaleFactorY(),
    IMU.getAccelBiasZ_mss(), IMU.getAccelScaleFactorZ()
  );

  Serial.print("  //Giroscopio:\n");
  Serial.printf(
    "  IMU.setGyroBiasX_rads(%f);\n  IMU.setGyroBiasY_rads(%f);\n  IMU.setGyroBiasZ_rads(%f);\n",
    IMU.getGyroBiasX_rads(),
    IMU.getGyroBiasY_rads(),
    IMU.getGyroBiasZ_rads()
  );


  Serial.print("  //Magnetometro:\n");
  Serial.printf(
    "  IMU.setMagCalX(%f, %f);\n  IMU.setMagCalY(%f, %f);\n  IMU.setMagCalZ(%f, %f);\n",
    IMU.getMagBiasX_uT(), IMU.getMagScaleFactorX(),
    IMU.getMagBiasY_uT(), IMU.getMagScaleFactorY(),
    IMU.getMagBiasZ_uT(), IMU.getMagScaleFactorZ()
  );

  Serial.println("Retomando em 10 segundos.");

  delay(10000);
}

uint32_t cont = 0;

void loop() {

  double dt = (double)(micros() - timer) / 1000000; // Calculate delta time
  timer = micros();

  if(Serial.available()){
    int v = Serial.parseInt();
    if(v == 1){
      printBiases();
    }
    else if (v == 10){
      IMU.calibrateMag();
    }
    else if (v == 11){
      IMU.calibrateGyro();
    }
  }
  // read the sensor
  IMU.readSensor();

  // Uso de MPUFusion:

  IMUFullFusion SENS;
  SENS.ACCEL.x = IMU.getAccelX_mss();
  SENS.ACCEL.y = IMU.getAccelY_mss();
  SENS.ACCEL.z = IMU.getAccelZ_mss();

  SENS.GYRO.x = IMU.getGyroX_rads();
  SENS.GYRO.y = IMU.getGyroY_rads();
  SENS.GYRO.z = IMU.getGyroZ_rads();

  SENS.MAG.x = IMU.getMagX_uT();
  SENS.MAG.y = IMU.getMagY_uT();
  SENS.MAG.z = IMU.getMagZ_uT();

  IMUOrientation oriKal = getFullOrientation(CNTX_KAL, ALGO_KALMAN_V1, SENS, dt);
  IMUOrientation oriMad = getFullOrientation(CNTX_MAD, ALGO_MADGWICK_V1, SENS, dt);


  if(cont++ % 30 == 29){
    Serial.printf( "\t%f\t%f\t\t%f\t%f\t\t%f\t%f\n", oriKal.pitch, oriMad.pitch, oriKal.roll, oriMad.roll, oriKal.yaw, oriMad.yaw );
    cont = 0;
  }


  delay(2);

}
