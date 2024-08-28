/**
  ******************************************************************************
  * @file           : common.c
  * @brief          : common objects
  ******************************************************************************
  */

#include "main.h"
#include "common.h"






//TODO: Testing
// Функция для сравнения двух строк типа char*
uint8_t compareStrings(const char* str1, const char* str2, uint16_t len1, uint16_t len2) {
	if (!(str1 && str2) || len1!=len2 || len1 == 0)
	{
		return 0;
	}
	for (; (*str1 == *str2) && len1!=0; --len1)
	{
        ++str1;
        ++str2;
	}
    return !(len1 ? 1 : 0);
}

//TODO: Testing
/* Функция для выделения участков из строки (\0 необязателен),
 * разделённых delimiter
 * Заполняет строки числом не менее fill_string_len, возвращает его же
 * Не копирует str
 *
 */
USR_StatusTypeDef split(char* str, uint16_t len, char delimiter,
		String* string_pul, uint16_t& fill_string_len, uint16_t* len_to_last_sym  ) {
	if (!len || !str )
	{
		return USR_ERR;
	}
	uint16_t start = 0;
	uint16_t fill_string = 0;
    for (uint16_t i = 0; i<len && fill_string<fill_string_len; ++i) {
        if (str[i] == delimiter) {
        	if (i!= start)	// Защита от нескольких пробелов подряд
        	{
        	string_pul[fill_string].p_ = &str[start];
        	string_pul[fill_string].len_ = i-start;
        	++fill_string;
        	}

        	if (len_to_last_sym!=nullptr)
        	{
        		*len_to_last_sym = i;
        	}

            start = i + 1;
        }
    }
    /* После последнего пробела выполняется, если он не в конце
     * и есть нехватка заполняемых строк
     */
    if (fill_string<fill_string_len && start<len)
    {
    	string_pul[fill_string].p_ = &str[start];
    	string_pul[fill_string].len_ = len-start;
    	++fill_string;
    	if (len_to_last_sym!=nullptr)
    	{
    		*len_to_last_sym = len;
    	}
    }
    fill_string_len = fill_string;
    return USR_OK;

}



USR_StatusTypeDef charToUint16(char* str, size_t len, uint16_t & result) {
    result = 0;
    for (size_t i = 0; i < len; ++i) {
        if (str[i] >= '0' && str[i] <= '9') {
        	//TODO сделать макрос максимального uint16_t
        	if ((result==6553 && (str[i] - '0')>5) || result>6553)
        	{
        		return USR_ERR;
        	}

            result = result * 10 + (str[i] - '0');
        } else {
            return USR_ERR;
        }
    }
    return USR_OK;
}




