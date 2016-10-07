// Programa del prototipo de nodo de adquisición de datos v2.0 para Plataforma SODAQ Autonomo
// Incluye mediciones de acelerometro y giroscopio, mediciones de temperatura, celda de carga, tiempo de RTC y estado de bateria 
// Modificado y adaptado para plataforma SODAQ Autonomo por Rodolfo Andrade Mayorga, grande la U.
// 25 de Julio de 2016

#include <SPI.h>
#include <SdFat.h>
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
#define Pin_onewire 15      //
#define Pin_onewire2 14     //
#define CLK         5       //
#define DOUT        4       //
#define pin_test    8       // pin para hacer pruebas de funcionamiento 
#define chipSelect CS_SD    //
#define ts_fast 100000      // tiempo de muestreo para sensores rápidos [microsegundos]
#define ts_slow 150         // tiempo de muestreo para sensores lentos (Num de veces que se ejecutan los lentos antes de los rápidos)
#define t_request_temp 135  // tiempo para el request de la temperatura  
#define i2c_IMU 0x69        // dirección I2C de la IMU
#define pin_batery A7       // pin analógico para la bateria, 
#define ret 500             // delay programado
#define t_com_test  310     // Tiempo para con sultar la radio y para ver si llegó algo, para este caso 30 segundos
#define BATVOLT_R1 4.7      //
#define BATVOLT_R2 10       //
#define ADC_AREF 3.3        //
#define ID_NODO 1           //

unsigned long  next_fast_sampling; //tiempo para la futura muestra de sensores rápidos
unsigned char  contador=0;         //contador para pasa de las mediciones rapidas a las lentas
unsigned long  cont=0;
String         st_name;
unsigned long  last_stat_send;
unsigned long  mins_between_last_stat;
byte           id_nodo;
char           fileName[13]; 
float calibration_factor = -455;

//---------------------- Creación de objetos

HX711 scale(DOUT, CLK);
SdFat sd;
File file;
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
void setup()
{   
  pinMode(BEE_VCC, OUTPUT);
  digitalWrite(BEE_VCC, HIGH);
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  
  Serial1.begin(BeeBaud);  //Inicio comunicación serial
  delay(100);

  scale.set_scale();
  scale.set_scale(calibration_factor);
  scale.tare();  //Reset the scale to 0
  eeprom_read();          //Lectura memoria EEPROM
  begin_SDcard();
  begin_RTC();
  //rtc.adjust(DateTime(__DATE__, __TIME__));
  build_fileName();
  begin_IMU();
  begin_temp_humi();
  Serial1.println("Paso config.");
  
  //----------------------------- Variables para comunicación -----------------------------------------//
  st_name = "s"+String(id_nodo);//nombre del nodo en el protocolo de comunicación
  //Serial1.println(st_name);
  mins_between_last_stat = 65000;//65segundos;
  last_stat_send = millis();
  //delay(1000);
}

void loop()
{ 
  int i; 
  send_ping();
    for (i=1;i<t_com_test;i++)
    {
     leer_sensores();
    }
  get_message();
}

void leer_sensores()
{
  next_fast_sampling = micros()+ts_fast;  //next_fast_sampling es el tiempo para tomar la futura muestra rápida
  contador++;
 
 if (contador !=ts_slow)    
 fast_sensors();               //Guarda Axyz, Gxyz
     
  if (contador == t_request_temp)
 {
  temperatura.requestTemperatures();                     //Solicitud de temperatura OneWire
 }
 
 if (contador == ts_slow)
 slow_sensors();                    //Lectura y rescate de sensores lentos
   
    Smart_delay();
}

void Smart_delay()
{    
  while (micros()<next_fast_sampling)       
  {      
  } 
}

void fast_sensors()
{   
 dataframe_fast *p_Frame_fast = &Frame_fast;       
 cont++;
 Frame_fast.ID_device=id_nodo;
 Frame_fast.N_frame=cont;

 accelgyro.getMotion6(&(Frame_fast.acc_x),&(Frame_fast.acc_y),&(Frame_fast.acc_z),
                      &(Frame_fast.giro_x),&(Frame_fast.giro_y),&(Frame_fast.giro_z));                

  Frame_fast.IMU_time=micros();    
      if (file.open(fileName, FILE_WRITE)) 
      {  
        //file.write(p_Frame_fast,30);
        file.write(p_Frame_fast,24);
        file.close();
       }  
      else 
      {
        //Serial.println("Error opening sd");
      } 
}

void slow_sensors()
{  
    cont++;
    Frame_slow.ID_device=id_nodo;
    Frame_slow.N_frame=cont;  
    temperatura.requestTemperatures();
    temperatura2.requestTemperatures();
    //Frame_slow.temperature1 = temperatura.getTempCByIndex(0);
    Frame_slow.temperature1 = temperatura.getTempFByIndex(0);
    Frame_slow.temperature2 = temperatura2.getTempFByIndex(0);
    Frame_slow.weigh=scale.get_units();
    //Serial1.println("Peso");
    //Serial1.println(scale.read_average());
    //Serial1.println(scale.get_units());
    
    DateTime RTCT = rtc.now(); 
    Frame_slow.time_RTC=RTCT.unixtime(); 
    Frame_slow.battery_status=getBATVOLT();

     contador=0;
     dataframe_slow *p_Frame_slow = &Frame_slow;
     
      if (file.open(fileName, FILE_WRITE)) 
      {  
        file.write(p_Frame_slow,26);
        file.close();
       }  
      else 
      {
        //Serial.println("Error opening sd");
      }
}

