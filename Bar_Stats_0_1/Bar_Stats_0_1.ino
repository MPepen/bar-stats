

/*
 * Device provides 1 UUID for the entire sensor
 * 4 Services
 * acceleration " m/s^2 " 
 * magnetic " gauss"
 * gyro " dps (degree per second)"
 * temperature " C"
 * 
    Please note the long strings of data sent mean the *RTS* pin is
    required with UART to slow down data sent to the Bluefruit LE!  
*/
#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

/* The LSM9DS1 sensor information */
#include <Wire.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>  // not used in this demo but required!


/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* ...software SPI, using SCK/MOSI/MISO user-defined SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_SCK, BLUEFRUIT_SPI_MISO,
//                             BLUEFRUIT_SPI_MOSI, BLUEFRUIT_SPI_CS,
//                             BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

/* The service information */
#define ACC_SERVICE_UUID128 "77-BD-52-80-E5-B9-4A-7D-86-EB-7D-AB-D6-B9-E2-10"
#define ACC_CHAR_X_UUID128  "3B-F3-5F-A9-A6-9C-45-C2-BF-41-FB-E3-F5-8E-8A-3A"
#define ACC_CHAR_Y_UUID128  "CC-EE-EE-46-BE-42-4A-C7-AE-47-72-D5-E5-35-AE-3A"
#define ACC_CHAR_Z_UUID128  "A4-F1-60-24-3E-01-41-1E-82-55-52-99-71-B0-50-78"
#define MAG_SERVICE_UUID128 "21-4D-E2-77-C1-2A-43-51-84-09-96-74-3F-84-9C-CD"
#define MAG_CHAR_X_UUID128  "50-DE-51-8B-CD-25-47-70-A0-C9-B5-BC-BF-8E-19-AA"
#define MAG_CHAR_Y_UUID128  "E2-E9-2C-96-1D-D5-4B-A2-B4-35-B8-2A-50-CA-C3-BC"
#define MAG_CHAR_Z_UUID128  "DB-36-BC-2E-D2-D2-48-7C-AD-1C-63-F0-08-87-FF-C5"
#define GYR_SERVICE_UUID128 "5A-60-55-0C-99-43-4B-CF-A0-A5-40-E2-2A-B0-A3-E6"
#define GYR_CHAR_X_UUID128  "C0-62-75-25-D0-9A-4E-EB-92-0B-DB-73-B9-6F-63-32"
#define GYR_CHAR_Y_UUID128  "3E-99-2B-92-FE-AD-4F-CF-AE-C8-4F-7C-B8-87-AB-0E"
#define GYR_CHAR_Z_UUID128  "D5-D6-91-26-28-FA-42-5B-94-4A-03-4F-4C-AD-CA-1A"
#define TMP_SERVICE_UUID128 "9A-3C-8F-A6-5B-2B-46-DC-9E-A0-6F-58-FF-18-16-46"
#define TMP_CHAR_UUID128    "DA-3D-9E-51-BF-52-41-A0-81-7D-D0-7B-BE-48-31-72"

/* Extra GUID just in case
076C0DBE-5983-4FB3-AC8B-A84A0D336632
 */

/* The LSM9DS1 sensor Configuration */
// i2c
Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();
#define LSM9DS1_SCK A5
#define LSM9DS1_MISO 12
#define LSM9DS1_MOSI A4
#define LSM9DS1_XGCS 6
#define LSM9DS1_MCS 5

// You can also use software SPI
//Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1(LSM9DS1_SCK, LSM9DS1_MISO, LSM9DS1_MOSI, LSM9DS1_XGCS, LSM9DS1_MCS);
// Or hardware SPI! In this case, only CS pins are passed in
//Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1(LSM9DS1_XGCS, LSM9DS1_MCS);


/* The service information */

int32_t ACC_SERVICE_ID;
int32_t ACC_CHAR_X_ID;
int32_t ACC_CHAR_Y_ID;
int32_t ACC_CHAR_Z_ID;
//int32_t MAG_SERVICE_ID
//int32_t MAG_CHAR_X_ID
//int32_t MAG_CHAR_Y_ID
//int32_t MAG_CHAR_Z_ID
//int32_t GYR_SERVICE_ID
//int32_t GYR_CHAR_X_ID
//int32_t GYR_CHAR_Y_ID
//int32_t GYR_CHAR_Z_ID
//int32_t TMP_SERVICE_ID
//int32_t TMP_CHAR_ID

