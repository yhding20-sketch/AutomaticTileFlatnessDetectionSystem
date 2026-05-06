/*************************************************
File:             Example_twoStepperRun.ino
Description:      The expansion board performs the following actions 
                  1.Stepper motor 1 rotates clockwise at a speed of 200 steps/s, and stepper motor 2 rotates counterclockwise at a speed of 800 steps/s for 3 seconds
                  2.Stepper motor stops for 1 second
                  3.Stepper motor 1 rotates counterclockwise at a speed of 200 steps/s, and stepper motor 2 rotates clockwise at a speed of 800 steps/s for 3 seconds
                  4.Stepper motor stops for 1 second
                 
Note:               
**************************************************/
#include <BMP73T104.h>
#include "BMD31M090.h"
#include "Bitmap.h"
#include "BML36M001.h"

#define D0 28   //发送信号
 
BMP73T104 myStepper1(2,StepMotor);          //Constructing the Stepper Motor Object
BMP73T104 myStepper2(2,StepMotor);          //Constructing the Stepper Motor Object
 
BML36M001 BML36(22,&Wire1);

void setup() { 
  pinMode(D0, OUTPUT);
  digitalWrite(D0, LOW);
  myStepper1.begin(STEP1,HALF_STEP);        //Stepper motor 1 initialization
  myStepper1.setStepperMaxSpeed(1200);
  myStepper1.setStepperAcceleration(800);

  myStepper2.begin(STEP2,HALF_STEP);        //Stepper motor 2 initialization
  myStepper2.setStepperMaxSpeed(1200);
  myStepper2.setStepperAcceleration(800);

  Serial.begin(115200);                              // 修改波特率为9600
  BML36.begin();                                    //Initialize BML36M001   
  BML36.setDistanceModeLong();                         //Set the probe mode to be long-distance.  Detection range:40~4000(max)（Unit: mm）                                                
  BML36.startRanging();                                //Start measuring the distance


}

void loop() {


  //加激光测距
  if (BML36.getINT())                               //Ask if the data is ready  0：data is not ready 1：data is ready
  {
   Serial.print("Distance(mm):");                               //Print string:“Distance(mm):”        
   Serial.println(BML36.readDistance());                        //Gets distance data（Unit: mm）
   while (BML36.readDistance() >100)
   {
delay(100);
   }

   //FIRST
    myStepper2.stepperRun(CW,600); 
    delay(4000);
    myStepper2.stepperStop();
    delay(5000);//第1次传输
    digitalWrite(D0, HIGH);  // 高电平
    delay(700);               // 保持50ms
    digitalWrite(D0, LOW);   // 恢复低电平
    delay(100);
    myStepper2.stepperRun(CCW,600); 
    delay(1000);
    myStepper2.stepperStop();
    delay(100);

    //SECOND
    myStepper1.stepperRun(CCW,600); 
    delay(2000);
    myStepper1.stepperStop(); 
    delay(100);
    myStepper2.stepperRun(CW,600); 
    delay(1000);
    myStepper2.stepperStop();
    delay(5000);//第2次传输
    digitalWrite(D0, HIGH);  // 高电平
    delay(700);               // 保持50ms
    digitalWrite(D0, LOW);   // 恢复低电平
    delay(100);
    myStepper2.stepperRun(CCW,600); 
    delay(1000);
    myStepper2.stepperStop();
    delay(100);

  //THIRD
    myStepper1.stepperRun(CCW,600); 
    delay(2000);
    myStepper1.stepperStop(); 
    delay(100);
    myStepper2.stepperRun(CW,600); 
    delay(1000);
    myStepper2.stepperStop();
    delay(5000);//第3次传输
    digitalWrite(D0, HIGH);  // 高电平
    delay(700);               // 保持50ms
    digitalWrite(D0, LOW);   // 恢复低电平
    delay(100);
    myStepper2.stepperRun(CCW,600); 
    delay(4000);
    myStepper2.stepperStop();
    delay(100);

  //BACK
   myStepper1.stepperRun(CW,600); 
   delay(4000);
  myStepper1.stepperStop();
  delay(100); 
   
   BML36.clearInterrupt();                                             //Clear the BML36M001 interrupt flag bit   
  } 


}