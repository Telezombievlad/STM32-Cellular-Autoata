// No copyright. Vladislav Aleinik, 2019

//=============================================================================================
// ABSTRACTING OUT OF 7-SEGMENT DISPLAY PINS
//=============================================================================================

// Pin Mapping:
#define A  LL_GPIO_PIN_11
#define B  LL_GPIO_PIN_7
#define C  LL_GPIO_PIN_4
#define D  LL_GPIO_PIN_2
#define E  LL_GPIO_PIN_1
#define F  LL_GPIO_PIN_10
#define G  LL_GPIO_PIN_5
#define DP LL_GPIO_PIN_3

#define POS0 LL_GPIO_PIN_6
#define POS1 LL_GPIO_PIN_8
#define POS2 LL_GPIO_PIN_9
#define POS3 LL_GPIO_PIN_12

const uint32_t PINS_USED = A|B|C|D|E|F|G|DP|POS0|POS1|POS2|POS3;

const uint32_t DIGITS[10] = 
{
	A|B|C|D|E|F,   // 0
    B|C,           // 1
    A|B|D|E|G,     // 2
    A|B|C|D|G,     // 3
    B|C|F|G,       // 4
    A|C|D|F|G,     // 5
    A|C|D|E|F|G,   // 6
    A|B|C,         // 7
    A|B|C|D|E|F|G, // 8
    A|B|C|D|F|G    // 9
};

const uint32_t TRIPLET = A|D|G;

const uint32_t POSITIONS[4] =
{
	POS1|POS2|POS3, // 0
	POS0|POS2|POS3, // 1
	POS0|POS1|POS3, // 2
	POS0|POS1|POS2	// 3
};

//=============================================================================================
// CONFIGURING
//=============================================================================================

void display_config()
{
    // Clocking GPIOA
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

	// Changing GPIO modes
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_1 , LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_2 , LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_3 , LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_4 , LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5 , LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_6 , LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_7 , LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_8 , LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_9 , LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_10, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_11, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_12, LL_GPIO_MODE_OUTPUT);

    // Enabling clocking on the TIM1 timer
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM1);

    // Configure the pin
    //LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_11, LL_GPIO_AF_2);

    // Turn on compare register
    LL_TIM_OC_SetCompareCH1(TIM1, 500);

    // Set polarity
    LL_TIM_OC_SetPolarity(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1);

    // Enable interrupt on compare equal
    LL_TIM_EnableIT_CC1(TIM1);

    LL_TIM_EnableCounter(TIM1);

    NVIC_EnableIRQ(TIM1_CC_IRQn);
	NVIC_SetPriority(TIM1_CC_IRQn, 1);
}

//=============================================================================================
// DISPLAY STATE
//=============================================================================================

struct Display_State
{
	uint32_t display;
	unsigned number;
};

static Display_State DISPLAY_STATE = {0, 1234};

//=============================================================================================
// ALTERING DISPLAY STATE
//=============================================================================================

void setNumberQuarter(unsigned tick)
{
	uint32_t divisors[4] = {1, 10, 100, 1000};

	unsigned quarter = tick % 4;
	unsigned divisor = divisors[quarter];

	if (quarter == 3)
	{
		DISPLAY_STATE.display = TRIPLET | POSITIONS[quarter];
		return;
	}

	DISPLAY_STATE.display = DIGITS[(DISPLAY_STATE.number / divisor) % 10] | POSITIONS[quarter];
}

void displayNumber(unsigned toDisplay)
{
	DISPLAY_STATE.number = toDisplay;
}

//=============================================================================================
// WRITING TO MICROCONTROLLER
//=============================================================================================

void pushToMC()
{
	uint32_t surroundingState = ~PINS_USED & LL_GPIO_ReadOutputPort(GPIOA);
	uint32_t toWrite = PINS_USED & STATE.display;

	LL_GPIO_WriteOutputPort(GPIOA, surroundingState | toWrite);
}