void setup(void)
{
  while (!Serial); // required for Flora & Micro
  delay(500);

  boolean success;

  Serial.begin(115200);
  
  Serial.println(F("Adafruit Bluefruit Bar_Stats Demo"));
  Serial.println(F("---------------------------------------------------"));

  randomSeed(micros());

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  /* Perform a factory reset to make sure everything is in a known state */
  Serial.println(F("Performing a factory reset: "));
  if (! ble.factoryReset() ){
       error(F("Couldn't factory reset"));
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  // this line is particularly required for Flora, but is a good idea
  // anyways for the super long lines ahead!
  // ble.setInterCharWriteDelay(5); // 5 ms

  /* Change the device name to make it easier to find */
  Serial.println(F("Setting device name to 'Bar_Stats_Sensor': "));

  if (! ble.sendCommandCheckOK(F("AT+GAPDEVNAME=Bar_Stats_Sensor")) ) {
    error(F("Could not set device name?"));
  }

  /* Add the Heart Rate Service definition */
  /* Service ID should be 1 */
  Serial.println(F("Adding the Bar_Stats_Sensor Acceleration Service definition: "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID128=" ACC_SERVICE_UUID128), &ACC_SERVICE_ID);
  if (! success) {
    error(F("Could not add Bar_Stats_Sensor Acceleration service"));
  }

  /* Add the Acceleration Measurement characteristic */
  /* Chars ID for Measurement should be 1 */
  Serial.println(F("Adding the Bar_Stats_Sensor Acceleration X Measurement characteristic: "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID128=" ACC_CHAR_X_UUID128 ", PROPERTIES=0x12, MIN_LEN=2, MAX_LEN=3"), &ACC_CHAR_X_ID);
    if (! success) {
    error(F("Could not add Acceleration X characteristic"));
  }

  Serial.println(F("Adding the Bar_Stats_Sensor Acceleration Y Measurement characteristic: "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID128=" ACC_CHAR_Y_UUID128 ", PROPERTIES=0x12, MIN_LEN=2, MAX_LEN=3"), &ACC_CHAR_Y_ID);
    if (! success) {
    error(F("Could not add Acceleration Y characteristic"));
  }

  Serial.println(F("Adding the Bar_Stats_Sensor Acceleration Z Measurement characteristic: "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID128=" ACC_CHAR_Z_UUID128 ", PROPERTIES=0x12, MIN_LEN=2, MAX_LEN=3"), &ACC_CHAR_Z_ID);
    if (! success) {
    error(F("Could not add Acceleration Z characteristic"));
  }
  
  /* Reset the device for the new service setting changes to take effect */
  Serial.print(F("Performing a SW reset (service changes require a reset): "));
  ble.reset();

  Serial.println();
  //  while (!Serial) {
//    delay(1); // will pause Zero, Leonardo, etc until serial console opens
//  }
  
  Serial.println("LSM9DS1 data read demo");
  
  // Try to initialise and warn if we couldn't detect the chip
  if (!lsm.begin())
  {
    Serial.println("Oops ... unable to initialize the LSM9DS1. Check your wiring!");
    while (1);
  }
  Serial.println("Found LSM9DS1 9DOF");

  // helper to just set the default scaling we want, see above!
  setupSensor();
  
}
void setupSensor()
{
  // 1.) Set the accelerometer range
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_4G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_8G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_16G);
  
  // 2.) Set the magnetometer sensitivity
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_8GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_12GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_16GAUSS);

  // 3.) Setup the gyroscope
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_500DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_2000DPS);
}
void setCharValue(int32_t charId, float value, int precision=3){
  //Set the specified characteristic to a floating point value.
  //Construct an AT+GATTCHAR command to send the float value.
  //The command will look like: AT+GATTCHAR=<charId>,<value as array of bytes>
  ble.print(F("AT-GATTCHAR="));
  ble.print(charId, DEC);
  ble.print(F(","));
  ble.println(value, precision);
  if (!ble.waitForOK())
  {
    Serial.println(F("Failed to get response !"));
  }
}

void loop(void)
{

  lsm.read();  /* ask it to read in the data */ 
  /* Get a new sensor event */ 
  sensors_event_t a, m, g, temp;
  lsm.getEvent(&a, &m, &g, &temp); 

// Read the Acceleration fron the Bar_Stats_Sensor
  float acceleration_X = a.acceleration.x;
  float acceleration_Y = a.acceleration.y;
  float acceleration_Z = a.acceleration.z;

//Set the characteristic values.
setCharValue(ACC_CHAR_X_ID,  acceleration_X);
setCharValue(ACC_CHAR_Y_ID,  acceleration_Y);
setCharValue(ACC_CHAR_Z_ID,  acceleration_Z);
  
  /* Delay before next measurement update */
  delay(2000);
}

//void loop() 
//{
//  lsm.read();  /* ask it to read in the data */ 
//
//  /* Get a new sensor event */ 
//  sensors_event_t a, m, g, temp;
//
//  lsm.getEvent(&a, &m, &g, &temp); 
//
//  Serial.print("Accel X: "); Serial.print(a.acceleration.x); Serial.print(" m/s^2");
//  Serial.print("\tY: "); Serial.print(a.acceleration.y);     Serial.print(" m/s^2 ");
//  Serial.print("\tZ: "); Serial.print(a.acceleration.z);     Serial.println(" m/s^2 ");
//
//  Serial.print("Mag X: "); Serial.print(m.magnetic.x);   Serial.print(" gauss");
//  Serial.print("\tY: "); Serial.print(m.magnetic.y);     Serial.print(" gauss");
//  Serial.print("\tZ: "); Serial.print(m.magnetic.z);     Serial.println(" gauss");
//
//  Serial.print("Gyro X: "); Serial.print(g.gyro.x);   Serial.print(" dps");
//  Serial.print("\tY: "); Serial.print(g.gyro.y);      Serial.print(" dps");
//  Serial.print("\tZ: "); Serial.print(g.gyro.z);      Serial.println(" dps");
//  
//  Serial.print("\temp: "); Serial.print(temp.temperature);      Serial.println(" C");
//  
//
//  Serial.println();
//  delay(200);
//}
