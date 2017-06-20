#include <Wire.h>
#include <Servo.h> 

const int pinoServo = 7;

char sStatusMotor   = "";
int  posicao = 90;

Servo servoBerco;

void setup() {
  Wire.begin(8);              
  Wire.onReceive(receiveEvent); 
  Serial.begin(9600);           
  
  servoBerco.attach(pinoServo);
}

void loop() {
   Serial.print(sStatusMotor);
   if (sStatusMotor == '1'){
      loopMotor(1); 
   } else {
      loopMotor(0); 
   }
   
   delay(1);  
}

void receiveEvent(int howMany) {
  while (1 < Wire.available()) { 
    sStatusMotor = Wire.read();
    Serial.print(sStatusMotor);         
  }

  int x = Wire.read();    // receive byte as an integer 
}

void loopMotor(int iLigaMotor){
  if (iLigaMotor == 1){ 
      for(posicao = 90; posicao<110; posicao+=5){
        servoBerco.write(posicao);
        delay(400);  
      }
      
      for(posicao = 110; posicao>60; posicao-=5){
        servoBerco.write(posicao);
        delay(400);  
      }
      
      for(posicao = 60; posicao<90; posicao+=5){
        servoBerco.write(posicao);
        delay(400);  
      }  
  } else {
     if (posicao >= 90){
         for(posicao = posicao; posicao<90; posicao+=5){
          servoBerco.write(posicao);
          delay(400);  
        }       
     } else {
        for(posicao = posicao; posicao>90; posicao-=5){
          servoBerco.write(posicao);
          delay(400);  
        }   
     }  
  }
  
}
