#include "main.h"
#include "Utils.h"
#include <stdbool.h>

/* Причина последнего сбрса контроллера */
extern TStatus LastResReason;
/* Структура с версиями */
extern TBlockInfo BlockVersions;

/* Состояние работы модуля */
ModuleStateType ModuleState = PASSIVE;

void StatReqHandler(const TCommReply * const comm);
void InfoReqHandler(const TCommReply * const comm);
void SaveT0handler(const TCommReply * const comm);
void SaveT1handler(const TCommReply * const comm);

void ReturnStatus(const TStatus stat);
void ReturnValue(const uint8_t cmd, const uint16_t value);
//void ReturnError(const uint16_t errCod);
void ReturnLongReply(const uint8_t * const pBuff, size_t buffSize);
void SendReply(const uint8_t * const buff, size_t length);

/* Обработчики коротких команд */
const TcommHandler CommHandlers[15] =
{
	StatReqHandler,
	SaveT0handler,
	SaveT1handler,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	InfoReqHandler,
};

/**
	* @brief  Обработчик команды
	* @param  comm: указатель на команду
  * @retval none
  */
void CommHandler(const TCommReply * const comm)
{
	if(ModuleState != PASSIVE)				// Если модуль в активном состоянии
	{
		if (comm->commNum < 0x0F)			// Определить тип команды (длинная/короткая)
		{
			if (CommHandlers[comm->commNum] == 0)		// Если обработчика такого нет,
				ReturnStatus(ST_CMD_UNKNOWN);					// Передать ошибку
			else
				CommHandlers[comm->commNum](comm);		// Выполнить соотв-ий обработчик
		}
		else													// Если длинная команда
		{
			// Обработка длинных команд
			/*if (comm->commIndx < ELEMENTS_OF(LongCommHandler))
				LongCommHandler[comm->commIndx](comm);
			else*/
				ReturnStatus(ST_CMD_UNKNOWN);
		}
	}
	else
	{
		if((comm->commNum == 0) || ((comm->commNum == 0x0E) && (comm->commParam == 0)))
			CommHandlers[comm->commNum](comm);			// В пассивном состоянии выполнять только команды 0 и 0х0Е
		else
			ReturnStatus(LastResReason);						// На все остальные отправлять статус
	}
};

/* Значение тоггл-байта */
uint8_t ToggleByte;
/* Значение параметра для последнего извлеченного события */
uint16_t LastEventValue;
/* Приоритет последнего извлеченного события */
uint8_t LastEventPriority;
/* Признак, что было послано событие */
bool LastEventWasSent = false;

/**
	* @brief  Обработчик запроса статуса
	* @param  comm: указатель на команду
  * @retval none
  */
void StatReqHandler(const TCommReply * const comm)
{
	if(ModuleState != PASSIVE)
	{
		if (ToggleByte == comm->toggleByte)			// Если тоггл-байт не изменился и до этого была передача события, повторить передачу
		{
			if (LastEventWasSent)
				ReturnValue(LastEventPriority, LastEventValue);
			else
				ReturnStatus(ST_OK);
		}
		else			// Если тоггл-байт отличается и до этого была передача события, сбросить признак последней передачи
		{
			if (LastEventWasSent)
				LastEventWasSent = false;
			/* Извлечь событие из очереди и при его наличии отослать статус-рапорт. В проивном случае отправить "0" */
			if (DeQueue(&LastEventValue, &LastEventPriority))
			{
				ReturnValue(LastEventPriority, LastEventValue);
				LastEventWasSent = true;
			}
			else
				ReturnStatus(ST_OK);
			ToggleByte = comm->toggleByte;
		}
	}
	else
		ReturnStatus(LastResReason);		// В пассивном состоянии отправлять причину сброса
}

/* Переменная для запоминания T0 */
uint16_t T0;
/* Переменная для запоминания T1 */
uint16_t T1;

// Обработчик для сохранения временной переменной Т0
void SaveT0handler(const TCommReply * const comm)
{
	T0 = comm->commParam;
	ReturnStatus(ST_OK);
}

// Обработчик для сохранения временной переменной Т1
void SaveT1handler(const TCommReply * const comm)
{
	T1 = comm->commParam;
	ReturnStatus(ST_OK);
}

/**
	* @brief  Обработчик запроса информации и изменения состояния
	* @param  comm: указатель на команду
  * @retval none
  */
void InfoReqHandler(const TCommReply * const comm)
{
	switch (comm->commParam)
	{
		case 0:			// Перейти в активный режим
			if (BlockVersions.HwVersion > HW_VERSION)
				ReturnStatus(WRONG_HW_VERSION);
			else
				ReturnValue(ST_RAP_CODE, (uint16_t)(-100));
			
			ActiveModeInit();
			ModuleState = ACTIVE;
			EnQueue(LONG_COM_DONE_CODE, ST_OK);
			break;
		case 1:			// Переслать размер структуры с версиями
			ReturnValue(comm->commNum, sizeof(TBlockInfo));
			break;
		case 2:			// Переслать структуру с версиями
			ReturnLongReply((uint8_t*)&BlockVersions, sizeof(TBlockInfo));
			break;
		default:		// Команда не опознана
			ReturnStatus(ST_CMD_UNKNOWN);
			break;
	}
}

