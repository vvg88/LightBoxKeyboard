#include "main.h"
#include "Utils.h"

uint32_t ButtonsState = 0xFFFFFFFF, ButtonsPrevState = 0xFFFFFFFF;		// Состояния кнопок (текущее и предыдущее)

uint16_t ButtonsPaState = PORTA_BUTTONS_DEFAULT_STATE, ButtonsPaPrevState = PORTA_BUTTONS_DEFAULT_STATE;		// Состояние линий кнопок на порту А
bool AntiChattPaWait = false;									// Признак ожидания антидребезга
uint8_t AntiChattPaWaitCntr = 0;							// Счетчик антидребезга
__IO bool ButtnsStableChanged = false;				// Признак изменения состояния кнопок
__IO uint16_t ButtonsPaStableState = PORTA_BUTTONS_DEFAULT_STATE;		// Stable state of buttons on a PORTA

uint8_t enc0State = 0x03, enc0NewState = 0x03;		// Encoders' lines state
uint8_t enc0cntr = 0x04;													// Encoders' state counter (counts edges on encoders' lines)
//__IO uint8_t EncodersState = 0;
//__IO bool EncsChanged = false;

// TIM6 Update interrupt handler. It's used for scanning of buttons' and encoders' state
void TIM6_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	
	ButtonsPaPrevState = ButtonsPaState;		// Read new buttons' state
	ButtonsPaState = (uint16_t)(GPIOA->IDR & PORTA_BUTTONS_DEFAULT_STATE);
	
	if (ButtonsPaState != ButtonsPaPrevState)		// if it's been changed
	{
		if (AntiChattPaWait)					// if antichattering period has already been started
			AntiChattPaWaitCntr = 0;		// restart it by reset of a counter
		else
			AntiChattPaWait = true;			// start antichattering period
	}
	else
	{
		if (AntiChattPaWait)												// if it's antichattering period
		{
			if (AntiChattPaWaitCntr++ == 20)					// increment a counter until it reaches 20 (10 ms)
			{
				AntiChattPaWait = false;								// stop antichattering period
				AntiChattPaWaitCntr = 0;								// reset a counter
				ButtonsPaStableState = ButtonsPaState;	// save a new stable state
				ButtnsStableChanged = true;							// set a flag that a state of buttons has been changed
			}
		}
	}
	
	enc0NewState = (uint8_t)(GPIOC->IDR >> 5);		// Read an encoder's new state
	switch (enc0State)														// Depending on a current and a new state
	{																							// increment or decrement a counter
		case 3:
			if (enc0NewState == 2)
				enc0cntr++;
			if (enc0NewState == 1)
				enc0cntr--;
			break;
		case 2:
			if (enc0NewState == 0)
				enc0cntr++;
			if (enc0NewState == 3)
				enc0cntr--;
			break;
		case 1:
			if (enc0NewState == 0)
				enc0cntr--;
			if (enc0NewState == 3)
				enc0cntr++;
			break;
		case 0:
			if (enc0NewState == 1)
				enc0cntr++;
			if (enc0NewState == 2)
				enc0cntr--;
			break;
	}
	if ((enc0cntr == 0) || (enc0cntr == 8))								// If the counter is incremented or decremented by 4
	{																											// it means that its state has changed
		uint8_t encCode = 0;
		encCode = 0xC0 | (enc0cntr == 0 ? 0x05 : 0x04);			// Create an event code and enqueue it
		EnQueue(ENCODER_CODE, encCode);
		enc0cntr = 4;																				// Reset the counter for a detection of a new event
		//EncodersState = (EncodersState & ~0x03) | (enc0cntr == 0 ? 0x02 : 0x01);
		//EncsChanged = true;
	}
}

// This handler detects which button has been pressed or released
// and creates corresponding event
void ButtonsHandler(void)
{
	if (ButtnsStableChanged)			// if a state's been changed
	{
		ButtonsPrevState = ButtonsState;			// Save a new state
		ButtonsState = (ButtonsState & 0xF800) | ((ButtonsPaStableState & 0x01FF) | ((ButtonsPaStableState >> 2) & 0x0600));
		
		uint8_t buttonCode = 0;
		for (int i = 0; i < 32; i++)		// In this loop
		{
			if((ButtonsState ^ ButtonsPrevState) & (1 << i))					// Detect which button changed its state
			{
				buttonCode = ((ButtonsState ? 0x02 : 0x01) << 6) | i;		// Create an event depending on pressing or releasing a button
				EnQueue(BUTTON_CODE, buttonCode);												// and enqueue an event
			}
		}
		ButtnsStableChanged = false;
	}
	
	/*if (EncsChanged)
	{
		uint8_t encCode = 0;
		for (int i = 0; i < 8; i + 2)
		{
			if ((EncodersState & (0x03 << i)) ^ )
		}
		EncsChanged = false;
	}*/
};

void EncodersInit(void)
{
	
};
