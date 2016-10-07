// Programa del prototipo de nodo de adquisición de datos v3.2 para Plataforma SODAQ Autonomo
// Incluye mediciones de acelerometro, mediciones de temperatura, celda de carga, tiempo de RTC.
// Modificado y adaptado para plataforma SODAQ Autonomo por Nelson Gatica Rodolfo Andrade Mayorga.
// 25 de Julio de 2016
// Modificado por Emedin Riffo y  Nelson Gatica el 28-09-2016 a las 17:40


// wash dog en caso de posibles problemas por demora en codigo.
#include "Sodaq_wdt.h"
#ifdef ARDUINO_ARCH_AVR
#elif ARDUINO_ARCH_SAMD
#endif

//Bibliotecas
#include <DallasTemperature.h>
#include <Wire.h>
#include <MPU9250.h>
#include <RTClib.h>
#include "HX711.h"
#include "frame_definition_v05.h"

//MAPA DE PINES (replica cajas)
//  ___________________
// |     /       \     |-D15- N/C
// |    |         |    |-D14- N/C
// |    |         |    |-D13- N/C
// |    |         |    |-D12- N/C
// |    |_________|    |-D11- N/C
// |                   |-D10- N/C
// |                   |-D9-- N/C
// |                   |-SDA- SDA I2C IMU y RTC
// |       SODAQ       |-SCL- SCL I2C IMU y RTC
// |                   |-D8-- N/C
// |      AUTONOMO     |-D7-- DT HX711
// |                   |-D6-- SCK HX711
// |                   |-D5-- N/C
// |                   |-D4-- N/C
// |                   |-D3-- N/C
// |                   |-D2-- N/C
// |                   |-D1-- Señal Temp 2
// |                   |-D0-- Señal Temp 1
// |                   |-3V3- Linea de 3V3
//GND----|___________________|-5V-- N/C

//-----------------------------Definición de variables y constantes
#define BeeBaud 115200     // velocidad serial de la radio (default)
#define Pin_onewire 0      // Temperatura 1
#define Pin_onewire2 1     // Temperatura 2
#define CLK         6      // CLK ADC
#define DOUT        7      // DT ADC
#define pin_test    8      // pin para hacer pruebas de funcionamiento 
#define ts_fast 100000     // tiempo de muestreo para sensores rápidos [microsegundos]
#define ts_slow 150        // tiempo de muestreo para sensores lentos (Num de veces que se ejecutan los lentos antes de los rápidos)
#define tRequestTempSensor 135 // tiempo para el request de la temperatura  
#define i2c_IMU 0x69       // dirección I2C de la IMU
#define pin_batery A7      // pin analógico para la bateria 
#define ret 500            // delay programado
#define size_package 14    // Tamaño de paquetes rapidos

/* 
IMPORTANTE : ACTUALMENTE LA CABECERA DEL FRAME API DEL PAQUETE IMU ESTA DEFINIDO PARA 5 MEDICIONES IMU. 
SI SE CAMBIA EL NUMERO DE MEDICIONES POR PAQUETE-IMU, SE DEBE ACTUALIZAR LA CABEZERA DE data_IMU_all EN FORMA MANUAL
*/
#define IMU_package_size 5      // Numeros de paquetes rapidos a enviar
byte data_IMU_all[size_package * IMU_package_size + 9]; // definicion de tamaño de paquete de frame_rapidos a enviar

unsigned long  next_fast_sampling; //tiempo para la futura muestra de sensores rápidos
unsigned char  contador = 0;       //contador para pasa de las mediciones rapidas a las lentas
float calibration_factor = -455;   //Factor de calibracion para lectura de peso

byte count_IMU_read = 0; //Contador de numero de paquetes a enviar
byte contador_shift_IMU_package = 0; // Contador para enpaquetado de la lectura de la IMU


//------ Creación de objetos
HX711 scale(DOUT, CLK); //objeto para lectura de peso
MPU9250 accelgyro(0x69); //Dirección I2C 0x69 para la IMU
RTC_DS3231 rtc; //variable para el RTC

