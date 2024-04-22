#include <NexText.h>
#include <Nextion.h>
#include <max6675.h>
#include <math.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
SoftwareSerial nextion(17, 16);
uint32_t rades, pagina, S1v, S2v, S3v, S4v, TemEsp, sm, bINICIAR, Dt, TEx;
int reintento = 2;
float Tact, delta, t, ti, Prms, radval;
int  signo, tg, t0, ni, tf;
bool est1, est2, est3, est4, bandera, bandera2, bandera3, banderaA;
String txtT, txtT2, nombreArch, lineaArch, radsufijo;
char nombreArch0[20];
int thermo_sck_pin = 35;//;
int thermo_cs_pin  = 33;//;
int thermo_so_pin  = 31;//;
//Sensor luminosidad
uint32_t lum;
uint16_t ir, full;



MAX6675 thermocouple(thermo_sck_pin, thermo_cs_pin, thermo_so_pin);
//Definiciones term. infrarrojos
float st1, st2, st3, st4;
int vt1, vt2, vt3, vt4;
Adafruit_MLX90614 mlx1 = Adafruit_MLX90614(0x50);
Adafruit_MLX90614 mlx2 = Adafruit_MLX90614(0x51);
Adafruit_MLX90614 mlx3 = Adafruit_MLX90614(0x52);
File logFile;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  TemEsp = 30;
  Wire.begin();
  nexInit();
  mlx1.begin();
  mlx2.begin();
  mlx3.begin();
  //mlx4.begin();
  est1 = false;
  est2 = false;
  est3 = false;
  est4 = false;
  bandera = true;
  bandera2 = true;
  bandera3 = true;
  banderaA = true;
  sm = 1;
  Dt = 1;
  TEx = 200;//150 peque//400 grande
  sendCommand("page 0");
  pinMode(7, OUTPUT);

  Serial.println("Memoria:::" + String(SD.begin(7)));
  while (reintento > 0) {
    delay(100);
    Serial.println("Memoria;;;: " + String(SD.begin(7)));
    if (SD.begin(7)) {
      // Serial.println("Memoria encontrada: " + String(SD.begin(7)));

      reintento = 0;
      sm = 1;
      delay(80);

    } else {
      if (bandera) {
        sendCommand("page 2");
        bandera = false;
      } else {
        Serial.println("Memoria no encontrada");
        sendCommand("get Error.sm2.val");
        recvRetNumber(&sm);
        Serial.println("valor sm2: " + String(sm) + "reintento: " + String(reintento));
        if (sm == 0) {
          reintento = 0;
        }
        delay(100);
      }
    }


  }


  delay(500);
  bandera = true;
  sendCommand("page 1");
  delay(80);
  sendCommand("get Medida.nombreArch.txt");
  delay(80);
  recvRetString(nombreArch0, 50);
  delay(80);
  nombreArch = String(nombreArch0) + ".CVS";
  delay(80);
  sendCommand("get Medida.Dt.val");
  delay(80);
  recvRetNumber(&Dt);
  radsufijo = "";
  radval = 0;


  Serial.println("==========" + nombreArch + "===========");
  Serial.println("Dt: " + String(Dt));
  txtT = "Medida.TEx.val=" + String(TEx);
  sendCommand(txtT.c_str());
  tg = 45;


}