/**
	* @brief  Послать статус-рапорт
	* @param  stat: статус
  * @retval none
  */
void ReturnStatus(const TStatus stat)
{
	ReturnValue(ST_RAP_CODE, stat);
}

//int k = 0;
/**
	* @brief  Переслать значение (статус-рапорт или ответ на команду)
	* @param  cmd: номер команды
	* @param  value: передаваемое значение
  * @retval none
  */
void ReturnValue(const uint8_t cmd, const uint16_t value)
{
	TCommReply reply;
	
	reply.commNum = cmd;					// Сформировать ответ
	reply.modAddr = MODULE_ADR;
	reply.commParam = value;
		
	SendReply((uint8_t*)&reply, 3);		// Послать ответ
}

/**
	* @brief  Послать код ошибки таблицы стим-ции
	* @param  errCod: код ошибки
  * @retval none
  */
//void ReturnError(const uint16_t errCod)
//{
//	ReturnValue(ST_RAP_CODE, errCod);
//}

/**
	* @brief  Переслать длинный ответ
	* @param  pBuff: указатель на буфр с данными
	* @param  buffSize: размер буфера
  * @retval none
  */
void ReturnLongReply(const uint8_t * const pBuff, size_t buffSize)
{
	TCommReply reply;
	
	reply.commNum = 0x0F;						// Сформировать длиный ответ
	reply.modAddr = MODULE_ADR;
	reply.commLen = buffSize - 1;
	reply.commIndx = pBuff[0];			// В байт, где для команды передается индекс, записать первый байт буфера ответа
	memmove(reply.commData, &pBuff[1], buffSize - 1);//memmove(&reply.commData[1], pBuff, buffSize - 1);
	
	SendReply((uint8_t*)&reply, buffSize + 2);		// Передать ответ
}

/**
	* @brief  Принять команду.
	* @param  buff: указатель на буфер, куда принять команду
  * @retval Длина полученных данных
  */
size_t GetCommand(uint8_t * const buff)
{
	const uint8_t addrMask = MODULE_ADR << 4;
	uint8_t crc = 0;
	uint8_t * pRxBuff = 0;
	uint8_t * pEnd = 0;			// Указатель на конец длинного сообщения
	
	MODIFY_REG(USART1->CR1, 0, USART_CR1_RE);		// Разрешить прием
	while (1)
	{
		register uint16_t usartWord;
		
		while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);			// Принять байт
		usartWord = USART_ReceiveData(USART1);
		
		if (usartWord & 0x100)														// Если адресный байт
		{																									// Обнулить все переменные
			pRxBuff = 0;
			pEnd = 0;
			crc = 0;
			if (addrMask == (uint8_t)(usartWord & 0xF0))		// Если адрес совпал с адресом модуля
			{
				buff[0] = crc = (uint8_t)usartWord;						// Сохранить его
				pRxBuff = &buff[1];
			}
			continue;
		}
		else																					// Если принят обычный байт
		{
			if (pRxBuff == 0)														// Принятый байт не адресован модулю
				continue;																	// Ожидать адресный байт
			if ((buff[0] & 0x0F) != 0x0F)								// Если принимается короткая команда
			{
				if (pRxBuff < &buff[3])										// Принять 2 и 3 байты команды
				{
					*pRxBuff++ = (uint8_t)usartWord;
					crc += (uint8_t)usartWord;							// Вычислить CRC
				}
				else
				{
					if (crc == (uint8_t)usartWord)					// Принять CRC и проверить ее
						break;
					pRxBuff = 0;														// В случае несовпадения, сбросить автомат состояний
				}
			}
			else
			{
				if (pRxBuff == &buff[1])														// Принять длину сообщения длиной команды
					pEnd = &buff[3] + (size_t)(usartWord & 0xFF);			// Вычислить указатель на конец сообщения
				else if (pRxBuff == pEnd)														// Если пришел последний байт длинного сообщения
				{
					if (crc == (uint8_t)usartWord)										// Принять и проверить CRC
						break;
					pRxBuff = 0;
					continue;
				}
				*pRxBuff++ = (uint8_t)usartWord;										// Принять байт длинного сообщения
				crc += (uint8_t)usartWord;
			}
		}
	}
	
	MODIFY_REG(USART1->CR1, USART_CR1_RE, 0);		// Запретить прием
	return pRxBuff - buff;											// Вернуть длину принятой команды
};

/**
	* @brief  Послать ответ на команду.
	* @param  buff: указатель на буфер с передаваемыми данными
	* @param  length: длина буфера
  * @retval None
  */
void SendReply(const uint8_t * const buff, size_t length)
{
	uint8_t crc = 0;
	
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);		// ДОждаться освобождения передатчика
	crc = buff[0];
	USART_SendData(USART1, crc | 0x100);			// Послать первый байт
	
	for (uint8_t i = 1; i < length; i++)			// Передать оставшиеся байты сообщения
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		crc += buff[i];
		USART_SendData(USART1, buff[i]);
	}
	
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	USART_SendData(USART1, crc);							// Передать CRC
	
}
