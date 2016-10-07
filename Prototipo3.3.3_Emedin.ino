// Programa del prototipo de nodo de adquisición de datos v2.0 para Plataforma SODAQ Autonomo
// Incluye mediciones de acelerometro y giroscopio, mediciones de temperatura, celda de carga, tiempo de RTC y estado de bateria
// Modificado y adaptado para plataforma SODAQ Autonomo por Nelson Gatica Rodolfo Andrade Mayorga, grande la U.
// 25 de Julio de 2016
// Modificado por Emedin Riffo y  Nelson Gatica el 28-09-2016 a las 17:40

#include "Sodaq_wdt.h"
#ifdef ARDUINO_ARCH_AVR
#elif ARDUINO_ARCH_SAMD
#endif

#include <DallasTemperature.h>
#include <Wire.h>
#include <MPU9250.h>
#include <RTClib.h>
#include "HX711.h"
#include "frame_definition_v05.h"

//  ___________________ ----- MAPA DE PINES
// |     /       \     |-D15- Señal Temp 1
// |    |         |    |-D14- Señal Temp 2
// |    |         |    |-D13- GND Temp 1 y 2
// |    |         |    |-D12- N/C
// |    |_________|    |-D11- N/C
// |                   |-D10- N/C
// |                   |-D9-- GND IMU
// |                   |-SDA- SDA I2C IMU y RTC
// |       SODAQ       |-SCL- SCL I2C IMU y RTC
// |                   |-D8-- GND RTC
// |      AUTONOMO     |-D7-- N/C
// |                   |-D6-- N/C
// |                   |-D5-- SCK HX711
// |                   |-D4-- DT HX711
// |                   |-D3-- GND HX711
// |                   |-D2-- N/C
// |                   |-D1-- N/C Reservado para GPS
// |                   |-D0-- N/C Reservado para GPS
// |                   |-3V3- Linea de 3V3
// |___________________|-5V-- N/C

//-----------------------------Definición de variables y constantes
#define BeeBaud 115200      // velocidad serial de la radio (default)
#define Pin_onewire 0      //
#define Pin_onewire2 1     //
#define CLK         6      //
#define DOUT        7       //
#define pin_test    8       // pin para hacer pruebas de funcionamiento 
#define chipSelect CS_SD    //
#define ts_fast 100000      // tiempo de muestreo para sensores rápidos [microsegundos]
#define ts_slow 150         // tiempo de muestreo para sensores lentos (Num de veces que se ejecutan los lentos antes de los rápidos)
#define t_request_temp 135  // tiempo para el request de la temperatura  
#define i2c_IMU 0x69        // dirección I2C de la IMU
#define pin_batery A7       // pin analógico para la bateria, 
#define ret 500             // delay programado
#define BATVOLT_R1 4.7      //
#define BATVOLT_R2 10       //
#define ADC_AREF 3.3        //
#define ID_NODO 1           //
#define size_package 14
#define num_package 5

byte data_IMU_all[size_package*num_package+9]; 
unsigned long  next_fast_sampling; //tiempo para la futura muestra de sensores rápidos
unsigned char  contador = 0;       //contador para pasa de las mediciones rapidas a las lentas
unsigned long  cont = 0;
byte           id_nodo;
float calibration_factor = -455;

byte datos_Temp_Clock_Weight[25];
byte datos_IMU[size_package];
byte cont_num_package=0;
byte contador_shift=0;


//---------------------- Creación de objetos
HX711 scale(DOUT, CLK);
MPU9250 accelgyro(0x69); //Dirección I2C 0x69 para la IMU
I2Cdev   I2C_M;
RTC_DS3231 rtc;
OneWire ourWire(Pin_onewire);
OneWire ourWire2(Pin_onewire2);
DallasTemperature temperatura(&ourWire);
DallasTemperature temperatura2(&ourWire2);
dataframe_fast Frame_fast;//Estructura para guardar datos
dataframe_slow Frame_slow;

