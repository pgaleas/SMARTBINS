/*
  SensorQueue.cpp - Library for manage data from different sensors.
  Project SMARTHARVEST - Universidad de La Frontera
  Created by Patricio Galeas, May 10, 2017.
*/

#include SensorQueue.h

SensorQueue::SensorQueue(int bufferSize) {
	
	_bufferSize = bufferSize;

	QueueArray <SensorData> _sensorsQueue;

	QueueArray <SensorData> _bufferQueue;
}

/* Define the size of the data set to be loaded in the (transmition) buffer */
void SensorQueue::setBufferSize(int newBufferSize) {
	_bufferSize = bufferSize;
}

/* Increase _bufferSize in one register */
void SensorQueue::increaseBufferSize(){
	_bufferSize++;
} 

/* Decrease _bufferSize in one register */
void SensorQueue::decreaseBufferSize(){
	_bufferSize--;
} 

/* Get the actual size of _bufferSize */
int SensorQueue::getBufferSize(){
	return _bufferSize;
}


/* Add one time_t value to the queue */
void SensorQueue::push(char sensorType, char counter, time_t data){
	SensorData register = new SensorData(sensorType, counter, data);
	_sensorQueueArray.push(register);
}

/* Add one int16_t array to the queue */
void SensorQueue::push(char sensorType, char counter, int16_t data0, int16_t data1, int16_t data2){
	SensorData register = new SensorData(sensorType, counter, data0, data1, data2);
	_sensorQueueArray.push(register);
} 

/* Add one float value to the queue */
void SensorQueue::push(char sensorType, char counter, float data){
	SensorData register = new SensorData(sensorType, counter, data);
	_sensorQueueArray.push(register);
} 



/* Add one long value to the queue */
void SensorQueue::push(char sensorType, char counter, long data){
	SensorData register = new SensorData(sensorType, counter, data);
	_sensorQueueArray.push(register);
}


/* Get the data contained in _bufferQueueArray, 
corresponding to the last _bufferSize registers 
from _sensorQueueArray */
SensorData[] SensorQueue::popBuffer(){

}

/* Get the data contained in _bufferQueueArray in string format, 
corresponding to the last _bufferSize registers from _sensorQueueArray */
String popBufferAsString() {

}  

		
/* Moves _bufferSize elements 
from _sensorQueueArray to _bufferQueueArray */
void SensorQueue::loadBuffer(){

} 

