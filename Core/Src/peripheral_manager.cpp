/**
  ******************************************************************************
  * @file           : peripheral_manager.cpp
  * @brief          : common objects
  ******************************************************************************
  */

#include "peripheral_manager.h"
#include "common.h"

/*Parser static massive*/
const char * Parser::commandMassive[3] = {
			 "MOV",
			 "ECHO",
			 "STOP"};	// Порядок должен совпадать с таковым в enum Command


USR_StatusTypeDef Parser::parse(char * payload, uint16_t len, BlindMessage * pbmessage)
{
	/* Первичная инициализиация */
	pbmessage->status_ = BlindMessageStatus::invalid;
	/* на 1 больше, чем максимальное количество всех аргументов,
	 * включая имя модуля и команду
	 */
	constexpr uint16_t required_manager_count{2};	// только модуль и команды
	uint16_t manager_count{required_manager_count};
	String manager_strings[required_manager_count];
	uint16_t command_len; // длина модуля и команды команды
	split(payload, len, ' ', manager_strings, manager_count, &command_len);


	if (manager_count<required_manager_count
			|| manager_strings[0].len_ < 3
			|| manager_strings[1].len_ < 3 )
	{
		pbmessage->err_status_ = BlindMessageErrStatus::invalid;
		return USR_OK;
	}
	if (!compareStrings(FIRMWARE_NAME,
			manager_strings[0].p_,
			FIRMWARE_NAME_LEN,
			manager_strings[0].len_))
	{
		pbmessage->err_status_ = BlindMessageErrStatus::invalid_module;
		return USR_OK;
	}
	uint8_t isFind = 0;
	for (int i=0; i<static_cast<uint8_t>(Command::STOP)+1; i++)
	{
		if (compareStrings(Parser::commandMassive[i],
				manager_strings[1].p_,
				strlen(Parser::commandMassive[i]),
				manager_strings[1].len_))
		{
			pbmessage->command_ = static_cast<Command>(i);
			isFind=1;
			break;
		}
	}
	if (!isFind)
	{
		pbmessage->err_status_ = BlindMessageErrStatus::invalid_command;
		return USR_OK;
	}
	//TODO: вынести в отдельную функцию
	//TODO: Стоит сделать проверки на переполнение по аргументам

	uint16_t args_shift = command_len+1; // После пробела
	switch (static_cast<int>(pbmessage->command_))
	{
		case static_cast<int>(Command::MOV):
		{
			static const uint16_t required_args_count{2};	// Число аргументов
			uint16_t args_count{required_args_count};
			String arg_strings[required_args_count];

			if ((len-(args_shift))<=0)
			{
				pbmessage->err_status_ = BlindMessageErrStatus::invalid_args;
				return USR_OK;
			}

			split((payload+args_shift), (len-args_shift), ' ', arg_strings, args_count);


			if (args_count<required_args_count
					|| arg_strings[0].len_ < 2
					|| arg_strings[1].len_ < 2 )
			{
				pbmessage->err_status_ = BlindMessageErrStatus::invalid_args;
				return USR_OK;
			}

			/*Значение для левого двигателя*/
			if (arg_strings[0].p_[0] == '+')
			{
				pbmessage->ldir_ = EngineDir::clockwise;
				if (charToUint16(arg_strings[0].p_+1,	// После знака
						arg_strings[0].len_-1,	// меньше, чем знак
						pbmessage->lpos_) != USR_OK)
				{
					pbmessage->err_status_ = BlindMessageErrStatus::invalid_args;
					return USR_OK;
				}

			}
			else if (arg_strings[0].p_[0] == '-')
			{
				pbmessage->ldir_ = EngineDir::counterclockwise;
				if (charToUint16(arg_strings[0].p_+1,	// После знака
						arg_strings[0].len_-1,	// меньше, чем знак
						pbmessage->lpos_) != USR_OK)
				{
					pbmessage->err_status_ = BlindMessageErrStatus::invalid_args;
					return USR_OK;
				}
			}
			else
			{
				pbmessage->err_status_ = BlindMessageErrStatus::invalid_args;
				return USR_OK;
			}


			/*Значение для правого двигателя*/
			if (arg_strings[1].p_[0] == '+')
			{
				pbmessage->rdir_ = EngineDir::clockwise;
				if (charToUint16(arg_strings[1].p_+1,	// После знака
						arg_strings[1].len_-1,	// меньше, чем знак
						pbmessage->rpos_) != USR_OK)
				{
					pbmessage->err_status_ = BlindMessageErrStatus::invalid_args;
					return USR_OK;
				}

			}
			else if (arg_strings[1].p_[0] == '-')
			{
				pbmessage->rdir_ = EngineDir::counterclockwise;
				if (charToUint16(arg_strings[0].p_+1,	// После знака
						arg_strings[0].len_-1,	// меньше, чем знак
						pbmessage->rpos_) != USR_OK)
				{
					pbmessage->err_status_ = BlindMessageErrStatus::invalid_args;
					return USR_OK;
				}
			}
			else
			{
				pbmessage->err_status_ = BlindMessageErrStatus::invalid_args;
				return USR_OK;
			}
			break;
		}
		case static_cast<int>(Command::ECHO):
		{
			/*Недопустимость */
			if ((len-(args_shift+1))<=0)
			{
				pbmessage->err_status_ = BlindMessageErrStatus::invalid_args;
				return USR_OK;
			}
			uint16_t byte_for_copy = ((len-(args_shift+1))<MAX_ECHO_TEXT) ?
					len-args_shift : MAX_ECHO_TEXT;
			pbmessage->ECHOText_len = byte_for_copy;
			memcpy(pbmessage->ECHOText_, (payload+args_shift), byte_for_copy);
			break;
		}
		case static_cast<int>(Command::STOP):
		{
			break;
		}
	}


	pbmessage->status_ = BlindMessageStatus::correct;

//	UNUSED(pbmessage->status_);
	return USR_OK;


}
