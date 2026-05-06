/*****************************************************************
File:        readTemperatureAndHumidityWithIIC
Description: Power on to obtain the equipment information once, 
             including (SN/PID/VER), and then repeatedly obtain 
             the temperature and humidity value through IIC and 
             display the value in the serial port and OLED screen.
******************************************************************/
#include "BM25S2021-1.h"
#include "BMD31M090.h"
#include "Bitmap.h"

#define BMD31M090_WIDTH   128        // BMD31M090 Module display width, in pixels
#define BMD31M090_HEIGHT  64         // BMD31M090 Module display height, in pixels
#define BMD31M090_ADDRESS 0x3C       // See Schematic "I2C Addr SEL" for address setting,
BM25S2021_1 BMht(&Wire);
BMD31M090   BMD31(BMD31M090_WIDTH, BMD31M090_HEIGHT, &Wire1);

void setup() 
{
    BMD31.begin(BMD31M090_ADDRESS);
    BMht.begin(); 
    BMD31.clearDisplay();
}

void loop() 
{
    float humidity = BMht.readHumidity();
    float temperature = BMht.readTemperature();

    BMD31.drawString(10, 1,(u8*)"  Humi:");
    BMD31.drawString(10, 3, (u8*)"  Temp:");
    BMD31.drawString(110, 1,(u8*)"%");
    BMD31.drawString(110, 3, (u8*)"C");


    // 显示新的温湿度数值
    char humidityStr[10];
    dtostrf(humidity, 4, 2, humidityStr);
    BMD31.drawString(70, 1, (u8*)humidityStr);

    char temperatureStr[10];
    dtostrf(temperature, 4, 2, temperatureStr);
    BMD31.drawString(70, 3, (u8*)temperatureStr);
        

    if ((temperature >= 18 && temperature <= 28) && (humidity >= 45 && humidity <= 55)) {
        BMD31.drawString(10, 6, (u8*)"  suitable!     ");
    } 
    else {
        BMD31.drawString(10, 6, (u8*)"  unsuitable     ");
    }

    delay(200);
}