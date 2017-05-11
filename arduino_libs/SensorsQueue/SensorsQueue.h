/*
  SensorsQueue.h - Library for manage data from different sensors.
  Project SMARTHARVEST - Universidad de La Frontera
  Created by Patricio Galeas, May 10, 2017.
*/
#ifndef SensorsQueue_h
#define SensorsQueue_h

#include <Arduino.h>
#include <QueueArray.h>

Class SensorsQueue {
	
	public:
		
		SensorsQueue(int bufferSize); // Constructor 

		void setBufferSize(int newBufferSize); // Define the size of the data set to be loaded in the (transmition) buffer

		void increaseBufferSize(); // Increase _bufferSize in one register.

		void decreaseBufferSize(); // Decrease _bufferSize in one register.

		int getBufferSize(); // Get the actual size of _bufferSize

		void push(SensorData data); // Add one register to the queue

		SensorData pop(); // Get the last register from the queque

		SensorData[] nPop(); // Get the data contained in _bufferQueueArray, corresponding to the last _bufferSize registers from _sensorQueueArray

	private:

		int _bufferSize; // Define the number of register to be loaded in the buffer for future transmition. 

		QueueArray _sensorQueueArray; // Queue containing the actual stored sensor data

		QueueArray _bufferQueueArray; // Queue containing the data to be transmitted

		void loadBuffer(); // Moves _bufferSize elements from _sensorQueueArray to _bufferQueueArray


}

#endif
