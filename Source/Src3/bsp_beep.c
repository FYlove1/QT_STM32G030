#include "bsp_beep.h"
#include "tim.h"

/**
 * ÉèÖÃ·äÃùÆ÷µÄ×´Ì¬
 * 1:¿ªÆô·äÃùÆ÷   0:¹Ø±Õ·äÃùÆ÷
 */
void set_Beep_Status(uint8_t status)
{
	status ? HAL_TIMEx_PWMN_Start(&htim17, TIM_CHANNEL_1) : HAL_TIMEx_PWMN_Stop(&htim17, TIM_CHANNEL_1) ;
}

