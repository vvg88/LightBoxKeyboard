#include "main.h"
#include "Utils.h"

//int i = 0;

const uint8_t ButtonsCodes[32] = 
{
	29,		// ENC1
	27,		// JOY-E
	26,		// JOY-D
	25,		// JOY-C
	5,		// KEY5
	6,		// KEY6
	30,		// ENC2
	14,		// KEY14
	22,		// KEY22
	10,		// KEY10
	13,		// KEY13
	4,		// KEY4
	19,		// KEY19
	2,		// KEY2
	7,		// KEY7
	11,		// KEY11
	1,		// KEY1
	31,		// ENC3
	15,		// KEY15
	20,		// KEY20
	18,		// KEY18
	17,		// KEY17
	12,		// KEY12
	0,		// KEY0
	21,		// KEY21
	3,		// KEY3
	8,		// KEY8
	9,		// KEY9
	16,		// KEY16
	28,		// ENC0
	24,		// JOY-B
	23,		// JOY-A
};


//TButtonsState ButtonsState = { 0xFFFFFFFF };		// Buttons' state (current and previous)
//TButtonsState ButtonsPrevState = { 0xFFFFFFFF };	
uint32_t ButtonsState = 0xFFFFFFFF, ButtonsPrevState = 0xFFFFFFFF;

void ReadButtons(void);
void ReadEncoders(void);

// TIM6 Update interrupt handler. It's used for scanning of buttons' and encoders' state
void TIM6_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	
	ReadButtons();
	ReadEncoders();
}

uint16_t ButtonsPaState = PORTA_BUTTONS_DEFAULT_STATE, ButtonsPaPrevState = PORTA_BUTTONS_DEFAULT_STATE;		// Buttons' lines state on a port А
uint16_t ButtonsPbState = PORTB_BUTTONS_DEFAULT_STATE, ButtonsPbPrevState = PORTB_BUTTONS_DEFAULT_STATE;		// Buttons' lines state on a port B
uint16_t ButtonsPcState = PORTC_BUTTONS_DEFAULT_STATE, ButtonsPcPrevState = PORTC_BUTTONS_DEFAULT_STATE;		// Buttons' lines state on a port C

bool AntiChattPaWait = false;									// Признак ожидания антидребезга
bool AntiChattPbWait = false;
bool AntiChattPcWait = false;

uint8_t AntiChattPaWaitCntr = 0;							// Счетчик антидребезга
uint8_t AntiChattPbWaitCntr = 0;
uint8_t AntiChattPcWaitCntr = 0;

__IO bool ButtnsStableChanged = false;				// Признак изменения состояния кнопок

__IO uint16_t ButtonsPaStableState = PORTA_BUTTONS_DEFAULT_STATE;		// Stable state of buttons on a PORTA
__IO uint16_t ButtonsPbStableState = PORTB_BUTTONS_DEFAULT_STATE;		// Stable state of buttons on a PORTB
__IO uint16_t ButtonsPcStableState = PORTC_BUTTONS_DEFAULT_STATE;		// Stable state of buttons on a PORTC

