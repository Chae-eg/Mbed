#include "mbed.h"
#include "motordriver.h"
#include "Adafruit_SSD1306.h"
#include "DHT22.h"
#include "Oled_Mode.h"

#define NOMALMODE							Ocon.mode = 1;
#define PASSMODE							Ocon.mode = 2;
#define DOORMODE							Ocon.mode = 3;
#define NOTE_C_DELAY   				1911										//half period(usec)
#define BUZZER_ON      				buzzer = 0.5
#define BUZZER_OFF     				buzzer = 0
#define JS_NEUTRAL_VALUE  		54
#define INIT_PASS							PASSWD = 0000          //initial password
#define CORR_PASS							0000
#define BUZZER_PLAYTIME 			1.0
#define BUZZER_PLAY_INTERVAL 	3.0
#define DOOR_TIME 						30.0

DigitalIn InputBtn(PA_14);				//btn1
DigitalIn ResetBtn(PB_7);					//btn2
DigitalIn DoorBtn(PC_4); 					//btn3

AnalogIn jsX(PC_2);
AnalogIn jsY(PC_3);

DigitalOut G_Led(PA_13);
DigitalOut Y_Led(PB_10);
DigitalOut R_Led(PA_4);

bool CanControl;

DHT22 dht22(PB_2);
PwmOut buzzer(PC_9);
Motor motorA(PA_7 , PC_8);
Oled_Mode Ocon(true);

Ticker Condition_tim;
Ticker jsTicker;

Timeout Door_tim;
Timeout buzzerTimer;

void TimeOver();

int PASSWD;
int x, y ;
int PW_digit =1000;                   //initial password digiti value

void measureJoystick(){
	x = (int)(jsX * 100) - JS_NEUTRAL_VALUE;
	if (abs(x) <= 3) x=0;
	
	y = (int)(jsY * 100) - JS_NEUTRAL_VALUE;
	if (abs(y) <= 3) y=0;
}

void SetPW() {
	if (y<0& (PASSWD/PW_digit)%10!=9){
		Y_Led = 1;
		PASSWD += PW_digit;
  }
  else if (y>0&(PASSWD/PW_digit)%10!=0){
		Y_Led = 1;
		PASSWD -= PW_digit;
  }
	Ocon.ChangePass(PASSWD);
}
void Password(){ //curssor
			Y_Led = 0;
    if (x > 0 & PW_digit !=1){
      PW_digit = PW_digit / 10;
			Ocon._cusor++;
    }
    else if (x < 0 & PW_digit!=1000) {
      PW_digit = PW_digit * 10;
			Ocon._cusor--;
    }
		SetPW();
}

void BuzzerSound(){
static int buzOn = 0 ;
	if(!buzOn){
		BUZZER_ON;
		buzOn=1;
		buzzerTimer.attach(BuzzerSound,1.0);
	}
	else{
		BUZZER_OFF;
		buzOn=0;
		buzzerTimer.attach(BuzzerSound,3.0);
	}
}

void DoorCon(){
	if(CanControl){
		DOORMODE;
		BUZZER_ON;	
		R_Led = 1;	
		Ocon.DetectMode();
		if(Ocon.DoorState){
			motorA.backward(0.5); //closing
		}
		else{
			motorA.forward(0.5); //opening
		}
		wait(2.0);
		R_Led = 0;
		BUZZER_OFF;
		NOMALMODE;
		Ocon.DoorState=!Ocon.DoorState;
		motorA.stop();
		}
		else{ // need to input password
			PASSMODE;
			jsTicker.attach(measureJoystick,0.2);
	}
}

void TimeOver(){
		Ocon.RemoveDisplay();
	if(Ocon.DoorState) DoorCon();
	CanControl=false;
}

void CorrPass(){
	if (PASSWD == CORR_PASS){
		CanControl=true;
		DoorCon();
		jsTicker.detach();
		Door_tim.attach(TimeOver,DOOR_TIME);
	}
		INIT_PASS;
		Ocon.ChangePass(PASSWD);
}

void getTemHum(){
	if(dht22.sample()){
			Ocon._TEMP = dht22.getTemperature() /10.0f ;
			Ocon._HUM=dht22.getHumidity()/10.0f;
		}
}

void setup(){
	pc.baud(115200);
	getTemHum();
	motorA.stop();
	Ocon.DoorState=false;
	NOMALMODE;
	CanControl=false;
	Condition_tim.attach(getTemHum,3.0);

	INIT_PASS;
  G_Led = 0;
  Y_Led = 0;
  R_Led = 0;	

	buzzer.period_us(NOTE_C_DELAY * 2);
}

int main()
{   	
	setup();
	while(1){
		if(Ocon.mode==1){
			if(!DoorBtn) DoorCon();
		}
		else if(Ocon.mode==2){
			Password();
			if(!InputBtn)		CorrPass();
			else if (!ResetBtn) 	INIT_PASS;
			else if (!DoorBtn) 	NOMALMODE;
		}
		Ocon.DetectMode();
        wait(0.3);
    }
	}

	

