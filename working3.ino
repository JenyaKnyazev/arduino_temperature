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
struct node{
    int id;
    char *name;
    int temperature;
    node *next;
    node(int ID,char *NAME,int T){
        id=ID;
        name=NAME;
        temperature=T;
        next=NULL;
    }
};
struct vector_list{
    node *head;
    int len;
    int ID;
    vector_list(){
        head=NULL;
        len=0;
        ID=0;
    }
    void add(char *str,int id,int temp){
        node *element=new node(id,str,temp);
        if(head==NULL){
            head=element;
            len++;
            return;
        }
        node *run=head;
        while(run->next!=NULL)
            run=run->next;
        run->next=element;
        len++;
    }
    int del(int ID){
        node *prev=NULL;
        node *run=head;
        while(run!=NULL&&run->id!=ID){
            prev=run;
            run=run->next;
        }
        if(run==NULL||ID==0){
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("ERROR");
            lcd.setCursor(0,1);
            lcd.print("ID NOT EXIST");
            delay(2000);
            return 0;
        }
        prev->next=run->next;
        delete run;
        len--;
        return 1;
    }
    node& operator [](int index){
        node *run=head;
        while(index>0){
            run=run->next;
            index--;
        }
        return *run;
    }
};
vector_list vec;
void init_materials(){
    vec.add("PETG",0,270);
    vec.add("HDPE",0,200);
    vec.add("PC",0,250);
    vec.add("ABS",0,230);
    vec.add("PVC",0,180);
}

void add(int t){
    int i;
    vec.ID++;
    char *name = new char[17];
    sprintf(name,"TEMP_%d",vec.ID);
    vec.add(name,vec.ID,t);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ADD SUCCEDED");
    delay(2000);
}

void del(int id){
    int s=vec.del(id);
    if(s){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("DELETE SUCCEDED");
    }
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
            char *tempurary=vec[i].name;
            lcd.print(tempurary);
            lcd.setCursor(0,1);
            lcd.print(vec[i].temperature);
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
            play(vec[i].name,vec[i].temperature);
    }
}
void run(){
    int index=0;
    label:
    print(index-3);
    key = MatrixKeypad_waitForKey(keypad);
    if(key == '*'){
        index++;
        index%=(vec.len+3);
    }else if(key == '#')
        game(index-3);
    digitalWrite(pin_relay,HIGH);
    goto label;
}
void loop() {
   run();
}
