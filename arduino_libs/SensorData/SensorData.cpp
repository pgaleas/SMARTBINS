/*
  SensorData.cpp - Library for manage data from different sensors.
  Project SMARTHARVEST - Universidad de La Frontera
  Created by Patricio Galeas, May 10, 2017.
*/

#include SensorData.h

char _data[6];
char _sensorMetaData;


/* Constructor */
SensorData::SensorData() {

}


/* Set time_t data (Example: RTC) */
void SensorData::SetTimeData(char sensorType, char counter, time_t t) {
		
		_dataType = 0; // corresponding to time_t

		_counter = counter;

		_sensorMetaData = sensorType; // no shiftting / "or" are necessary

		_data[0] = t >> 24;
		_data[1] = t >> 16;
		_data[2] = t >> 8;
		_data[3] = t;
}

/* Set int16_t (array) data (Example: IMU) */
void SensorData::SetInt16Data(char sensorType, char counter, int16_t i1, int16_t i2, int16_t i3) {
		
		_dataType = 1; // corresponding to int16_t

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
void SensorData::SetFloatData(char sensorType, char counter, float f) {
		
		_dataType = 2; // corresponding to float

		_counter = counter;

		_sensorMetaData = _dataType << 4 | sensorType;
		
		_data[0] = f >> 24;
		_data[1] = f >> 16;
		_data[2] = f >> 8;
		_data[3] = f;
}

/* Set long data (Example: Weight) */
void SensorData::SetLongData(char sensorType, char counter, long l) {
		
		_dataType = 3; // corresponding to long

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
	_dataType = (int8_t)((_sensorMetaData & 0xF0) >> 4) 
	
	switch (_dataType) {
	    case 0:    // RTC data (timestamp) - 4 Bytes
			// Here the reconstruct the time value from the byte array (_data)
	    	time_c t = (((time_c)_data[0]) << 24) | (((time_c)_data[1]) << 16) | (((time_c)_data[2]) << 8) | (time_c)_data[3];
	    	Serial.println("time:" + t + "\n"); 
			break;

	    case 1: // IMU data - 6 Bytes (1 Byte each axis)
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
	    	Serial.println("ax:" + ax + " ay:" + ay + " az:" + az + "\n");
			break;

	    case 2: // TEMPERATURE data - 4 Bytes (float)
			// Here the reconstruct the time value from the byte array (_data)
	    	float temperature = (((float)_data[0]) << 24) | (((float)_data[1]) << 16) | (((float)_data[2]) << 8) | (float)_data[3];
	    	Serial.println("temperature: " + temperature + "\n"); 
	    	break;

	    case 3: // WEIGHT data - 4 Bytes (float)
			// Here the reconstruct the time value from the byte array (_data)
	    	long weight = (((long)_data[0]) << 24) | (((long)_data[1]) << 16) | (((long)_data[2]) << 8) | (long)_data[3];
	    	Serial.println("weight: " + weight + "\n"); 
	    	break;
	}
}

