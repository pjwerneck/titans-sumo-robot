
/*
Pinagem

Servo D - 6
Servo E - 7
Sensor D - A1
Sensor E - A2

Sensor Ultrassonico Echo - 11
Sensor Ultrassonico Trig - 12


Botoes

btn1 - entra no modo de Combate
btn2 - entra no modo de Calibragem
btn3 - entra no modo de Ajuste dos servos


Dependencias

https://arduino-new-ping.googlecode.com/files/NewPing_v1.5.zip


*/



#include <NewPing.h>
#include <Servo.h>


#define TRIGGER_PIN  12  // trigger pin on the ultrasonic sensor
#define ECHO_PIN     11  // echo pin on the ultrasonic sensor
#define MAX_DISTANCE 100 // maximum distance in centimeters
#define BUTTONS_PIN A0

#define RIGHT_WHEEL_PIN 6 // pin used for right wheel servo
#define LEFT_WHEEL_PIN  7 // pin used for left wheel servo

#define RIGHT_SENSOR_PIN A1 // pin used for right opto-sensor
#define LEFT_SENSOR_PIN A2 // pin used for left opto-sensor

#define RIGHT 0 // index for right side in wheel and sensor data structures
#define LEFT 1  // index for left side in wheel and sensor data structures



Servo wheels[2];

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);


int sline[2];

int smax[2] = {0, 0};
int smin[2] = {1023, 1023};

int sf = 0;

unsigned int distance = 0;

unsigned int ping_speed = 50;
unsigned long ping_timer;

unsigned long search_timer;


// 0: calibrate
// 1: run
// 2. servo adjusting
int mode = 0;


void move_f(){
  wheels[RIGHT].write(0);
  wheels[LEFT].write(180);
}

void move_b(){
  wheels[RIGHT].write(180);
  wheels[LEFT].write(0);
}

void move_l(){
  wheels[RIGHT].write(0);
  wheels[LEFT].write(0);
}

void move_r(){
  wheels[RIGHT].write(180);
  wheels[LEFT].write(180);
}

void move_stop(){
  wheels[RIGHT].write(90);
  wheels[LEFT].write(90);
}

void read_sensors(){
  sline[RIGHT] = map(analogRead(A1), smin[RIGHT], smax[RIGHT], 0, 100);
  sline[LEFT] = map(analogRead(A2), smin[LEFT], smax[LEFT], 0, 100);
  
  read_sonar();
  
}

void read_sonar(){
  if (millis() >= ping_timer){
    ping_timer += ping_speed;
    sonar.ping_timer(echo_callback);
  }
}
  

void echo_callback(){
  
  if (sonar.check_timer()){
    distance = sonar.ping_result / US_ROUNDTRIP_CM;
  }
}


void calibrate(){
  digitalWrite(13, HIGH);
  
  sline[RIGHT] = analogRead(A1);
  sline[LEFT] = analogRead(A2);
    
   
  for (int i=0; i < 2; i++){
    if (sline[i] > smax[i]){
      smax[i] = sline[i];
    }    
    if (sline[i] < smin[i]){
      smin[i] = sline[i];
    }    
  }

  digitalWrite(13, LOW);   
}

void check_mode()
{
  int a_btn = analogRead(BUTTONS_PIN);
  
  if ((a_btn < 350) && (a_btn > 250)){
    mode = 0;
  }

  if ((a_btn < 550) && (a_btn > 450)){
    mode = 1;
  }    
  
  if ((a_btn < 150) && (a_btn > 50)){
    mode = 3;
  }

}
  


void setup()
{
  
  randomSeed(analogRead(A4));
  
  wheels[RIGHT].attach(RIGHT_WHEEL_PIN);
  wheels[LEFT].attach(LEFT_WHEEL_PIN);
  move_stop();
  
  ping_timer = millis();
  search_timer = 0;
  
  Serial.begin(9600); 
  
  pinMode(13, OUTPUT);
}


void loop()
{  
  check_mode();
  
  Serial.print("M ");
  Serial.println(mode);
  
  if (mode == 0){
    calibrate();
  }
  
  if (mode == 1){
    run();
  }
  
  if (mode == 2){
    search();
  }
  
  if (mode == 3){
    motor_tuning();
  }
  
  delay(10);
    
}


void motor_tuning(){
  wheels[0].write(90);
  wheels[1].write(90);
}
  

void search(){
  // if both on border, move right, choose a random side to move to

  if (search_timer == 0){
    search_timer = millis();
  }
    
 if (millis() - search_timer > 1000){
   search_timer = 0;
   mode = 1;
   return;
 }

  if ((sline[0] < 50) && (sline[1] < 50)){
    move_r();
  }
      
  else {
  // otherwise, move to the other side
    if (sline[RIGHT] < 50){
      move_l();
    }
    
    if (sline[LEFT] < 50){
      move_r();
    }
  }
  
  read_sonar();
  
  if ((distance > 0) && (distance < 80)){
    mode = 1;
  }
  
}



void run(){
  
  read_sensors();
  read_sonar();
 
  Serial.print("S ");
  Serial.print(sline[0]);
  Serial.print(", ");
  Serial.println(sline[1]);
  Serial.print("D ");
  Serial.println(distance);
  
  
  // over black. move forward.
  if ((sline[0] > 50) && (sline[1] > 50)){
    Serial.println("m");
    move_f();
    sf = 0;
    
  }   

  // over white border
  if ((sline[0] < 50) || (sline[1] < 50)){

    // stop and enter search mode   
    move_stop();
    
    mode = 2;
  }
  
}
   

void lero(){
    if (sf == 0){    
      // if both on border, choose a random side to move to
      if ((sline[0] < 50) && (sline[1] < 50)){
        // choose a random side to move to
          sf = random(1, 3);
        }    
      
      // otherwise, move to the other side
      else if (sline[0] < 50){
        sf = 0;
      }
      
      else if (sline[1] < 50){
        sf = 1;
      }
    }
      // rotate to the choosen side until full retreat
    
    while ((sline[0] < 50) || (sline[1] < 50)){
      if (sf == 1){
        move_r();
      } else {
        move_l();
      }
      
       delay(100);
       
       move_stop();
      
       read_sensors();

    }
    
   
} 
  
  


  
    
