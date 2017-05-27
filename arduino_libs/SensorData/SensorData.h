/*
  SensorData.h - Library for manage data from different sensors.
  Project SMARTHARVEST - Universidad de La Frontera
  Created by Patricio Galeas, May 10, 2017.
*/
#ifndef SensorData_h
#define SensorData_h

#include <Arduino.h>

class SensorData {
	
	public:
		/* Constructor for time_t data (Example: RTC) */
		SensorData();
		
		/* Set time_t data (Example: RTC) */
		void SetTimeData(int8_t sensorType, int8_t counter, time_t t);

		/* Set int16_t (array) data (Example: IMU) */
		void SetInt16ArrayData(int8_t sensorType, int8_t counter, int16_t i1, int16_t i2, int16_t i3);

		/* Set int16_t data (Example: Temperature) */
		void SetInt16Data(int8_t sensorType, int8_t counter, int16_t i);

		/* Set long data (Example: Weight) */
		void SetLongData(int8_t sensorType, int8_t counter, long l);

		/* Print the sensor data and datatype */
		void print(); // 

	private:
		/* _sensorMetaData contains two values: (1) a code of 4 bits for the type of the data returned by the sensor
		and (2) a code of 4 bits to identify the type of sensor. 
		The total of 8 bits will be devided in:
		4 (left) bits for the dataType (mask = 0xF0 or 11110000): .
			000 : time_t
			001 : int16_t
			010 : float
			011 : long

		4 (right) bits for sensorType (mask = 0xF or 00001111):
			000 : Timestamp
			001 : IMU
			010 : Temperature
			011 : Weight
		*/
		int8_t _sensorMetaData;

		/*
		This value will be used to get how (sequence)
		 each data register was inserted in the SensorQueue.*/
		int8_t _counter;


		/* Sensor data 
		6 bytes for storing different data types 
		IMU : The IMU returns 3 int16_t values (ax, ay, az). Each value will be splitted 
		into 2 bytes 

		Temperature: The temperature sensor returns a float number (4bytes). 
		Using the same procedure the data will be splitted in 4 bytes.

		Time :  The RTC returns a time_c value (4bytes). It will be also splited into 4 bytes.

		Weight : The weight sensor returns a long (4bytes). It will be also codified 
		into 4 bytes.
		*/
		int8_t _data[6];
		
};
#endif
