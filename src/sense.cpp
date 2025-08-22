#include "sense.h"

VL53L0X l0x;
VL53L1X l1x;
Ultrasonic usnd(TX2,RX2,1000000);

volatile uint32_t dist_ultsnc,dist_l0x;
volatile bool ultsnc_finish, l0x_finish;

bool _init_bus(){
    Wire.begin(SDA_L1X, SCL_L1X);
    Wire.setClock(400000);
    delay(100);
    return true;  // 暂时只初始化一个Wire
}

void test_sensor(){
	Wire.beginTransmission(0x29);
	Serial.printf("%d\n",Wire.endTransmission(1));
    uint16_t range;
    while(1){
		Serial.printf("1x:%d us:%d\n",l1x.read(),usnd.read());continue;

        range = l1x.read();
        if (l1x.timeoutOccurred()) {
            Serial.println("TIMEOUT");
        } else {
            Serial.printf("Range: %d mm\n", range);
        }
        delay(100);
    }
}

bool init_sensor(){
    if(!_init_bus()){
        Serial.println("Bus init failed!");
        return false;
    }
    
    delay(100); // 等待总线稳定
    
    l1x.setTimeout(500);
    if(!l1x.init()){
        Serial.println("L1X init failed!");
        return false;
    }
    
    l1x.setDistanceMode(VL53L1X::Long);
    l1x.setMeasurementTimingBudget(60000);
    l1x.startContinuous(60);
#if false
	if(!l0x.init()){
		Serial.println("L0X init failed!");
		return false;
	}
	l0x.setMeasurementTimingBudget(35000);
	l0x.startContinuous(50);
#endif
    Serial.println("Sensor initialized successfully!");
    return true;
}

void task_measure_split(void *pvParameters){
	dist_ultsnc=usnd.read()*1000;
	delay(60);
	dist_ultsnc+=usnd.read()*1000;
	dist_ultsnc/=2;
	ultsnc_finish=1;
	vTaskDelete(NULL);
}

double measure_once(){
	l0x_finish=ultsnc_finish=0;
	xTaskCreatePinnedToCore(
		task_measure_split,"task_measure_split",
		2048,NULL,10,NULL,
		0
	);
	uint32_t dt_l1x=l1x.read();
	// DISABLED: dist_l0x=l0x.readRangeContinuousMillimeters();
	while(!ultsnc_finish){vTaskDelay(1);}
	if(dt_l1x<4001&&dt_l1x>5){ // LONG valid
		if(dist_l0x>2001||dist_l0x<5){ // SHORT out of range
			return dt_l1x*0.9+dist_ultsnc*0.1;
		}
		return dt_l1x*0.3+dist_l0x*0.6+dist_ultsnc*0.1;
	}else{ // LONG invalid
		if(dist_l0x>2001||dist_l0x<5){ // SHORT out of range
			return 0; // Ultrasonic not so trusted
		}
		return dt_l1x*0.7+dist_ultsnc*0.3;
	}
}