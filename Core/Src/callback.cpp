/**
  ******************************************************************************
  * @file           : callback.cpp
  * @brief          : callback objects
  ******************************************************************************
  */

#include "usr_main.h"
#include "main.h"
#include "peripheral_manager.h"

extern Engine left_engine;
extern Engine right_engine;

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == left_engine.htim_->Instance)
	{
		left_engine.updateTrack();
	}
	if (htim->Instance == right_engine.htim_->Instance)
	{
		right_engine.updateTrack();
	}

};




