/*
  SensorsQueue.h - Library for manage data from different sensors.
  Project SMARTHARVEST - Universidad de La Frontera
  Created by Patricio Galeas, May 10, 2017.
*/
#ifndef SensorsQueue_h
#define SensorsQueue_h

#include <Arduino.h>
#include <QueueArray.h>

class SensorsQueue {
	
	public:
		
		SensorsQueue(int bufferSize); // Constructor 

		void setBufferSize(int newBufferSize); // Define the size of the data set to be loaded in the (transmition) buffer

		void increaseBufferSize(); // Increase _bufferSize in one register.

		void decreaseBufferSize(); // Decrease _bufferSize in one register.

		int getBufferSize(); // Get the actual size of _bufferSize

		void push(char sensorType, float data); // Add one float value to the queue

		void push(char sensorType, float data[]); // Add one float array to the queue

		void push(char sensorType, long data); // Add one long value to the queue

		void push(char sensorType, time_t data); // Add one time_t value to the queue

		SensorData[] popBuffer(); // Get the data contained in _bufferQueueArray, corresponding to the last _bufferSize registers from _sensorQueueArray

		String popBufferAsString(); // Get the data contained in _bufferQueueArray in string format, corresponding to the last _bufferSize registers from _sensorQueueArray


	private:

		int _bufferSize; // Define the number of register to be loaded in the buffer for future transmition. 

		QueueArray _sensorsQueue; // Queue containing the actual stored sensors data

		QueueArray _bufferQueue; // Queue containing the data to be transmitted

		void loadBuffer(); // Moves _bufferSize elements from _sensorQueueArray to _bufferQueueArray


}

#endif
