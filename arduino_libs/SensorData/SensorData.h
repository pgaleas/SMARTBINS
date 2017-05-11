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
		SensorData(char dataType, char data[6]); // Constructor
		void print();

	private:
		char _dataType;
		char _data[6];
		
};