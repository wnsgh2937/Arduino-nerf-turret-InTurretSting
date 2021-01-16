#include <ComponentObject.h>
#include <RangeSensor.h>
#include <SparkFun_VL53L1X.h>
#include <vl53l1x_class.h>
#include <vl53l1_error_codes.h>
#include <Wire.h>
#include "SparkFun_VL53L1X.h" //Click here to get the library: http://librarymanager/All#SparkFun_VL53L1X
#include <Servo.h>
#include <math.h>

#define SHUTDOWN_PIN 2
#define INTERRUPT_PIN 3
#define pi 3.14
Servo body;
Servo reload;
SFEVL53L1X distanceSensor;
/*
 * 150 30
 */
const int MAX_BODY_DEGREE = 90;
const int MIN_BODY_DEGREE = 50;
const int MAX_RELOAD_DEGREE = 40;
const int MIN_RELOAD_DEGREE =3;

const int SCANNING_COUNT_NUMBER=5;
const int SCANNING_ERROR_RANGE_PERCENTAGE=1;

const int ENEMY_DETECTING_COUNT_NUMBER=10;

const int SHORT_ENEMY_DETECTING_RANGE=200;/*10 50 150*/
const int MEDIUM_ENEMY_DETECTING_RANGE=200;
const int LONG_ENEMY_DETECTING_RANGE=200;

const int SHORT=500;
const int MEDIUM=1500;

const int TURRET_MODE=true;
const int CONTROL_MODE=false;

const int SCANNING_SPEED=1;
const int DETECTING_SPEED=1;

const int LED_RED=5;
const int LED_GREEN=6;
const int LED_BLUE=7;


int scan_map[181];

struct enemy_degree{
  int middle;
  int last;
};

void onControlMode(){
  
}
void LED_ON(int mode){
  int led_arr[3]={LED_RED,LED_GREEN,LED_BLUE};
  digitalWrite(LED_RED,HIGH);
  digitalWrite(LED_GREEN,HIGH);
  digitalWrite(LED_BLUE,HIGH);
  for(int i=0;i<3;i++){
    if(mode==i){
      digitalWrite(led_arr[i],LOW);
    }else{
      digitalWrite(led_arr[i],HIGH);
    }
  }

}
void smootheWrite(Servo servo,int current_degree,int set_degree){
  if(current_degree<set_degree){
    for(int d=current_degree;d<=set_degree;d++){
      servo.write(d);
      delay(22);
    }
  }else{
    for(int d=current_degree;set_degree<=d;d--){
      servo.write(d);
      delay(22);
    }
  }
}

int getSensorDistance(){
  distanceSensor.startRanging(); 
  while (!distanceSensor.checkForDataReady()){
    delay(1);
  }
  int distance= distanceSensor.getDistance();
  distanceSensor.clearInterrupt();
  distanceSensor.stopRanging();
  return distance;
}

bool checkMeasurementError(int distance_arr[SCANNING_COUNT_NUMBER],int distance_average){
  bool is_error=false;
  int error=(distance_average*SCANNING_ERROR_RANGE_PERCENTAGE)/100;
  int min_bound=distance_average-error;
  int max_bound=distance_average+error;
  for(int i=0;i<SCANNING_COUNT_NUMBER;i++){
    if(distance_arr[i]<min_bound || max_bound<distance_arr[i]){
      is_error=true;
    }
  }
  return is_error;
}

int getAverageDistance(){
      int distance_arr[SCANNING_COUNT_NUMBER];
      int average_distance;
      bool is_error=true;
      while(is_error){
        for(int i=0;i<SCANNING_COUNT_NUMBER;i++){
          distance_arr[i]= getSensorDistance();
        }
        int distance_sum=0;
        for(int i=0;i<SCANNING_COUNT_NUMBER;i++){
          distance_sum+=distance_arr[i];
        }

        average_distance=distance_sum/SCANNING_COUNT_NUMBER;
        is_error=checkMeasurementError(distance_arr,average_distance);
      }
      return average_distance;   
}

void scanning(){
  LED_ON(2);
  int before_body_degree=MIN_BODY_DEGREE;
  for(int degree=MIN_BODY_DEGREE;degree<=MAX_BODY_DEGREE;degree+=SCANNING_SPEED){
      smootheWrite(body,before_body_degree,degree);
      before_body_degree=degree;
      delay(100);
      scan_map[degree]=getAverageDistance();
      Serial.print(degree);
      Serial.print("도 :");
      Serial.print(scan_map[degree]);
      Serial.print("\n");
  }
}

void shot(int shot_count){
  Serial.print("Shot ON");
  for(int i=0;i<shot_count;i++){
    reload.write(MIN_RELOAD_DEGREE);
    delay(300);
    reload.write(MAX_RELOAD_DEGREE);
    delay(300);
  }
  reload.write(MIN_RELOAD_DEGREE);
  delay(1000);
}


