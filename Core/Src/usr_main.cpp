/**
 ******************************************************************************
 * @file      usr_main.cpp
 * @brief     Hardware independent code in preference
 ******************************************************************************
 */


#include "main.h"
#include "lwip.h"
#include "usr_main.h"
#include "net.h"
#include "common.h"
#include <string>
#include "peripheral_manager.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
//TrackCell full[4] = {{10,10},{6,10},{4,20},{2,20}};
//TrackCell stop[4]= {{2,100},{2,1},{2,1},{8,100}};;

/*Главные настройки*/
//const char * commands[2] = {"MOV", "ECHO"};
//#define MAX_BUFFER_CELL	10 //TODO: это убрать в место расположения буфера
//#define MAX_BUFFER_MESSAGE_SIZE	200
//#define MAX_BUFFER_SIZE MAX_BUFFER_MESSAGE_SIZE*MAX_BUFFER_CELL
Engine left_engine;
Engine right_engine;
EngineCommander engine_commander(&left_engine, &right_engine);
BlindScheduler blind_scheduler(&engine_commander);

/*Может быть стоит совместить */
BlindMessage bmessage = {};
BlindMessage * pbmessage = &bmessage;


Parser blind_parser;

uint8_t IsMessageArrived = 0;


void usr_main()
{

	InitIpAdrr();
	UDPTransportInit();

	UsrPin en_pin = {GPIOE, l_EN_Pin};
	UsrPin dir_pin = {GPIOE, l_DIR_Pin};
	UsrPin pwm_pin = {GPIOB, l_PWM_Pin};

	left_engine.link_htim(&htim2);
	left_engine.link_pins(en_pin,  dir_pin, pwm_pin);
	left_engine.link_channel(TIM_CHANNEL_3);


	en_pin = {GPIOD, r_EN_Pin};
	dir_pin = {GPIOD, r_DIR_Pin};
	pwm_pin = {GPIOD, r_PWM_Pin};

	right_engine.link_htim(&htim4);
	right_engine.link_pins(en_pin,  dir_pin, pwm_pin);
	right_engine.link_channel(TIM_CHANNEL_1);

//	driver1.SetTrack(full);
//	driver1.track_phase_len_ = 4;	// убрать
//	driver1.StartTrack();
	while (1)
	{


		MX_LWIP_Process();

		/*Прибыло сообщение*/
		if (IsMessageArrived)
		{
//			UDPSend(reinterpret_cast<uint8_t *>(payload), p->len);
			/*TODO:
			 * Здесь находится иммитация работы кольцевого буффера. В дальнейшем необходимо его добавить.
			 * Копирование сразу в буфер невозможно поскольку не проверена валидность информации parseroм,
			 * который заполняет передаточную переменную pbmessage
			 */
//			static Command ringCommandBuf[MAX_BUFFER_CELL];
//			static uint8_t ringBuf[MAX_BUFFER_SIZE];
			blind_scheduler.Run(IsMessageArrived, pbmessage);
		}
//		HAL_Delay(2000);
//		driver1.SmoothStopTrack();
//		HAL_Delay(100000);

	};
};