// Buttons' state reading
void ReadButtons(void)
{
	ButtonsPaPrevState = ButtonsPaState;		// Read new buttons' state
	ButtonsPaState = (uint16_t)(GPIOA->IDR & PORTA_BUTTONS_DEFAULT_STATE);
	
	ButtonsPbPrevState = ButtonsPbState;
	ButtonsPbState = (uint16_t)(GPIOB->IDR & PORTB_BUTTONS_DEFAULT_STATE);
	
	ButtonsPcPrevState = ButtonsPcState;
	ButtonsPcState = (uint16_t)(GPIOC->IDR & PORTC_BUTTONS_DEFAULT_STATE);
	
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
	
	if (ButtonsPbState != ButtonsPbPrevState)			// All the same like for buttons on the PORTA
	{
		if (AntiChattPbWait)
			AntiChattPbWaitCntr = 0;
		else
			AntiChattPbWait = true;
	}
	else
	{
		if (AntiChattPbWait)
		{
			if (AntiChattPbWaitCntr++ == 20)
			{
				AntiChattPbWait = false;
				AntiChattPbWaitCntr = 0;
				ButtonsPbStableState = ButtonsPbState;
				ButtnsStableChanged = true;
			}
		}
	}
	
	if (ButtonsPcState != ButtonsPcPrevState)			// All the same like for buttons on the PORTA
	{
		if (AntiChattPcWait)
			AntiChattPcWaitCntr = 0;
		else
			AntiChattPcWait = true;
	}
	else
	{
		if (AntiChattPcWait)
		{
			if (AntiChattPcWaitCntr++ == 20)
			{
				AntiChattPcWait = false;
				AntiChattPcWaitCntr = 0;
				ButtonsPcStableState = ButtonsPcState;
				ButtnsStableChanged = true;
			}
		}
	}
}

uint8_t enc0State = 0x03, enc0NewState = 0x03;		// Encoders' lines state
uint8_t enc1State = 0x03, enc1NewState = 0x03;
uint8_t enc2State = 0x03, enc2NewState = 0x03;
uint8_t enc3State = 0x03, enc3NewState = 0x03;

uint8_t enc0cntr = 0x02;													// Encoders' state counter (counts edges on encoders' lines)
uint8_t enc1cntr = 0x02;
uint8_t enc2cntr = 0x02;
uint8_t enc3cntr = 0x04;


