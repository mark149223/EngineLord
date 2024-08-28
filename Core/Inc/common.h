/**
  ******************************************************************************
  * @file           : common.h
  * @brief          : common objects
  ******************************************************************************
  */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <cstring>

/* Exported definitions ----------------------------------------------------------*/
#define BEG_LATCH(var) static uint8_t var; \
	if (var)	\
	{			\
	var = 0;	\

#define END_LATCH };

#define UNLATCH(var) var = 1;


#define FIRMWARE_NAME "JAWSX"
#define FIRMWARE_NAME_LEN 5

#define MAX_ECHO_TEXT (uint8_t) 100



/* Exported variables ------------------------------------------------------------*/

/* Exported class prototypes ---------------------------------------------*/
typedef enum: uint8_t
{
  USR_OK,
  USR_ERR
} USR_StatusTypeDef;

enum class EngineState: uint8_t
{
  RUN,
  END,
  SMOOTH_STOP,
  EMERGENCY_STOP,
};

enum class Command : uint8_t
{
	MOV,
	ECHO,
//	EMSTOP,
	STOP
};	///< Порядок важен. STOP должен быть в конце

enum class EngineDir : uint8_t
{
	clockwise,
	counterclockwise
};

enum class EngineObj : uint8_t
{
	left,
	right
};

enum class BlindMessageStatus : uint8_t
{
	correct,
	invalid
};

enum class BlindMessageErrStatus : uint8_t
{
	invalid,
	invalid_module,
	invalid_command,
	invalid_args
};

typedef struct String_
{
	char * p_;	///< название модуля
	uint16_t len_;
} String;

struct UsrPin
{
	GPIO_TypeDef * GPIOx;
	uint16_t GPIO_Pin;
};

typedef struct BlindMessage_
{
	Command command_;	// Далее перечисляются все возможные аргументы
	uint16_t lpos_;
	uint16_t rpos_;
	EngineDir ldir_;
	EngineDir rdir_;
	char ECHOText_[MAX_ECHO_TEXT];
	uint8_t ECHOText_len;
	BlindMessageStatus status_;
	BlindMessageErrStatus err_status_;
} BlindMessage;

typedef struct ECHOArgs_
{
	uint8_t ECHOText_len;
	char ECHOText_[MAX_ECHO_TEXT];
} ECHOArgs;

typedef struct MOVArgs_
{
	uint16_t lpos_;
	uint16_t rpos_;
	EngineDir engine_dir_;
} MOVArgs;





/* Exported functions prototypes ---------------------------------------------*/
uint8_t compareStrings(const char* str1, const char* str2, uint16_t len1,
		uint16_t len2);
USR_StatusTypeDef split(char* str, uint16_t len, char delimiter,
		String * string_pul, uint16_t& fill_string_len, uint16_t* len_to_last_sym = nullptr);
USR_StatusTypeDef charToUint16(char* str, size_t len, uint16_t & result);

//BlindMessage parseASCIIMessage(char * payload, uint16_t len);



#ifdef __cplusplus
}
#endif




#endif /* INC_COMMON_H_ */
