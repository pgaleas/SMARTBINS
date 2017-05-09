#include StackRegister.h

StackRegister::StackRegister(byte dataType, byte data[6]) {
		_dataType = dataType;
		_data = data;
}

StackRegister::print() {
	
	String _registerString;
	switch (_dataType) {
	    case 0:    // RTC data (timestamp) - 4 Bytes
			// Here the code to convert the byte-data to string
	    	// _registerString = ???
			break;
	    case 0x01: // IMU data - 6 Bytes (1 Byte each axis)
			// Here the code to convert the byte-data to string
	    	// _registerString = ???
			break;
	    case 0x02: // TEMPERATURE data - 4 Bytes (float)
			// Here the code to convert the byte-data to string
			// _registerString = ???
	    	break;
	    case 0x03: // WEIGHT data - 4 Bytes (float)
			// Here the code to convert the byte-data to string
			// _registerString = ???
	    	break;
  }
	Serial.print(_registerString);
}

