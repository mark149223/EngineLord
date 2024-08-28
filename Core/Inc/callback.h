/**
  ******************************************************************************
  * @file           : callback.h
  * @brief          : callback objects
  ******************************************************************************
  */

#ifndef INC_CALLBACK_H_
#define INC_CALLBACK_H_


#ifdef __cplusplus
extern "C" {
#endif


void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim);

#ifdef __cplusplus
}
#endif


#endif /* INC_CALLBACK_H_ */
