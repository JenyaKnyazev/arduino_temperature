#include <Wire.h>
#include "max6675.h"
#include <LiquidCrystal_I2C.h>
#include <stdlib.h>
#include "MatrixKeypad.h"
#include <stdint.h>
int soPin = 2;//so serial out
int csPin = 3; // cs chip select cs pin
int sckPin = 4;//sck serial clock pin
MAX6675 Module(sckPin,csPin,soPin);
int lcd_sda_pin = 4;
int lcd_scl_pin = 5;
LiquidCrystal_I2C lcd(0x27,16,2);
int pin_relay = 13;
const uint8_t rown = 4; 
const uint8_t coln = 4; 
uint8_t rowPins[rown] = {12, 11, 10, 9}; 
uint8_t colPins[coln] = {8, 7, 6, 5};
char keymap[rown][coln] = 
  {{'1','2','3','A'}, 
   {'4','5','6','B'},
   {'7','8','9','C'},
   {'*','0','#','D'}};
char key;
MatrixKeypad_t *keypad;
struct materials{
    volatile int len = 5;
    volatile int id = 0;
    char** names;//={"PETG","HDPE","PC","ABS","PVC"};
    int* temperatures;//={270,200,250,230,180};
    int* id_arr;
};
materials mat;
void init_materials(){
    mat.names=(char**)malloc(sizeof(char*)*5);
    mat.temperatures=(int*)malloc(sizeof(int)*5);
    mat.names[0]="PETG";mat.names[1]="HDPE";
    mat.names[2]="PC";mat.names[3]="ABS";
    mat.names[4]="PVC";
    mat.temperatures[0]=270;mat.temperatures[1]=200;
    mat.temperatures[2]=250;mat.temperatures[3]=230;
    mat.temperatures[4]=180;
    mat.id_arr=(int*)calloc(5,sizeof(int));
    mat.len=5;
}
void add(int t){
    int len=mat.len+1;
    int i;
    char **n = (char**)malloc(sizeof(char*)*len);
    int* n2 = (int*)malloc(sizeof(int)*len);
    int* n3 = (int*)malloc(sizeof(int)*len);
    for(i=0;i<len-1;i++){
        n[i]=mat.names[i];
        n2[i]=mat.temperatures[i];
        n3[i]=mat.id_arr[i];
    }
    n[i]=(char*)malloc(sizeof(char)*16);
    n2[i]=t;
    n3[i]=++mat.id;
    sprintf(n[i],"TEMP_%d",mat.id);
    free(mat.names);
    free(mat.temperatures);
    free(mat.id_arr);
    mat.names=n;
    mat.temperatures=n2;
    mat.id_arr=n3;
    mat.len++;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ADD SUCCEDED");
    delay(2000);
}
int is_exist_id(int id){
    int i;
    for(i=5;i<mat.len;i++)
        if(mat.id_arr[i]==id)
            return i;
    return 0;
}
void del(int id){
    int index;
    if((index=is_exist_id(id))==0){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("ERROR");
        lcd.setCursor(0,1);
        lcd.print("ID NOT EXIST");
        delay(2000);
        return;
    }
    int len=mat.len-1;
    int i,r;
    char **n = (char**)malloc(sizeof(char*)*len);
    int* n2 = (int*)malloc(sizeof(int)*len);
    int* n3 = (int*)malloc(sizeof(int)*len);
    for(i=0;i<index;i++){
        n[i]=mat.names[i];
        n2[i]=mat.temperatures[i];
        n3[i]=mat.id_arr[i];
    }
    for(r=index+1;r<mat.len&&i<len;r++,i++){
        n[i]=mat.names[r];
        n2[i]=mat.temperatures[r];
        n3[i]=mat.id_arr[r];
    }
    free(mat.names);
    free(mat.temperatures);
    free(mat.id);
    mat.names=n;
    mat.temperatures=n2;
    mat.id_arr=n3;
    mat.len--;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("DELETE SUCCEDED");
    delay(2000);
}
void setup() {
  init_materials();
  pinMode(pin_relay,OUTPUT);
  digitalWrite(pin_relay,HIGH);
  keypad = MatrixKeypad_create((char*)keymap , rowPins, colPins, rown, coln); 
  lcd.init();
  lcd.backlight();
}
void print(int i){
    lcd.clear();
    lcd.setCursor(0,0);
    switch(i){
        case -3:
            lcd.print("ADD");
            break;
        case -2:
            lcd.print("DELETE");
            break;
        case -1:
            lcd.print("MANUAL");
            break;
        default:
            lcd.print(mat.names[i]);
            lcd.setCursor(0,1);
            lcd.print(mat.temperatures[i]);
    }
}
int input(){
    char num[17];
    int i=0,r;
    num[i]='\0';
    do{
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("tap id or temp:");
      lcd.setCursor(0,1);
      for(r=0;r<i;r++){
          int k = num[r]-'0';
          lcd.print(k);
      }
      key = MatrixKeypad_waitForKey(keypad);
      if(key>='0'&&key<='9'){
          num[i]=key;
          i++;
          num[i]='\0';
      }else if(key=='*'){
          i--;
          num[i]='\0';
      }
      delay(250);
    }while(key!='#'||i<1);
    return atoi(num);
}
void play(char *str,int temp){
    int d;
    do{
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(str);
        lcd.print(" ");
        lcd.print(temp);
        lcd.setCursor(0,1);
        d = Module.readCelsius();
        lcd.print(d);
        delay(250);
        if(d>=temp+10)
            digitalWrite(pin_relay,HIGH);
        else if(d<=temp-10)
            digitalWrite(pin_relay,LOW);
        MatrixKeypad_scan(keypad);
    }while(MatrixKeypad_hasKey(keypad)==false);
}
void game(int i){
    switch(i){
        case -3:
            add(input());
            break;
        case -2:
            del(input());
            break;
        case -1:
            play("MANUAL",input());
            break;
        default:
            play(mat.names[i],mat.temperatures[i]);
    }
}
int len(){
    return mat.len;
}
void run(){
    int index=0;
    label:
    print(index-3);
    key = MatrixKeypad_waitForKey(keypad);
    if(key == '*'){
        index++;
        index%=(len()+3);
    }else if(key == '#')
        game(index-3);
    digitalWrite(pin_relay,HIGH);
    goto label;
}
void loop() {
    run();
}
