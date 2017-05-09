Class StackRegister {
	
	public:
		Boolean _data_transmitted = false; //true if the register is already stored or transmitted
		StackRegister(byte dataType, byte data[6]); // Constructor
		void print();

	private:
		byte _dataType;
		byte _data[6];
		
};