//--------------------- SETUP
void setup(){
  pinMode(BEE_VCC, OUTPUT);
  digitalWrite(BEE_VCC, HIGH);
  Serial1.begin(BeeBaud);  //Inicio comunicación serial
  delay(100);
  sodaq_wdt_enable(WDT_PERIOD_8X);
  //scale.set_scale();
  scale.set_scale(calibration_factor);
  scale.tare();  //Reset the scale to 0
  begin_RTC();
  //rtc.adjust(DateTime(__DATE__, __TIME__));
  begin_IMU();
  begin_temp();
  Serial1.println("Fin Configuración Inicial.");
  make_head_API();
}

//Lee todos los sensores en este Loop
void loop(){
  sodaq_wdt_reset();
  //next_fast_sampling define una referencia de tiepo para la proxima toma de muestra
  next_fast_sampling = micros() + ts_fast; 
  contador++;

   if (contador == ts_slow)
    slow_sensors();
   else if (contador == t_request_temp){
    temperatura.requestTemperatures();                     //Solicitud de temperatura OneWire
    temperatura2.requestTemperatures();
   }
   else{
    read_IMU();               //Guarda Axyz, Gxyz
     cont_num_package++;
      if (cont_num_package == num_package){
       read_IMU_package();
       make_chek_sum();
       Serial1.write(data_IMU_all, size_package*num_package+9);
       cont_num_package=0;
       contador_shift=0;
      }
      else{
       read_IMU_package();
      }
   }
    
   //Lectura y rescate de sensores lentos
   Smart_delay();
}

void Smart_delay(){
  while (micros() < next_fast_sampling){
  }
}

void read_IMU()
{
  dataframe_fast *p_Frame_fast = &Frame_fast;
  //cont++;
  //Frame_fast.ID_device = id_nodo;
  //Frame_fast.N_frame = cont;
  
  //Guarda los datos de la IMU en un arreglo
  //byte datos_IMU[14];

  accelgyro.getMotion6(&(Frame_fast.acc_x), &(Frame_fast.acc_y), &(Frame_fast.acc_z),
                       &(Frame_fast.giro_x), &(Frame_fast.giro_y), &(Frame_fast.giro_z));

  Frame_fast.IMU_time = micros();
  //INICIO
  datos_IMU[0] = Frame_fast.Head >> 8;
  datos_IMU[1] = Frame_fast.Head;

  //DATOS ACELEROMETRO
  datos_IMU[2] = Frame_fast.acc_x >> 8;
  datos_IMU[3] = Frame_fast.acc_x;
  datos_IMU[4] = Frame_fast.acc_y >> 8;
  datos_IMU[5] = Frame_fast.acc_y;
  datos_IMU[6] = Frame_fast.acc_z >> 8;
  datos_IMU[7] = Frame_fast.acc_z;

  //DATOS TIEMPO DE MUESTREO
  datos_IMU[8] = Frame_fast.IMU_time >> 24;
  datos_IMU[9] = Frame_fast.IMU_time >> 16;
  datos_IMU[10] = Frame_fast.IMU_time >> 8;
  datos_IMU[11] = Frame_fast.IMU_time;

  //FINAL
  datos_IMU[12] = Frame_fast.End >> 8;
  datos_IMU[13] = Frame_fast.End;
  
  //Enviar datos IMU por radio.
  //Serial1.write(datos_IMU, 20); 
}

void read_IMU_package(){
 for(int i=0;i<size_package;i++){
  data_IMU_all[i+contador_shift+8] = datos_IMU[i];
 }
 contador_shift=contador_shift+size_package; 
}

