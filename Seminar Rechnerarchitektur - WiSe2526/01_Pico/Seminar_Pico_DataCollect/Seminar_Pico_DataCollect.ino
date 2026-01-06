// 15.Dez 2025
// Licht
#include <BH1750.h>
BH1750 lightMeter;
float lux = 0;

// Uhr
#include <Wire.h>
#include "RTClib.h"
#define SDA_UHR 14  
#define SCL_UHR 15
RTC_DS3231 rtc;

// SD Card
#include <SPI.h>
#include <SD.h>
const int _MISO = 16;  
const int _MOSI = 19;  
const int _CS = 17;
const int _SCK = 18;
const int RP_CLK_GPIO = -1; // Set to CLK GPIO
const int RP_CMD_GPIO = -1; // Set to CMD GPIO
const int RP_DAT0_GPIO = -1; // Set to DAT0 GPIO

// Temperatur
#include <OneWire.h>
OneWire TempPin10(21); // GPIO21 = DS18S20 S pin
float temperatur;
byte addr10[8];  
byte data10[12];

// Bewegung
const int GPIO_Bewegung = 22;   
int status_Bewegung = 0;

// Geräusch
const int analogPin = 28; // ADC2
const int GPIO_Geraeusch = 13;
int status_Geraeusch = 0;
int pegel = 0;
#define SHIFTLEN 5
int pegelArray[SHIFTLEN];

void setup() 
{  
  Serial.begin(115200);
  delay(3000);

  Serial.print("Initializing sound sensor...");
  //analogReadResolution(12);
  pinMode(analogPin, INPUT);
  pinMode(GPIO_Geraeusch, INPUT);  
  Serial.println("done");
  delay(500);

  Serial.print("Initializing movement sensor...");
  pinMode(GPIO_Bewegung, INPUT); 
  pinMode(LED_BUILTIN , OUTPUT); 
  Serial.println("done"); 
  delay(500);

  Serial.print("Initializing light sensor...");
  Wire.begin();
  lightMeter.begin();
  Serial.println("done");
  delay(500);
  
  Serial.print("Initializing real time clock...");
  Wire1.setSDA(SDA_UHR);
  Wire1.setSCL(SCL_UHR);
  Wire1.begin();
  if (!rtc.begin(&Wire1)) 
  {
    Serial.println("RTC not found!");
    return;//while (1);
  }
  Serial.println("...done");
  delay(500);

  Serial.print("Initializing temperature sensor...");
  if (!TempPin10.search(addr10))
  {    
    TempPin10.reset_search();
    Serial.println("Temperature sensor not found!");
    return;
  }
  Serial.println("...done");
  delay(500);

  Serial.print("Initializing SD card...");
  bool sdInitialized = false;  
  SPI.setRX(_MISO);
  SPI.setTX(_MOSI);
  SPI.setSCK(_SCK);
  sdInitialized = SD.begin(_CS);  
  if (!sdInitialized) 
  {
    Serial.println("failed, or not present");
    return;
  }
  Serial.println("...card initialized.");
  Serial.println("Starting!");
  delay(500);
}

void loop() 
{  
  File dataFile = SD.open("Data_28-11-2025.txt", FILE_WRITE);
  String dataString = GetTime();

  lux = lightMeter.readLightLevel();

  status_Bewegung = digitalRead(GPIO_Bewegung);   
  digitalWrite(LED_BUILTIN, status_Bewegung);  
   
  GetTempData(&TempPin10, addr10, data10);
  temperatur = Data2Temp(data10);

  pegel = PegelShiftMedian(analogRead(analogPin)); 
  
  if (dataFile) 
  {
    Serial.print(dataString);
    Serial.print(status_Bewegung);
    Serial.print(",");
    Serial.print(pegel);
    Serial.print(",");
    Serial.print(lux);
    Serial.print(",");
    Serial.println(temperatur);

    dataFile.print(dataString);
    dataFile.print(status_Bewegung);
    dataFile.print(",");
    dataFile.print(pegel);
    dataFile.print(",");
    dataFile.print(lux);
    dataFile.print(",");
    dataFile.println(temperatur);
    dataFile.close();        
  }  
  else   
    Serial.println("error opening log file");  
  delay(2250);
}

String GetTime() 
{
  DateTime now = rtc.now();
  char buffer[21];  
  snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d,%02d/%02d/%04d,", now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());
  return String(buffer);
}

int PegelShiftMedian(int p) 
{
    // 1. Shift (FIFO)
    if (p > 0) 
    {
        for (int i = 1; i < SHIFTLEN; i++)
            pegelArray[i - 1] = pegelArray[i];
        pegelArray[SHIFTLEN - 1] = p;
    }

    // 2. Kopie zum Sortieren
    int temp[SHIFTLEN];
    for (int i = 0; i < SHIFTLEN; i++)
        temp[i] = pegelArray[i];

    // 3. Einfacher Bubble-Sort
    for (int i = 0; i < SHIFTLEN - 1; i++) 
    {
        for (int j = 0; j < SHIFTLEN - 1 - i; j++) 
        {
            if (temp[j] > temp[j+1]) 
            {
                int t = temp[j];
                temp[j] = temp[j+1];
                temp[j+1] = t;
            }
        }
    }

    // 4. Median berechnen
    if (SHIFTLEN % 2 == 1) 
    {
        // ungerade Anzahl → mittleres Element
        return temp[SHIFTLEN / 2];
    } else {
        // gerade Anzahl → Mittelwert der beiden mittleren
        int a = temp[SHIFTLEN/2 - 1];
        int b = temp[SHIFTLEN/2];
        return (a + b) / 2;
    }
}

int PegelShiftMittelwert(int p) 
{  
  if (p>0) 
  {
    for (int i=1; i<SHIFTLEN; i++)      
      pegelArray[i-1] = pegelArray[i];  
    pegelArray[SHIFTLEN-1] = p;
  }
  int summe = 0;
  for (int j=0; j<SHIFTLEN; j++) 
    summe += pegelArray[j];
  return summe / SHIFTLEN;
}

bool GetTempData(OneWire *TempPin, byte Adresse[], byte Data[]) 
{  
  bool present = false;
  TempPin->reset();
  TempPin->select(Adresse);
  TempPin->write(0x44, 1);
  delay(750);
  present = TempPin->reset();
  TempPin->select(Adresse);
  TempPin->write(0xBE);
  for (int i = 0; i < 9; i++) 
    Data[i] = TempPin->read();   
  return present; 
}

float Data2Temp(byte Data[])
{  
  int TReading = (Data[1] << 8) + Data[0];
  int SignBit = TReading & 0x8000;
  if (SignBit)
    TReading = (TReading ^ 0xffff) + 1;
  float Temp = TReading * 0.0625;
  if (SignBit)
    Temp *= -1.0;
  return Temp;
}