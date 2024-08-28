/**
  ******************************************************************************
  * @file           : peripheral_manager.h
  * @brief          : peripheral manager
  ******************************************************************************
  */

#ifndef INC_PERIPHERAL_MANAGER_H_
#define INC_PERIPHERAL_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "common.h"
#include "net.h"

/* Private includes ----------------------------------------------------------*/

/* Exported variables ------------------------------------------------------------*/

/* Exported class prototypes ---------------------------------------------*/


typedef struct TrackCell_
{
	uint16_t period_;	///> in pulse count
	uint16_t repetition_;
} TrackCell;




/**
 * @brief Engine manager
 */
class Engine
{
public:
	Engine(){};
	Engine(Engine& copy) = delete;
	Engine(Engine&& moved) = delete;
	Engine& operator=(const Engine&) = delete;
	Engine& operator=(Engine&& other) = delete;
	~Engine() = default;

	USR_StatusTypeDef link_htim(TIM_HandleTypeDef * htim) {
		if (!htim){return USR_ERR;}
		htim_=htim;
		return USR_OK;};
	USR_StatusTypeDef link_pins(UsrPin en_pin, UsrPin dir_pin,UsrPin pwm_pin) {
		en_pin_ = en_pin;
		dir_pin_ = dir_pin;
		pwm_pin_ = pwm_pin;
		return USR_OK;};
	USR_StatusTypeDef link_channel(uint32_t channel) {
		channel_ = channel;
		return USR_OK;};


	USR_StatusTypeDef SetDir(EngineDir dir){
		dir_ = dir;
		return USR_OK;};
//	USR_StatusTypeDef SetTrack(TrackCell_* track, uint16_t track_phase_len) {
//	track_=track;
//	track_phase_len_ = track_phase_len;
//	return USR_OK;};	//TODO в коммандере будет
	USR_StatusTypeDef EnableDriver(void){
		HAL_GPIO_WritePin(en_pin_.GPIOx, en_pin_.GPIO_Pin, GPIO_PIN_RESET);
		return USR_OK;};
	USR_StatusTypeDef DisableDriver(void){
		HAL_GPIO_WritePin(en_pin_.GPIOx, en_pin_.GPIO_Pin, GPIO_PIN_SET);
		return USR_OK;};
	USR_StatusTypeDef ActivateDir(void){
		switch (static_cast<int>(dir_))
		{
			case static_cast<int>(EngineDir::clockwise):
			{
				HAL_GPIO_WritePin(dir_pin_.GPIOx, dir_pin_.GPIO_Pin, GPIO_PIN_SET);
				break;
			}
			case static_cast<int>(EngineDir::counterclockwise):
			{
				HAL_GPIO_WritePin(dir_pin_.GPIOx, dir_pin_.GPIO_Pin, GPIO_PIN_RESET);
				break;
			}
		}
		return USR_OK;};
	USR_StatusTypeDef LoadARR(uint8_t period){
		htim_->Instance->ARR = period*__HAL_TIM_GET_COMPARE(htim_, channel_);
		return USR_OK;
	};

	USR_StatusTypeDef ReloadTrack(void){
		track_phase_len_ = 0;
		track_phase_counter_ = 0;
		duration_counter_ = 0;
		return USR_OK;};

	USR_StatusTypeDef StartTrack(void){
		ActivateDir();
		EnableDriver();
		LoadARR(track_[0].period_);
		// Перемещение в теневой регистр
		htim_->Instance->EGR = TIM_EGR_UG;
		__HAL_TIM_CLEAR_FLAG(htim_, TIM_SR_UIF);
		HAL_TIM_PWM_Start_IT(htim_, channel_);
		state_ = EngineState::RUN;
		return USR_OK;};

	USR_StatusTypeDef EndTrack(void){
		state_ = EngineState::END;
		HAL_TIM_PWM_Stop(htim_, channel_);
		DisableDriver();
		ReloadTrack();

		return USR_OK;};

