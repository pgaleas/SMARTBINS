// Version dataframe datalogger_v05 
// Modificada por Rodolfo Andrade
// 28 de Junio de 2016

// Frame rápido
typedef struct dataframe_fast 
{
  const byte Head = 0xAA;
  byte ID_device;
  unsigned long N_frame;
    
  int16_t acc_x;
  int16_t acc_y;
  int16_t acc_z;
	
  int16_t giro_x;
  int16_t giro_y;
  int16_t giro_z;
  
  unsigned long IMU_time;
   
  const int End = 0xFEFE; // Se guarda en memoria como little endian 
 };

// Frame lento
typedef struct dataframe_slow 
{
  const byte     Head = 0xBB;
  byte           ID_device;
  unsigned long  N_frame;
    
  int16_t            temperature1;
  int16_t            temperature2;
  // unsigned int   temperature3;  // Obtener del RTC
   
  float           weigh;
  
  //unsigned long  time_RTC;
  unsigned long  time_RTC;

  //unsigned int   battery_status;
  float   battery_status;
      
  const int End = 0xFEFE; // Se guarda en memoria como little endian
};
