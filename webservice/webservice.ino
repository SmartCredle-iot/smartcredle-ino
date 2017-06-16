#include <Thread.h>

#include <Ultrasonic.h>
#include <Ethernet.h>
#include <SD.h>
#include <SPI.h>
#include <Servo.h>

int  c   =  3830;    // 261 Hz 
int  d   =  3400;    // 294 Hz 
int  e   =  3038;    // 329 Hz 
int  f   =  2864;    // 349 Hz 
int  g   =  2550;    // 392 Hz 
int  a   =  2272;    // 440 Hz 
int  b   =  2028;    // 493 Hz 
int  C   =  1912;    // 523 Hz 
// Define a special note, 'R', to represent a rest
int  R   =  0;

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

IPAddress ip{192, 168, 1, 22};
IPAddress mascara{255, 255, 255, 0};

EthernetServer server(8080);

File webFile; //biblioteca do cartao SD. 

#define TAMANHOCHAR 40
char HTTP_req[TAMANHOCHAR] = {0}; //Oque o cliente esta fazendo de REQUEST; nº de caracteres; inicializa com 0
char req_index = 0; //variavel para ajudar a ler o HTTP_req;

//Define os pinos
const int pinoSensor = A0;
const int pinoBuzzer = 3;
const int pinoTrig   = 5;
const int pinoEcho   = 6;
const int pinoServo  = 7;
const int pinoLed    = 8;

Ultrasonic ultrasonic(pinoTrig, pinoEcho); //Verificar para trocar a porta...

Servo Servo1; //Identifica um objeto do tipo Servo chamado Servo1

Thread myThreadServer = Thread();
Thread myThreadMusica = Thread();

//VARIAVEIS DE CONTROLE LIGADO/DESLIGADO
int iMusica    = 0;
int iBerco     = 0;
int iStatusLed = 0;

int Recebido; //Variável que armazenará o valor recebido pela serial
int posicao; //Variável que armazenará as posições do servo

char ComparaSTR(char *str, char *sFind){
  char found = 0;
  char index = 0;
  char len;

  len = strlen(str); //retorna o tamanho;

  if (strlen(sFind) > len){
     return 0;   
  }

  while (index < len){
     if (str[index] == sFind[found]){
        found++;
        if (strlen(sFind) == found){
           return 1;  
        } 
     } else {
        found = 0; 
     }

     index++;        
  }  
  return 0;
}

int verificaBerco(){
   boolean resp = false; 
   float cmSec  = 0;
   
   cmSec = ultrasonic.Ranging(CM);

   Serial.println("Centimetro:  ");
   Serial.print(cmSec);
   if (cmSec >= 10) {
     resp = 0;
   } else if (cmSec < 10) { 
     resp = 1; 
   } 

   return resp;
}


void lerDados(EthernetClient novoCliente){
   //realiza a leitura do sensor
   novoCliente.print(verificaBerco());
   novoCliente.println("|");

 
   //espero receber algo como 0|1   
}

void LimpaSTR(char *str, char iTamanho){
  for(int iFor = 0; iFor < iTamanho; iFor++ ){
     str[iFor] = 0;
  }  
}

void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(pinoBuzzer, HIGH);
    delayMicroseconds(tone);
    digitalWrite(pinoBuzzer, LOW);
    delayMicroseconds(tone);
  }
}

void playNote(char note, int duration) {
  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };
  
  // play the tone corresponding to the note name
  for (int i = 0; i < 8; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
}

void tocaMusica(){
   Serial.println("iniciou");
   if (iMusica == 0) { 
      char notes[] = "ccggaag ffeeddc ggffeed ggffeed ccggaag ffeeddc "; 
      int beats[]  = { 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 4 };
      int tempo    = 250;
      int iTamanho = 15;
     
      iMusica = 1; 
      for (int i = 0; i < iTamanho; i++) {      
          if (notes[i] == ' ') {
            delay(beats[i] * tempo); // rest
          } else {
            playNote(notes[i], beats[i] * tempo);
          }
        
          // pause between notes
          delay(tempo / 2); 
       }
   } else {
      iMusica = 0; 
      digitalWrite(pinoBuzzer, LOW);
   }

   delay(500);  
   Serial.println("final");
}