int getEnemyAverageDistance(){
  int average_distance;
  int sum=0;
  Serial.print("\n[");
  for(int i=0;i<ENEMY_DETECTING_COUNT_NUMBER;i++){
    int dist=getSensorDistance();
    sum+= dist;
    Serial.print(dist);
    Serial.print(" ");
  }
  Serial.print("]\n");    
  average_distance=sum/ENEMY_DETECTING_COUNT_NUMBER;
  return average_distance;   
}

int getRange(int dist){
  if(dist<SHORT){
      return SHORT_ENEMY_DETECTING_RANGE;
  }else if(dist<MEDIUM){
      return MEDIUM_ENEMY_DETECTING_RANGE;
  }else{
      return LONG_ENEMY_DETECTING_RANGE;
  }
}

enemy_degree* find_enemy_middle_degree(bool is_plus,int body_degree){
  enemy_degree* e_d=(enemy_degree*)malloc(sizeof(enemy_degree));
  int first_body_degree=body_degree;
  int current_body_degree=body_degree;
  while(MIN_BODY_DEGREE<current_body_degree||current_body_degree<MAX_BODY_DEGREE){
    body.write(current_body_degree+1);
    delay(22);
    int dist=getSensorDistance();
    int range=getRange(dist);
    int scan_dist=scan_map[current_body_degree];
    Serial.print("\ndist :");
    Serial.print(dist); 
    Serial.print("\nrng : ");
    Serial.print(range);
    Serial.print("\nscan_dist:");
    Serial.print(scan_dist);
    Serial.print("\n");
    
    if(dist<scan_dist-range){
      if(is_plus){
        current_body_degree++;
      }else{
        current_body_degree--;
      }
    }else{
      break;
    }
  }
  
  int middle=(first_body_degree+current_body_degree)/2;
  e_d->last=current_body_degree;
  e_d->middle=middle;
  return e_d;
}

void detecting(){
  int body_degree=MIN_BODY_DEGREE;
  int before_body_degree=MIN_BODY_DEGREE;
  bool is_plus=true;
  while(true){
    LED_ON(1);
    Serial.print("-------------------------------\n");
    smootheWrite(body,before_body_degree,body_degree);
    before_body_degree=body_degree;
    //body.write(body_degree);
    delay(100);
    int sensor_dist=getSensorDistance();
    int scan_dist=scan_map[body_degree];
    Serial.print("\nbody_degree :");
    Serial.print(body_degree); 
    Serial.print("\nsensor dist : ");
    Serial.print(sensor_dist);
    Serial.print("\nscan_dist:");
    Serial.print(scan_dist);
    Serial.print("\n");
    int range=getRange(sensor_dist);
    
    if(sensor_dist<scan_dist-range){
      LED_ON(0);
      int enemy_dist=getEnemyAverageDistance();
      
      range=getRange(enemy_dist);
      
      if(enemy_dist < scan_dist-range){
        enemy_degree* e_d=find_enemy_middle_degree(is_plus,body_degree);
        int middle_body_degree=e_d->middle;
        int last_body_degree=e_d->last;
        free(e_d);
        Serial.print("\nmbd : ");
        Serial.print(middle_body_degree);
        Serial.print("\nlbd:");
        Serial.print(last_body_degree);
        Serial.print("\n");

    
        smootheWrite(body,body_degree,middle_body_degree);
        shot(10);        
        before_body_degree=middle_body_degree;
        body_degree=last_body_degree;
      }
    }
    if(is_plus){
      body_degree+=DETECTING_SPEED;
    }else{
      body_degree-=DETECTING_SPEED;
    }
    if(MAX_BODY_DEGREE<body_degree){
      body_degree-=DETECTING_SPEED;
      is_plus=false;
    }else if(body_degree<MIN_BODY_DEGREE){
      body_degree+=DETECTING_SPEED;
      is_plus=true;
    }
    Serial.print("-------------------------------\n");
  }
}


void onTurretMode(){
   scanning();
   detecting();
}

void setup() {

  Wire.begin();

  Serial.begin(9600);
  Serial.println("VL53L1X Qwiic Test");

  if (distanceSensor.begin() != 0) //Begin returns 0 on a good init
  {
    Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
    while (1);
  }
  Serial.println("Sensor online!");
   
  body.attach(9);
  reload.attach(10);

  pinMode(LED_RED,OUTPUT);
  pinMode(LED_GREEN,OUTPUT);
  pinMode(LED_BLUE,OUTPUT);
  
  
  body.write(MIN_BODY_DEGREE);
  reload.write(MAX_RELOAD_DEGREE);
  reload.write(MIN_RELOAD_DEGREE);
  
  delay(1000);
  if(TURRET_MODE){
    onTurretMode();
  }else{
    onControlMode();
  }
}

void loop() { 
   
}