void slow_sensors()
{
  //cont++;
  //Frame_slow.ID_device = id_nodo;
  //Frame_slow.N_frame = cont;
  
  //Frame_slow.temperature1 = temperatura.getTempCByIndex(0);
  Frame_slow.temperature1 = temperatura.getTempFByIndex(0);
  Frame_slow.temperature2 = temperatura2.getTempFByIndex(0);
  Frame_slow.weigh = scale.read_average();

  DateTime RTCT = rtc.now();
  Frame_slow.time_RTC = RTCT.unixtime();
  
  contador = 0;
  //dataframe_slow *p_Frame_slow = &Frame_slow;



  //INICIO
  datos_Temp_Clock_Weight[8] = 0xBB;
  datos_Temp_Clock_Weight[9] = 0XBB;

  //TEMPERATURA 1
  datos_Temp_Clock_Weight[10] = Frame_slow.temperature1 >> 8;
  datos_Temp_Clock_Weight[11] = Frame_slow.temperature1;

  //TEMPERATURA 2
  datos_Temp_Clock_Weight[12] = Frame_slow.temperature2 >> 8;
  datos_Temp_Clock_Weight[13] = Frame_slow.temperature2;

  //PESO
  datos_Temp_Clock_Weight[14] = Frame_slow.weigh >> 24;
  datos_Temp_Clock_Weight[15] = Frame_slow.weigh >> 16;
  datos_Temp_Clock_Weight[16] = Frame_slow.weigh >> 8;
  datos_Temp_Clock_Weight[17] = Frame_slow.weigh;

  //FECHA
  datos_Temp_Clock_Weight[18] = Frame_slow.time_RTC >> 24;
  datos_Temp_Clock_Weight[19] = Frame_slow.time_RTC >> 16;
  datos_Temp_Clock_Weight[20] = Frame_slow.time_RTC >> 8;
  datos_Temp_Clock_Weight[21] = Frame_slow.time_RTC;

  //FINAL
  datos_Temp_Clock_Weight[22] = 0xFE;
  datos_Temp_Clock_Weight[23] = 0xFE;

  unsigned char check_sum=0xFF;
  for (int i=3; i<=(sizeof(datos_Temp_Clock_Weight)-2); i++){
    check_sum =check_sum - datos_Temp_Clock_Weight[i];  
   }
  datos_Temp_Clock_Weight[24]=check_sum;
  
//Enviar datos temperatura, reloj y peso por radio.
  Serial1.write(datos_Temp_Clock_Weight, 25);
}

void  begin_IMU(){
  Wire.begin();
  accelgyro.initialize();
  delay(ret);
  Serial1.println(accelgyro.testConnection() ? "MPU9250 connection failed" : "MPU9250 connection succesful");
  delay(ret);
}

void begin_temp(){
  temperatura.begin(); //Comenzar comunicación OneWire
  temperatura2.begin(); //Comenzar comunicación OneWire
  delay(ret);
}

void begin_RTC(){
  rtc.begin();
  delay(ret);
}

//////////////////////FUNCIONES PARA COMUNICACION///////////////////////////////////////

void make_head_API(){
//Head API
data_IMU_all[0] = 0x7E;//start
data_IMU_all[1] = 0x00;//lengthH
data_IMU_all[2] = 0x4B;//lengthL 
data_IMU_all[3] = 0x01;//type
data_IMU_all[4] = 0x01;//frame_ID
data_IMU_all[5] = 0x00;//frame_idH
data_IMU_all[6] = 0x00;//frame_idL
data_IMU_all[7] = 0x00;//Option

datos_Temp_Clock_Weight[0] = 0x7E;//start
datos_Temp_Clock_Weight[1] = 0x00;//lengthH
datos_Temp_Clock_Weight[2] = 0x15;//lengthL 
datos_Temp_Clock_Weight[3] = 0x01;//type
datos_Temp_Clock_Weight[4] = 0x01;//frame_ID
datos_Temp_Clock_Weight[5] = 0x00;//frame_idH
datos_Temp_Clock_Weight[6] = 0x00;//frame_idL
datos_Temp_Clock_Weight[7] = 0x00;//Option
}
void make_chek_sum(){
unsigned char check_sum=0xFF;
  for (int i=3; i<=(sizeof(data_IMU_all)-2); i++){
    check_sum =check_sum - data_IMU_all[i];  
   }
  data_IMU_all[78]=check_sum;
}