	USR_StatusTypeDef StopTrack(void){
		state_ = EngineState::END;
		HAL_TIM_PWM_Stop(htim_, channel_);
		DisableDriver();
		ReloadTrack();
		return USR_OK;};

	USR_StatusTypeDef SmoothStopTrack(void){
		state_ = EngineState::SMOOTH_STOP;
		return USR_OK;};

	EngineState GetStatus()
	{
		return state_;
	}

	TrackCell_ & GetTrackPhase(uint16_t phase)
	{
		return track_[phase];
	}

	void updateTrack(void)
	{
		duration_counter_++;
		if (duration_counter_== track_[track_phase_counter_].repetition_)
		{
			duration_counter_ = 0;
			track_phase_counter_++;
			if (track_phase_counter_ == track_phase_len_)
			{
				// TODO: проверить происходит ли немедленная остановка
				EndTrack();
			}
			else
			{
				LoadARR(track_[track_phase_counter_].period_);
			}


		}

	}

	static constexpr uint8_t phase_duration = 3;
	static constexpr uint16_t min_speed_multiplier = 60;
	static constexpr uint16_t max_speed_multiplier = 2;
	TrackCell_ track_[(min_speed_multiplier-max_speed_multiplier+1)*2];
	TIM_HandleTypeDef * htim_;
	volatile uint16_t track_phase_len_{0};
	volatile uint16_t track_phase_counter_{0};
	volatile uint16_t duration_counter_{0};

private:


	UsrPin en_pin_;
	UsrPin dir_pin_;
	UsrPin pwm_pin_;
	uint32_t channel_;

	EngineDir dir_{EngineDir::clockwise};
	EngineState state_{EngineState::END};
	bool driver_toggle_{0};
	bool pwm_toggle_{0};


};


class EngineCommander
{
public:
	EngineCommander(Engine * left_engine,  Engine * right_engine)
	: left_engine_(left_engine), right_engine_(right_engine){};
	EngineCommander(EngineCommander& copy) = delete;
	EngineCommander(EngineCommander&& moved) = delete;
	EngineCommander& operator=(const EngineCommander&) = delete;
	EngineCommander& operator=(EngineCommander&& other) = delete;
	~EngineCommander() = default;

	void Stop(EngineObj engine_obj_)
	{
		if (engine_obj_== EngineObj::left)
		{
			current_engine_ = left_engine_;
		}
		else
		{
			current_engine_ = right_engine_;
		}
		current_engine_->StopTrack();

	}
	void CalculateTrack(uint16_t steps,Engine * current_engine_)
	{

		TrackCell_ * track_ = current_engine_->track_;
		uint16_t phase_number = Engine::min_speed_multiplier - Engine::max_speed_multiplier;
		uint16_t half_steps = steps/2;
		uint16_t phase_step_remains = 0;
		uint16_t integer_phase_number = half_steps/Engine::phase_duration;

		if (integer_phase_number>=phase_number)
		{
			integer_phase_number = phase_number;
			phase_step_remains = 0;
		}
		else
		{
			phase_step_remains = half_steps % Engine::phase_duration;
		}
		/* Фронт */
		for (int i =0; i<integer_phase_number; i++)
		{
			track_[i].repetition_ = Engine::phase_duration;
			track_[i].period_ = Engine::min_speed_multiplier-i;
		}
		uint16_t max_speed_multiplier = track_[integer_phase_number-1].period_;
		/* Плато */
		if (phase_step_remains)
		{
			/* Для ускорения и торможения */
			track_[integer_phase_number-1].repetition_ += 2*phase_step_remains;
		}
		uint16_t smooth_step = steps-(integer_phase_number*Engine::phase_duration+phase_step_remains)*2;
		if (smooth_step)
		{
			track_[integer_phase_number-1].repetition_ += smooth_step;
		}
		/* Спад */
		track_[integer_phase_number-1].repetition_ += Engine::phase_duration;
		for (int i =1; i<integer_phase_number; i++)
		{
			track_[(integer_phase_number-1)+i].repetition_ = Engine::phase_duration;
			track_[(integer_phase_number-1)+i].period_ = max_speed_multiplier+i;
		}

		current_engine_->track_phase_len_ = 2*integer_phase_number-1;
	}