//Variables para lectura de temperatura.
OneWire ourWire(Pin_onewire);
OneWire ourWire2(Pin_onewire2);
DallasTemperature sensorTemp1(&ourWire);
DallasTemperature sensorTemp2(&ourWire2);

//Estructura para guardar datos rapidos y lentos
dataframe_fast Frame_fast;
dataframe_slow Frame_slow;

//--------------------- SETUP-------------------------------------------------------------
void setup() {
  // XBEE : comunicación de la radio
  pinMode(BEE_VCC, OUTPUT);
  digitalWrite(BEE_VCC, HIGH);
  Serial1.begin(BeeBaud);  //Inicio comunicación serial
  delay(100);
  make_head_API(); // Creacion de cabecera para comunicacion en modo API

  // SODAQ : wash dog en 8 segundos
  sodaq_wdt_enable(WDT_PERIOD_8X);

  /* SENSORES */
  // Calibracion de celda de carga
  scale.set_scale(calibration_factor);
  scale.tare();
  // Inicializa reloj, IMU y sensor de sensorTemp1.
  begin_RTC();
  begin_IMU();
  begin_temp();
}

//-----------------------------loop-------------------------------------------------------
void loop() {

  // Resetea el sistema si se queda pegado.
  sodaq_wdt_reset();



  //next_fast_sampling define una referencia de tiempo para la proxima toma de muestra
  next_fast_sampling = micros() + ts_fast;

  contador++;

  if (contador == ts_slow) { // Lectura y envio de datos de sensores lentos
    // Lee los datos lentos
    byte slowData = readSlowData();
    
    // Transfiere los datos por la puerta serial */
    Serial1.write(slowData, sizeof(slowData));
    
    // Resetea el contador
    contador = 0;
  }
  else if (contador == tRequestTempSensor) { // Solicitud de temperaturas a los termometros a traves del protocolo OneWire
    sensorTemp1.requestTemperatures();
    sensorTemp2.requestTemperatures();
  }
  else {
    count_IMU_read++;
    
    // Lee datos de IMU y los guarda IMU_data
    byte IMU_data = read_IMU();               
    
    if (count_IMU_read < IMU_package_size) { // Empaqueta los datos de la IMU en un bloque 
      package_IMU_data(IMU_data);
    }
    else {
      package_IMU_data(IMU_data); // Empaqueta el ultimo dato de la IMU
      
      // Ingresa el CheckSum a la data de la IMU en la posicion size_package * IMU_package_size + 8, donde 8 es el tamao de la cabezera del paquete API
      data_IMU_all[8 + size_package*IMU_package_size] = getChecksum(data_IMU_all);
      
      // Envia el paquete IMU (rapido) por la puerta serial
      Serial1.write(data_IMU_all, size_package * IMU_package_size + 9);
      
      count_IMU_read = 0;
      contador_shift_IMU_package = 0;
    }
  }

  //Lectura y rescate de sensores lentos
  while (micros() < next_fast_sampling) {}
}

//Función de

byte read_IMU()
{
  // variable para guardar datos de la IMU
  byte datos_IMU[size_package]; 

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
  
  return datos_IMU;
}

/* Empaqueta datos de la IMU en la variable global data_IMU_all */
void package_IMU_data(byte IMU_data) {
  for (int i = 0; i < size_package; i++) {
    data_IMU_all[i + contador_shift_IMU_package + 8] = IMU_data[i];
  }
  contador_shift_IMU_package = contador_shift_IMU_package + size_package;
}


