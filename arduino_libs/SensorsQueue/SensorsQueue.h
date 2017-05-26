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
		
		// Constructor 
		SensorsQueue(int bufferSize); 

		/* Define the size of the data set to be loaded 
		in the (transmition) buffer */
		void setBufferSize(int newBufferSize); 

		// Increase _bufferSize in one register.
		void increaseBufferSize(); 

		// Decrease _bufferSize in one register.
		void decreaseBufferSize(); 

		// Get the actual size of _bufferSize
		int getBufferSize(); 

		// Add one float value to the queue
		void push(char sensorType, float data); 

		// Add one float array to the queue
		void push(char sensorType, float data[]); 

		// Add one long value to the queue
		void push(char sensorType, long data); 

		// Add one time_t value to the queue
		void push(char sensorType, time_t data); 

		/* Get the data contained in _bufferQueueArray, 
		corresponding to the last _bufferSize registers 
		from _sensorQueueArray */
		SensorData[] popBuffer(); 

		/* Get the data contained in _bufferQueueArray 
		in string format, corresponding to the 
		last _bufferSize registers from _sensorQueueArray */
		String popBufferAsString(); 


	private:
		/* Define the number of register to be loaded 
		in the buffer for future transmition. */
		int _bufferSize; 

		/* Queue containing the actual stored sensors data */
		QueueArray _sensorsQueue; 

		/* Queue containing the data to be transmitted */
		QueueArray _bufferQueue; 

		/* Moves _bufferSize elements from _sensorQueueArray 
		to _bufferQueueArray */
		void loadBuffer(); 


}
#endif
