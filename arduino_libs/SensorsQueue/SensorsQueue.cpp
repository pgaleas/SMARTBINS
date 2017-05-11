/*
  SensorQueue.cpp - Library for manage data from different sensors.
  Project SMARTHARVEST - Universidad de La Frontera
  Created by Patricio Galeas, May 10, 2017.
*/

#include SensorQueue.h

SensorQueue::SensorQueue(int bufferSize) {
	_bufferSize = bufferSize;
}

// Define the size of the data set to be loaded in the (transmition) buffer
void SensorQueue::setBufferSize(int newBufferSize) {

}

		 
// Increase _bufferSize in one register.
void SensorQueue::increaseBufferSize(){

} 

// Decrease _bufferSize in one register.
void SensorQueue::decreaseBufferSize(){

} 

// Get the actual size of _bufferSize
int SensorQueue::getBufferSize(){

} 

// Add one register to the queue
void SensorQueue::push(SensorData data){

} 

// Get the last register from the queque
SensorData SensorQueue::pop(){

}

// Get the data contained in _bufferQueueArray, 
// corresponding to the last _bufferSize registers 
// from _sensorQueueArray
SensorData[] SensorQueue::nPop(){

}

// Moves _bufferSize elements from _sensorQueueArray to _bufferQueueArray
void SensorQueue::loadBuffer(){

} 