/* Lee los datos de los sensores lentos (temperatura, peso y fecha) */
byte readSlowData()
{
  // definicion de tamaño de paquete de frame_lentos a enviar
  byte datos_Temp_Clock_Weight[25]; 
  
  Frame_slow.temperature1 = sensorTemp1.getTempFByIndex(0);
  Frame_slow.temperature2 = sensorTemp2.getTempFByIndex(0);
  Frame_slow.weigh = scale.read_average();

  DateTime RTCT = rtc.now();
  Frame_slow.time_RTC = RTCT.unixtime();

  // Define la cabecera XBee API
  datos_Temp_Clock_Weight[0] = 0x7E;//start
  datos_Temp_Clock_Weight[1] = 0x00;//lengthH
  datos_Temp_Clock_Weight[2] = 0x15;//lengthL
  datos_Temp_Clock_Weight[3] = 0x01;//type
  datos_Temp_Clock_Weight[4] = 0x01;//frame_ID
  datos_Temp_Clock_Weight[5] = 0x00;//frame_idH
  datos_Temp_Clock_Weight[6] = 0x00;//frame_idL
  datos_Temp_Clock_Weight[7] = 0x00;//Option

  // Caracteres que definen el COMIENZO de los datos de los datos "lentos"
  datos_Temp_Clock_Weight[8] = 0xBB;
  datos_Temp_Clock_Weight[9] = 0XBB;

  // Datos de TEMPERATURA 1
  datos_Temp_Clock_Weight[10] = Frame_slow.temperature1 >> 8;
  datos_Temp_Clock_Weight[11] = Frame_slow.temperature1;

  //Datos de TEMPERATURA 2
  datos_Temp_Clock_Weight[12] = Frame_slow.temperature2 >> 8;
  datos_Temp_Clock_Weight[13] = Frame_slow.temperature2;

  //Datos de PESO
  datos_Temp_Clock_Weight[14] = Frame_slow.weigh >> 24;
  datos_Temp_Clock_Weight[15] = Frame_slow.weigh >> 16;
  datos_Temp_Clock_Weight[16] = Frame_slow.weigh >> 8;
  datos_Temp_Clock_Weight[17] = Frame_slow.weigh;

  //Datos de FECHA
  datos_Temp_Clock_Weight[18] = Frame_slow.time_RTC >> 24;
  datos_Temp_Clock_Weight[19] = Frame_slow.time_RTC >> 16;
  datos_Temp_Clock_Weight[20] = Frame_slow.time_RTC >> 8;
  datos_Temp_Clock_Weight[21] = Frame_slow.time_RTC;

  // Caracteres que definen el FINAL de los datos de los datos "lentos"
  datos_Temp_Clock_Weight[22] = 0xFE;
  datos_Temp_Clock_Weight[23] = 0xFE;

  // Agrega Checksum
  datos_Temp_Clock_Weight[24] = getChecksum(datos_Temp_Clock_Weight);

  return datos_Temp_Clock_Weight;
}


/* Calcula el CheckSum de un arreglo de bytes */
unsigned char getChecksum(byte dataArray) {
  // Inicializa variable para calcular el checksum
  unsigned char check_sum = 0xFF;
  // Resta el resto de los valores
  for (int i = 3; i <= (sizeof(dataArray) - 2); i++) {
    check_sum = check_sum - dataArray[i];
  }
  return check_sum;
}



void  begin_IMU() {
  Wire.begin();
  accelgyro.initialize();
  delay(ret);
  Serial1.println(accelgyro.testConnection() ? "MPU9250 connection failed" : "MPU9250 connection succesful");
  delay(ret);
}

void begin_temp() {
  sensorTemp1.begin(); //Comenzar comunicación OneWire
  sensorTemp2.begin(); //Comenzar comunicación OneWire
  delay(ret);
}

void begin_RTC() {
  rtc.begin();
  delay(ret);
}

//////////////////////FUNCIONES PARA COMUNICACION///////////////////////////////////////

void make_head_API() {
  //Head API
  data_IMU_all[0] = 0x7E;//start
  data_IMU_all[1] = 0x00;//lengthH
  data_IMU_all[2] = 0x4B;//lengthL
  data_IMU_all[3] = 0x01;//type
  data_IMU_all[4] = 0x01;//frame_ID
  data_IMU_all[5] = 0x00;//frame_idH
  data_IMU_all[6] = 0x00;//frame_idL
  data_IMU_all[7] = 0x00;//Option
}