// Encoders' state reading
void ReadEncoders(void)
{
	enc0NewState = (uint8_t)(GPIOC->IDR & 0x03);					// Read an encoders' new state
	enc1NewState = (uint8_t)((GPIOC->IDR >> 2) & 0x03);
	enc2NewState = (uint8_t)((GPIOC->IDR >> 4) & 0x03);
	enc3NewState = (uint8_t)((GPIOC->IDR >> 10) & 0x03);
		
	switch (enc3State)							// Depending on a current and a new state
	{																// increment or decrement a counter
		case 3:
			if (enc3NewState == 2)
				enc3cntr++;
			if (enc3NewState == 1)
				enc3cntr--;
			break;
		case 2:
			if (enc3NewState == 0)
				enc3cntr++;
			if (enc3NewState == 3)
				enc3cntr--;
			break;
		case 1:
			if (enc3NewState == 0)
				enc3cntr--;
			if (enc3NewState == 3)
				enc3cntr++;
			break;
		case 0:
			if (enc3NewState == 1)
				enc3cntr++;
			if (enc3NewState == 2)
				enc3cntr--;
			break;
	}
	enc3State = enc3NewState;
	
	switch (enc0State)							// All the same like for encoder 3 but this has TWO stable states (0 or 3)
	{																// and for 1 click it passes ONE intermediate state (1 or 2)
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
	enc0State = enc0NewState;
	
	switch (enc1State)							// All the same like for encoder 0
	{
		case 3:
			if (enc1NewState == 2)
				enc1cntr++;
			if (enc1NewState == 1)
				enc1cntr--;
			break;
		case 2:
			if (enc1NewState == 0)
				enc1cntr++;
			if (enc1NewState == 3)
				enc1cntr--;
			break;
		case 1:
			if (enc1NewState == 0)
				enc1cntr--;
			if (enc1NewState == 3)
				enc1cntr++;
			break;
		case 0:
			if (enc1NewState == 1)
				enc1cntr++;
			if (enc1NewState == 2)
				enc1cntr--;
			break;
	}
	enc1State = enc1NewState;
	
	switch (enc2State)							// All the same like for encoder 0
	{
		case 3:
			if (enc2NewState == 2)
				enc2cntr++;
			if (enc2NewState == 1)
				enc2cntr--;
			break;
		case 2:
			if (enc2NewState == 0)
				enc2cntr++;
			if (enc2NewState == 3)
				enc2cntr--;
			break;
		case 1:
			if (enc2NewState == 0)
				enc2cntr--;
			if (enc2NewState == 3)
				enc2cntr++;
			break;
		case 0:
			if (enc2NewState == 1)
				enc2cntr++;
			if (enc2NewState == 2)
				enc2cntr--;
			break;
	}
	enc2State = enc2NewState;
	
	uint8_t encCode = 0;
	if ((enc3cntr == 0) || (enc3cntr == 8))								// If the counter is incremented or decremented by 4
	{																											// it means that its state has changed
		encCode = 0xC0 | (enc3cntr == 0 ? 0x07 : 0x06);			// Create an event code and enqueue it
		EnQueue(BUTTON_CODE, encCode);
		enc3cntr = 4;																				// Reset the counter for a detection of a new event
	}
	
	if ((enc0cntr == 0) || (enc0cntr == 4))								// If the counter is incremented or decremented by 2
	{																											// it means that its state has changed
		encCode = 0;
		encCode = 0xC0 | (enc0cntr == 0 ? 0x01 : 0x00);
		EnQueue(BUTTON_CODE, encCode);
		enc0cntr = 2;
	}
	
	if ((enc1cntr == 0) || (enc1cntr == 4))								// If the counter is incremented or decremented by 2
	{																											// it means that its state has changed
		encCode = 0;
		encCode = 0xC0 | (enc1cntr == 0 ? 0x03 : 0x02);
		EnQueue(BUTTON_CODE, encCode);
		enc1cntr = 2;
	}
	
	if ((enc2cntr == 0) || (enc2cntr == 4))								// If the counter is incremented or decremented by 2
	{																											// it means that its state has changed
		encCode = 0;
		encCode = 0xC0 | (enc2cntr == 0 ? 0x05 : 0x04);
		EnQueue(BUTTON_CODE, encCode);
		enc2cntr = 2;
	}
}

// This handler detects which button has been pressed or released
// and creates corresponding event
void ButtonsHandler(void)
{
	if (ButtnsStableChanged)			// if a state's been changed
	{
		ButtonsPrevState = ButtonsState;			// Save a new state
		/*ButtonsState.PAbuttons = (ButtonsPaStableState & 0x01FF) | ((ButtonsPaStableState >> 2) & 0x0600);
		ButtonsState.PBbuttons = (ButtonsPbStableState & 0x0003) | ((ButtonsPbStableState >> 2) & 0x3FFC);
		ButtonsState.PCbuttons = ((ButtonsPcStableState >> 6) & 0x000F) | ((ButtonsPcStableState >> 9) & 0x0070);*/
		
		ButtonsState = (ButtonsPaStableState & 0x01FF) | ((ButtonsPaStableState >> 2) & 0x0600);
		ButtonsState |= ((ButtonsPbStableState & 0x0003) | ((ButtonsPbStableState >> 2) & 0x3FFC)) << 11;
		ButtonsState |= (((ButtonsPcStableState >> 6) & 0x000F) | ((ButtonsPcStableState >> 9) & 0x0070)) <<  25;
		
		uint8_t buttonCode = 0;
		for (int i = 0; i < 32; i++)		// In this loop
		{
			if((ButtonsState ^ ButtonsPrevState) & (1 << i))					// Detect which button changed its state
			{
				buttonCode = ((ButtonsState & (1 << i)) ? 0x80 : 0x40) | ButtonsCodes[i];	// Create an event depending on pressing or releasing a button
				EnQueue(BUTTON_CODE, buttonCode);																											// and enqueue an event
			}
		}
		ButtnsStableChanged = false;
	}
};

// Initialises the start condition of the encoders
void EncodersInit(void)
{
	enc0State = enc0NewState = (uint8_t)(GPIOC->IDR & 0x03);
	enc1State = enc1NewState = (uint8_t)((GPIOC->IDR >> 2) & 0x03);
	enc2State = enc2NewState = (uint8_t)((GPIOC->IDR >> 4) & 0x03);
};