void loop() {
  Serial.println("===================INICIA " + String(ni) + "=================");
  ti = millis();
  tf = millis() - ti;

  sendCommand("get Medida.bS04.val");
  recvRetNumber(&rades);
  delay(tg);


  while (tf  <= Dt * 1000) {// intervlo temporal
    Serial.println("1. Tiempo: " + txtT + ", tf: " + String(tf ) + ", Dt*1000: " + String(Dt * 1000));
    sendCommand("get Medida.Dt.val");
    recvRetNumber(&Dt);
    Tact = thermocouple.readCelsius() - 5.5;//Calibracion termocupla
    txtT = "tTEM.txt=\"" + String(Tact) + " \xB0 C\"";
    sendCommand(txtT.c_str());
    //// Interactuar con el esclavo
    delay(tg);
    sendCommand("get ValTemp.val");
    recvRetNumber(&TemEsp);

    delta = TemEsp - Tact;
    if (delta >= 0) {
      signo = 0;
    } else {
      signo = 1;
    }
    Wire.beginTransmission(2);
    int pr1, pr2, pr3, Pot1, Pot2, Pot3, Pot4, Pot5;
    pr3 = int(abs(delta * 10)  ) % 10;
    pr2 = int(abs(delta * 10 ) / 10) % 10;
    pr1 = int(abs(delta * 10) / 100) % 10;
    Wire.write(signo);
    Wire.write(pr1);
    Wire.write(pr2);
    Wire.write(pr3);
    Serial.println("P3:" + String(int(abs(delta * 10) )));
    Serial.println("Enviando: " + String(signo) + String(pr1) + String(pr2) + String(pr3) + "\n");
    Wire.endTransmission();
    delay(tg);
    Wire.requestFrom(2, 5);
    Pot1 = int(Wire.read());
    Pot2 = int(Wire.read());
    Pot3 = int(Wire.read());
    Pot4 = int(Wire.read());
    Pot5 = int(Wire.read());
    Prms = Pot1 * 100 + Pot2 * 10 + Pot3 + Pot4 * 0.1 + Pot5 * 0.01;
    txtT = "tPOT.txt=\"" + String(Prms) + " W\"";
    sendCommand(txtT.c_str());
    delay(tg);
    //

    //Valor S1
    st1 = VB(mlx1.readObjectTempC());
    txtT = "tS01.txt=\"" + String(st1) + "\xB0 C\"";
    sendCommand(txtT.c_str());

    vt1 = map(st1, 0, 150, 0, 255);
    txtT2 = "S1n.val=" + String(vt1);
    sendCommand(txtT2.c_str());


    //Valor sensor 2
    st2 = VB(mlx2.readObjectTempC());
    txtT = "tS02.txt=\"" + String(st2) + "\xB0 C\"";
    sendCommand(txtT.c_str());

    vt2 = map(st2, 0, 150, 0, 255);
    txtT2 = "S2n.val=" + String(vt2);
    sendCommand(txtT2.c_str());

    //Valor S3
    st3 = VB(mlx3.readObjectTempC());
    txtT = "tS03.txt=\"" + String(st3) + "\xB0 C\"";
    sendCommand(txtT.c_str());

    vt3 = map(st3, 0, 150, 0, 255);
    txtT2 = "S3n.val=" + String(vt3);
    sendCommand(txtT2.c_str());

    Serial.println("BOTON Lux" + String(rades));


    if (rades == 0) {
      tsl.setGain(TSL2591_GAIN_MED);
      tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
      lum = tsl.getFullLuminosity();
      ir = lum >> 16;
      full = lum & 0xFFFF;


      radsufijo = " Lux";
      radval = tsl.calculateLux(full, ir);
    } else if (rades == 1) {
      tsl.setGain(TSL2591_GAIN_HIGH);
      tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);


      lum = tsl.getFullLuminosity();
      ir = lum >> 16;
      full = lum & 0xFFFF;


      radsufijo = "uW/cm2";
      radval = ir / 3474; //3474 factor de escala en el sensor
    }
    //Valor sensor 4
    st4 = radval;
    txtT = "tS04.txt=\"" + String(st4, 2) + radsufijo + "\"";
    sendCommand(txtT.c_str());

    vt4 = map(st4, 0, 150, 0, 255);
    txtT2 = "S4n.val=" + String(vt4);
    sendCommand(txtT2.c_str());

    //Serial.println(txtT.c_str());

    ////Nombre arch
    sendCommand("get Medida.nombreArch.txt");
    recvRetString(nombreArch0, 20);
    delay(tg);
    nombreArch = String(nombreArch0) + ".CSV";

    memset(nombreArch0, 0, sizeof(nombreArch0));
    sendCommand("get Medida.bINICIO.val");
    recvRetNumber(&bINICIAR);
    delay(tg);
    tf = millis() - ti;
  }//Fin de whiletemporal
  //Guardar en msd

  if (sm == 1 and bINICIAR == 1) {

    if (bandera3) {
      ni = 0;
      t = 0;
      bandera3 = false;
    }

    logFile = SD.open(nombreArch, FILE_WRITE);
    Serial.println(logFile);

    if (logFile) {
      Serial.println("==========INICIA ESCRITURA========");
      if (banderaA) {
        lineaArch = "Tiempo(s),Temp. Esperada(°C),Temp. Actual(°C),Pot. Actual(W),Temp S1(°C),Temp S2(°C),Temp S3(°C),Temp S4(°C)";
        logFile.println(lineaArch);
        logFile.close();
        banderaA = false;
      } else {

        lineaArch = String(t) + "," + String(TemEsp) + "," + String(Tact) + "," + String(Prms) + "," + String(st1) + "," + String(st2) + "," + String(st3) + "," + String(st4);
        logFile.println(lineaArch);
        logFile.close();
        sendCommand("g1.txt=\"\"");
      }
      Serial.println(lineaArch);
    } else {
      Serial.println("==========No hay memoria========");
      sendCommand("g1.txt=\" No se ha podido guardar los datos compruebe que hay memoria o que no está dañada\"");
    }

  } else {
    banderaA = true;
    bandera3 = true;
  }//Guardar

  t = t + Dt;

  txtT = "t9.txt=\"" + String(t) + " (s)\"";
  sendCommand(txtT.c_str());

  txtT = "t.val=" + String(ni);
  sendCommand(txtT.c_str());

  //Micro sd
  Serial.println("Tiempo: " + txtT + ", Dt: " + String(Dt) + ", ni: " + String(ni) + ", TEx: " + String(TEx));
  Serial.println("Arch: " + nombreArch + ", TemEsp: " + String(TemEsp) + ", Delta: " + String(delta));
  ni += 1;

}



float VB(float vt)
{
  if (vt > TEx + 50.0) {
    return 0;
  }
  else {
    return vt;
  }
}