	void Run(EngineObj engine_obj_, uint16_t steps, EngineDir dir)
	{
		if (engine_obj_== EngineObj::left)
		{
			current_engine_ = left_engine_;
		}
		else
		{
			current_engine_ = right_engine_;
		}

//		current_engine_->StopTrack();

		CalculateTrack(steps,current_engine_);
		current_engine_->SetDir(dir);
		current_engine_->StartTrack();

	}
private:
	Engine * left_engine_;
	Engine * right_engine_;
	Engine * current_engine_;

};



class Parser
{
public:
	Parser(){
	};
	Parser(Parser& copy) = delete;
	Parser(Parser&& moved) = delete;
	Parser& operator=(const Parser&) = delete;
	Parser& operator=(Parser&& other) = delete;
	~Parser() = default;

	USR_StatusTypeDef parse(char * payload, uint16_t len, BlindMessage * bmessage);
private:
	static const char * commandMassive[3];
};



class BlindScheduler
{
public:
	BlindScheduler(EngineCommander * engine_commander):
		engine_commander_(engine_commander){};
	BlindScheduler(BlindScheduler& copy) = delete;
	BlindScheduler(BlindScheduler&& moved) = delete;
	BlindScheduler& operator=(const BlindScheduler&) = delete;
	BlindScheduler& operator=(BlindScheduler&& other) = delete;
	~BlindScheduler() = default;

	/*Суть функции в том, чтобы добавить команды в буфер команд
	 * и аргументы в буфер аргументов распаршенного сообщения*/
	//TODO: функция сырая и требует смены сигнатуры

	USR_StatusTypeDef addElToBuf(Command * ringCommandBuf, uint8_t * ringBuf)
	{
		return USR_OK;
	}
	/*Суть функции в том, чтобы забрать команды из буфера команд
	 * и аргументы из буфера аргументов р*/
	//TODO: функция сырая и требует смены сигнатуры
	USR_StatusTypeDef getFromBuf(Command * ringCommandBuf, uint8_t * ringBuf);
	//TODO: функция сырая и требует смены сигнатуры
	void Run(uint8_t & IsMessageArrived, BlindMessage * pbmessage)
	{
//		if (IsMessageArrived)
//		{
//			addElToBuf(ringCommandBuf, ringBuf);
//		}
//		/*TODO: Здесь должна быть мудрая логика на проверку команд в буфере
//		 * и по достижению необходимых условий - извлечения, что означает их дальнейшее выполнение
//		 */
//		//Проверка буфера на наличие командр
//		//Если буфера на возможность их выполнения
//		if (IsMessageArrived)
//		{
//			getFromBuf(ringCommandBuf, ringBuf);
//		}

	IsMessageArrived = 0;
	if (pbmessage->status_ == BlindMessageStatus::invalid)
	{
		return;
	}

	switch(static_cast<int>(pbmessage->command_))
	{
		case static_cast<int>(Command::MOV):
			{

			if (pbmessage->lpos_)
			{
				engine_commander_->Stop(EngineObj::left);
				engine_commander_->Run(EngineObj::left, pbmessage->lpos_, pbmessage->ldir_);
			}

			if (pbmessage->rpos_)
			{
				engine_commander_->Stop(EngineObj::right);
				engine_commander_->Run(EngineObj::right, pbmessage->rpos_, pbmessage->rdir_);
			}
				break;
			}
		case static_cast<int>(Command::ECHO):
			{
			UDPSend(reinterpret_cast<uint8_t *>(pbmessage->ECHOText_), pbmessage->ECHOText_len);
			break;
			}
		case static_cast<int>(Command::STOP):
			{
				engine_commander_->Stop(EngineObj::left);
				engine_commander_->Stop(EngineObj::right);
				break;
			}
	}
}
private:
	EngineCommander * engine_commander_;
};



#ifdef __cplusplus
}
#endif




#endif /* INC_PERIPHERAL_MANAGER_H_ */
