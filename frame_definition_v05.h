// Version dataframe datalogger_v05 
// Modificada por Rodolfo Andrade
// 28 de Junio de 2016

// Frame rápido
typedef struct dataframe_fast 
{
  int16_t Head = 0xAAAA;   
  int16_t acc_x;
  int16_t acc_y;
  int16_t acc_z; 
  int16_t giro_x;
  int16_t giro_y;
  int16_t giro_z; 
  
  unsigned long IMU_time; 
  int16_t End = 0xFEFE; // Se guarda en memoria (13bytes) como little endian 
 };

// Frame lento
typedef struct dataframe_slow 
{
  const byte   Head = 0xBB;  
  int16_t      temperature1;
  int16_t      temperature2;  
  int32_t      weigh;
  //unsigned long  time_RTC;
  uint32_t  time_RTC;
  int16_t End = 0xFEFE; // Se guarda en memoria (16bytes) como little endian
};
