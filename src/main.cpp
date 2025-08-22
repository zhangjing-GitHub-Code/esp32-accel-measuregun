#include <Arduino.h>
#include "sense.h"

#define MOVING_AVERAGE_SIZE 5
double pos_buffer[MOVING_AVERAGE_SIZE] = {0};
int buffer_index = 0;
unsigned long last_update = 0;
const int MIN_UPDATE_INTERVAL = 100; // 最小更新间隔(ms)
const double MIN_POS_CHANGE = 0.0005; // 最小位置变化(m)，小于此值认为静止
const double MAX_ACCEL = 20.0; // 最大加速度限制(m/s^2)

double x0,v0,t0;
double get_moving_average(double new_value) {
    pos_buffer[buffer_index] = new_value;
    buffer_index = (buffer_index + 1) % MOVING_AVERAGE_SIZE;
    
    double sum = 0;
    for(int i = 0; i < MOVING_AVERAGE_SIZE; i++) {
        sum += pos_buffer[i];
    }
    return sum / MOVING_AVERAGE_SIZE;
}

void update_accel(double measure_mm) {
    unsigned long current_time = millis();
    if(current_time - last_update < MIN_UPDATE_INTERVAL) {
        return;
    }
    
    double tnow = (double)current_time/1000;
    double x = get_moving_average(measure_mm/1000); // 滑动平均滤波
    
    // 第一次运行时初始化
    if(t0 == 0) {
        t0 = tnow;
        x0 = x;
        v0 = 0;
        last_update = current_time;
        return;
    }
    
    double delta_t = tnow - t0;
    double delta_x = x - x0;
    
    // 位置变化太小，认为静止
    if(abs(delta_x) < MIN_POS_CHANGE) {
        v0 = 0;
        last_update = current_time;
        return;
    }
    
    double v = delta_x/delta_t;
    double delta_v = v - v0;
    double a = delta_v/delta_t;
    
    // 限制最大加速度
    if(abs(a) > MAX_ACCEL) {
        a = (a > 0) ? MAX_ACCEL : -MAX_ACCEL;
    }
    
    Serial.printf("Pos: %.3f m, Vel: %.2f m/s, Accel: %.2f m/s^2\n", x, v, a);
    
    t0 = tnow;
    x0 = x;
    v0 = v;
    last_update = current_time;
}

void setup() {
	// put your setup code here, to run once:
	Serial.begin(115200);
	delay(2000);  // 等待串口稳定
	Serial.println("Starting initialization...");
	
	if(!init_sensor()) {
		Serial.println("Sensor init failed! System halted.");
		while(1) { delay(1000); }
	}
	
	Serial.println("Starting measurements...");
	test_sensor();
}

void loop() {
	double dist=measure_once();
	if(dist==0){ // Invalid
		return;
	}
	update_accel(dist);
	// put your main code here, to run repeatedly:
}