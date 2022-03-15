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
    volatile char names[25][17];//={"PETG","HDPE","PC","ABS","PVC"};
    volatile int temperatures[25];//={270,200,250,230,180};
    volatile int id_arr[25];
};
materials mat;
void init_materials(){
    char temp[5][17]={"PETG","HDPE","PC","ABS","PVC"};
    for(int i=0;i<5;i++){
        int r;
        for(r=0;temp[i][r]!='\0';r++)
            mat.names[i][r]=temp[i][r];
        mat.names[i][r]='\0';
    }
    mat.temperatures[0]=270;mat.temperatures[1]=200;
    mat.temperatures[2]=250;mat.temperatures[3]=230;
    mat.temperatures[4]=180;
    for(int i=0;i<5;i++)
        mat.id_arr[i]=0;
    for(int i=5;i<25;i++)
        mat.id_arr[i]=mat.temperatures[i]=0;
    mat.len=5;
    mat.id=0;
}
void add(int t){
    int i;
    ++mat.id;
    mat.temperatures[mat.len]=t;
    mat.id_arr[mat.len]=mat.id;
    sprintf(mat.names[mat.len],"TEMP_%d",mat.id);
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
    int index,r,i;
    if((index=is_exist_id(id))==0){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("ERROR");
        lcd.setCursor(0,1);
        lcd.print("ID NOT EXIST");
        delay(2000);
        return;
    }
    for(r=index;r<mat.len-1;r++){
        for(i=0;i<17;i++)
            mat.names[r][i]=mat.names[r+1][i];
        mat.temperatures[r]=mat.temperatures[r+1];
        mat.id_arr[r]=mat.id_arr[r+1];
    }
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
            char *tempurary=mat.names[i];
            lcd.print(tempurary);
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
      }else if(key=='*'&&i>0){
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
void run(){
    int index=0;
    label:
    print(index-3);
    key = MatrixKeypad_waitForKey(keypad);
    if(key == '*'){
        index++;
        index%=(mat.len+3);
    }else if(key == '#')
        game(index-3);
    digitalWrite(pin_relay,HIGH);
    goto label;
}
void loop() {
    run();
}