void ligaMotor(){  
  
 for(posicao = 90; posicao<110; posicao+=5){
    Servo1.write(posicao);
    Serial.println(posicao);
    delay(500);  
  }
  for(posicao = 110; posicao>=60; posicao-=5){
    Servo1.write(posicao);
    Serial.println(posicao);
    delay(500);  
  }
  
  for(posicao = 70; posicao<90; posicao+=5){
    Servo1.write(posicao);
    Serial.println(posicao);
    delay(500);  
  }
   
}

void statusLed(){
   if (iStatusLed == 0){
      digitalWrite(pinoLed, HIGH); 
      iStatusLed = 1;
   } else {
      digitalWrite(pinoLed, LOW);
      iStatusLed = 0;  
   }     
}

int statusPorta(){
    int valorSensor = 0;
    valorSensor = analogRead(pinoSensor);
    //int valorCorrigido = (valorSensor * 5) / 1024;
    //Serial.println(valorSensor);
  
    if(valorSensor > 0){
        Serial.println("Porta Aberta!!!");  
        Serial.print(valorSensor);      
    
    }else if (valorSensor == 0){
        Serial.println("Porta fechada!!!"); 
        Serial.print(valorSensor);    
    }
  
}

void threadServer(){
   EthernetClient client = server.available();  //Tratar cliente, verifica se possui um cliente
   if (client){
      boolean currentLineIsBlank = true; //ajuda a verificar se o cliente ja parrou de enviar notificação.

      while (client.connected()){ //enquanto o cliente estiver conectado.
         if (client.available()){ //verifica se possui informação vinda do cliente;
            char c = client.read(); //oque o cliente esta mandando..

            //preciso salvar todas as posições que vem do char;  
            if (req_index < (TAMANHOCHAR -1)){
               //é um array;
               HTTP_req[req_index] = c;
               req_index++;     
            } 

            if ((c == '\n') && (currentLineIsBlank)){ //verifica se oque o cliente mandou acabou. com o \n
               Serial.println("Servidor iniciado");
               //padrão para iniciar uma pagina HTML
               client.println("HTTP/1.1 200 OK");
               client.println("Content-Type: text/html");
               client.println("Connection: close");
               client.println();

               //Trata as REQ dos cliente;
               if (ComparaSTR(HTTP_req, "ajax_lerDados")){
                  lerDados(client);                
               } else if (ComparaSTR(HTTP_req, "ajax_statusMusica")){
                  tocaMusica();                       
               } else if (ComparaSTR(HTTP_req, "ajax_statusBercario")){
                  if (verificaBerco()){
                     Serial.println("ta no berco"); 
                  } else {
                     Serial.println("nao ta no berco");                                             
                  }                                 
               } else if (ComparaSTR(HTTP_req, "ajax_statusLuz")){
                  statusLed();              
               } else if (ComparaSTR(HTTP_req, "ajax_movimentaBerco")){
                  ligaMotor();               
               } else {                                 
                  client.println("<!DOCTYPE html>");
                  client.println("<htlm>");
                  client.println("<head>");
                  client.println("</head>");
                  client.println("<body>");
                  client.println("BEM VINDO");
                  client.println("</body>");                 
                  client.println("</html>"); 
               } 

               req_index = 0;
               LimpaSTR(HTTP_req, TAMANHOCHAR);
               break;     
            }

            if (c == '\n'){
               currentLineIsBlank = true;  
            } else if (c != '\r'){
               currentLineIsBlank = false; //ainda tem caracter a ser mandado              
            }                  
         }       
      } 

      delay(1);
      client.stop();    
      Serial.println("Fechou o servidor");
   }  
} 

void setup() {
  analogReference(INTERNAL);
  pinMode(pinoBuzzer, OUTPUT);
  pinMode(pinoLed, OUTPUT);
  
  Servo1.attach(pinoServo);
  
  // inicia modulo ethernet...
  Ethernet.begin(mac,ip);
  server.begin();
  Serial.begin(9600);
  Serial.println("iniciou"); 

  myThreadServer.onRun(threadServer);
  myThreadServer.setInterval(1);

  myThreadMusica.onRun(tocaMusica);
  myThreadMusica.setInterval(1);
  

}

void loop(){
   //if (myThreadMusica.shouldRun()) {
   //   myThreadMusica.run(); 
   //}
   tocaMusica;  
   if (myThreadServer.shouldRun()) {
      myThreadServer.run(); 
   }

   
}

