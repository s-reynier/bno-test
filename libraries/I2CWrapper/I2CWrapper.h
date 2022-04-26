#ifndef I2C_WRAPPER_H
#define I2C_WRAPPER_H

#include <DebugConfig.h>
#include <HardwareConfig.h>

class I2CWrapper {
    
    public:
    static void init(void);
    #ifdef HAVE_BMP280
    static Bmp280 bmp280;
    #else  
    static Ms5611 ms5611;
    #endif
    
    #ifdef HAVE_ACCELEROMETER
    static Vertaccel vertaccel;
    #endif

    /* init both devices but not the TW bus */
    static void init(void);
        
    void disableAcquisition(void);

    /* barometer part */
    static bool havePressure(void);
    static double getAlti(void);	
    static void getTempAlti(double& temp, double& alti);
    
    /* IMU part */
    /* tap callback is triggered by getRawAccel and getAccel */
    static bool haveAccel(void);
    static bool haveNewAccel(void);
    static bool resetNewAccel(void);
    static void getRawAccel(int16_t* rawAccel, int32_t* quat);
    static double getAccel(double* vertVector); //vertVector = NULL if not needed
        
    static bool haveGyro(void);
    static bool haveNewGyro(void);
    static bool resetNewGyro(void);
    static void getRawGyro(int16_t* rawGyro, int32_t* quat);
    
    static void getRawAccelGyro(int16_t* rawAccel, int16_t* rawGyro, int32_t* quat);
    static void getAccelGyro(double* vertVector, double* gyroVector);

    static bool haveMag(void);
    static void getRawMag(int16_t* rawMag);
    static void getNorthVector(double* vertVector, double* northVector); //give the vertVector obtained previously
    static void getNorthVector2(double* vertVector, double* gyroVector, double* northVector); //give the vertVector and gyroVector obtained previously
};

extern I2CWrapper i2CWrapper;

#endif //I2C_WRAPPER_H