#include "bsp_softwareTimer.h"
#include "main.h"

/**
 * 函数功能： 设置软件定时器的定时值
 * 参数：
 *     @tsTimeType *TimeType ： 软件定时器的结构体
 *     @uint32_t TimeInter ；   软件定时器的时间（单位：ms）
 * 返回值：
 *      无
*/
void setTime(tsTimeType *TimeType, uint32_t TimeInter)
{
    TimeType->TimeStart = HAL_GetTick();    //获取系统运行时间
    TimeType->TimeInter = TimeInter;      //间隔时间
}

/**
 * 函数功能： 比较软件定时启的值
 * 参数：
 *      @tsTimeType *timeType： 软件定时器结构体
 * 返回值：
 *      返回1，代表超时
*/
uint8_t  compareTime(tsTimeType *TimeType)  //比较时间
{
    return ((HAL_GetTick()-TimeType->TimeStart) >= TimeType->TimeInter);  //返回1代表超时
}


