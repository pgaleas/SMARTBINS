/*
  SensorData.cpp - Library for manage data from different sensors.
  Project SMARTHARVEST - Universidad de La Frontera
  Created by Patricio Galeas, May 10, 2017.
*/

#include "SensorData.h"

int8_t _data[6];
int8_t _sensorMetaData;


/* Constructor */
SensorData::SensorData() {

}


/* Set time_t data (Example: RTC) */
void SensorData::SetTimeData(int8_t sensorType, int8_t counter, time_t t) {
		
		//int8_t _dataType = 0; corresponding to time_t (not used in this case)

		_counter = counter;

		_sensorMetaData = sensorType; // no shiftting / "or" are necessary

		_data[0] = t >> 24;
		_data[1] = t >> 16;
		_data[2] = t >> 8;
		_data[3] = t;
}

/* Set int16_t (array) data (Example: IMU) */
void SensorData::SetInt16ArrayData(int8_t sensorType, int8_t counter, int16_t i1, int16_t i2, int16_t i3) {
		
		int8_t _dataType = 1; // corresponding to int16_t

		_counter = counter;

		_sensorMetaData = _dataType << 4 | sensorType;
		
		_data[0] = i1 >> 8;
		_data[1] = i1;
		_data[2] = i2 >> 8;
		_data[3] = i2;
		_data[4] = i3 >> 8;
		_data[5] = i3;
}

/* Set float data (Example: Temperature) */
void SensorData::SetInt16Data(int8_t sensorType, int8_t counter, int16_t i) {
		
		int8_t _dataType = 2; // corresponding to float

		_counter = counter;

		_sensorMetaData = _dataType << 4 | sensorType;
		
		_data[0] = i >> 8;
		_data[1] = i;

}

/* Set long data (Example: Weight) */
void SensorData::SetLongData(int8_t sensorType, int8_t counter, long l) {
		
		int8_t _dataType = 3; // corresponding to long

		_counter = counter;

		_sensorMetaData = _dataType << 4 | sensorType;
		
		_data[0] = l >> 24;
		_data[1] = l >> 16;
		_data[2] = l >> 8;
		_data[3] = l;	
}

/* Print the sensor data and datatypeAndCounte */
void SensorData::print() {
	
	String _registerString;
	int8_t _dataType;

	/* Filtering the _dataType from the first 4 bits of _sensorMetaData 
	using the mask (11110000) and shifting */ 
	_dataType = (int8_t)((_sensorMetaData & 0xF0) >> 4);
	
	switch (_dataType) {
	    case 0: {
	    	// RTC data (timestamp) - 4 Bytes
			// Here the reconstruct the time value from the byte array (_data)
	    	time_t time = (((time_t)_data[0]) << 24) | (((time_t)_data[1]) << 16) | (((time_t)_data[2]) << 8) | (time_t)_data[3];
	    	Serial.println("time: ");
	    	Serial.println(time, DEC);
	    	Serial.println("\n"); 
			break;
		}
	    case 1: {
	    	// IMU data - 6 Bytes (1 Byte each axis)
			// Here the reconstruct the time value from the byte array (_data)
	    	
			/* Dado que que en 
			MPU9250.cpp -> getAccel(float* ax, float* ay, float* az) 
			a pesar de que los parametros son float, los valores 
			de los registros de la IMU estan en int16_t. De este modo
			podriamos guardar estos valores sin hacer es escalamiento 
			(_accelScale) y conservarlos en int16_t para
			guardarlos en la cola.
			Para esto podriamos implementar un nuevo metodo:
			void MPU9250::getAccel_Raw(int16_t* ax, int16_t* ay, int16_t* az)
			*/
	    	int16_t ax = (((int16_t)_data[0]) << 8) | (int16_t)_data[1]; 
	    	int16_t ay = (((int16_t)_data[2]) << 8) | (int16_t)_data[3];
	    	int16_t az = (((int16_t)_data[4]) << 8) | (int16_t)_data[5];
	    	Serial.println("ax:");
	    	Serial.println(ax, DEC);
	    	Serial.println(" ay:");
	    	Serial.println(ay, DEC);
	    	Serial.println(" az:");
	    	Serial.println(az, DEC);
	    	Serial.println("\n");
			break;
		}
	    case 2: {
	    	// TEMPERATURE data - 4 Bytes (int16_t)
			// Here the reconstruct the time value from the byte array (_data)
	    	float temperature = (((int16_t)_data[0]) << 8) | (int16_t)_data[1];
	    	Serial.println("temperature: ");
	    	Serial.println(temperature, DEC);
	    	Serial.println("\n"); 
	    	break;
	    }
	    case 3: {
	    	// WEIGHT data - 4 Bytes (long)
			// Here the reconstruct the time value from the byte array (_data)
	    	long weight = (((long)_data[0]) << 24) | (((long)_data[1]) << 16) | (((long)_data[2]) << 8) | (long)_data[3];
	    	Serial.println("weight: ");
	    	Serial.println(weight, DEC);
	    	Serial.println("\n"); 
	    	break;
	    }
	}
}