void eeprom_read()
{
 int Direccion = 0; 
 id_nodo = ID_NODO;
}

void begin_SDcard()
{
   if (sd.begin(chipSelect)) 
   {
    Serial1.println("microSD Inicializada.");
   }
   else
   {
    Serial1.println("Error microSD, o no esta presente");
   }  
   delay(ret);
}

void  begin_IMU()
{
 Wire.begin();
 accelgyro.initialize();
 delay(ret);
 Serial1.println(accelgyro.testConnection() ? "MPU9250 connection failed" : "MPU9250 connection succesful");
 delay(ret); 
}

void begin_temp_humi()
{
  temperatura.begin(); //Comenzar comunicación OneWire
  temperatura2.begin(); //Comenzar comunicación OneWire
  delay(ret);
}

void begin_RTC()
{  
 rtc.begin();
 delay(ret);
}

void build_fileName()
{
  unsigned long n_nodo    = id_nodo; 
  //unsigned long  t_RTC_ini = RTC.getTime();
  DateTime RTCT = rtc.now(); 
  unsigned long  t_RTC_ini = RTCT.unixtime(); 
  fileName[0]='N';
  fileName[1]= n_nodo+'0';
  fileName[2 ] = (t_RTC_ini/10000000)%10  +'0';
  fileName[3 ] = (t_RTC_ini/1000000)%10   +'0';
  fileName[4 ] = (t_RTC_ini/100000)%10    +'0';
  fileName[5 ] = (t_RTC_ini/10000)%10     +'0';
  fileName[6 ] = (t_RTC_ini/1000)%10      +'0';
  fileName[7 ] = (t_RTC_ini/100)%10       +'0';
  fileName[8 ] = '.';
  fileName[9 ] = 'R';
  fileName[10] = 'A';
  fileName[11] = 'W';
  fileName[12] = '\0';
}

//////////////////////FUNCIONES PARA COMUNICACION///////////////////////////////////////

// Calls proper function on command name
void msg_parser(String msg) {
  int idx_semicolon = msg.indexOf(";");
  if(idx_semicolon > -1){
    String n = msg.substring(idx_semicolon+1);
    // if the command is for me
    if(n.equals(st_name)) {
      String cmd = msg.substring(0, idx_semicolon);
      // do something with the command
      call_tasks(cmd);
    }
    else{
      //Serial.println("not for me");
      // do nothing
    }
  }
  else{
    //Serial.println("not a command");
    // do nothing
  }
}

void send_ping()
{
   //digitalWrite(pin_sleep,0); //Despertar bateria
   //delay(3);
  unsigned long mins = (millis() - last_stat_send);
  if(mins >= mins_between_last_stat)
  {
    //Serial.println(mins);
    Serial1.println("conectar;"+st_name);  
  }
  // digitalWrite(pin_sleep,1); //Dormir la radio
}

// Check for messages if any
void get_message() 
{
  String msg = get_msg();
  if(msg != "")
  {
    msg_parser(msg);
  }
  else 
  {
    //Serial.println("message is empty");
    // do nothing;
  }
}

String get_msg()
{
  String message(""); //string that stores the incoming message
  while(Serial1.available())
  {//while there is data available on the serial monitor
    message+=char(Serial1.read());//store string from serial command
  }
  return clean_str(message);
}

String clean_str(String s)
{
  while(s.lastIndexOf("\n") > -1 && s.lastIndexOf("\r") > -1)
  {
   s.replace("\n", "");
   s.replace("\r", "");
  }
  return s;
}

void call_tasks(String cmd) 
{
  if(cmd.equals("enviar"))
  {
    enviar_estadistica();
  }
}

void enviar_estadistica() 
{
  Serial1.println(construir_string());
  delay(55);//Tiempo verificado
  if(get_msg().equals("ok;"+st_name))
  {
    last_stat_send = millis();
  }
}

String construir_string()
{
  String dataString = "";
  dataString+=String("T1:") + String(temperatura.getTempCByIndex(0))+String(";");
  dataString+=String("T2:") + String(temperatura2.getTempCByIndex(0))+String(";");
  dataString+=String("F:") + String(Frame_slow.time_RTC)+String(";");
  dataString+=String("B:") + String(Frame_slow.battery_status)+String(";");
  dataString+=String("P:") + String(Frame_slow.weigh);
  return dataString;
}
float getBATVOLT()
{
  uint16_t batteryVoltage = analogRead(BAT_VOLT);
  return (ADC_AREF / 1023) * (BATVOLT_R1 + BATVOLT_R2) / BATVOLT_R2 * batteryVoltage;
}
