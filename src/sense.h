#include "Arduino.h"
#include "VL53L0X.h"
#include "VL53L1X.h"
#include "Ultrasonic.h"

extern VL53L0X l0x;
extern VL53L1X l1x;
extern Ultrasonic usnd;
extern volatile uint32_t dist_ultsnc,dist_l0x;
extern volatile bool ultsnc_finish, l0x_finish;

const int 	SCL_L1X=21,SDA_L1X=22,
			SCL_L0X=19,SDA_L0X=18;

bool _init_bus();  // 返回是否初始化成功

void test_sensor();

bool init_sensor();

// If timeout/invalid, return 0
double measure_once();