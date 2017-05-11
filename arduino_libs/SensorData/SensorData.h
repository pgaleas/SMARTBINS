/*
  SensorData.h - Library for manage data from different sensors.
  Project SMARTHARVEST - Universidad de La Frontera
  Created by Patricio Galeas, May 10, 2017.
*/
#ifndef SensorData_h
#define SensorData_h

#include <Arduino.h>

Class SensorData {
	
	public:
		Boolean _data_transmitted = false; //true if the register is already stored or transmitted
		SensorData(byte dataType, byte data[6]); // Constructor
		void print();

	private:
		byte _dataType;
		byte _data[6];
		
};