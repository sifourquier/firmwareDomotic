#include "LPC11xx.h"

#include <cr_section_macros.h>

#define ABS(N) ((N<0)?(-N):(N))

#include "modbus.h"
#include "main.h"
#include "board.h"
#include "DS18B20.h"
#include "pwm.h"
#include "oneWire.h"
#include "TLI4970.h"
#include "triac.h"
#include "iap_driver.h"
//#include "math.h"
#include "sin.h"
#include "firmware.h"
#include "lcd_thermostat.h"
#include "bme280_user.h"
#include "sht30.h"
#include "screenFeroFluid.h"

/*
consomation
triak0% tli4950=OFF -> ~4mA 12v -> 48mW
triak100% +tli4950+ON -> ~10.2mA 12v -> 122mW
 */

#define NB_SAMPLE_MOYENNE (2*50*NB_POINT_SIN) //2s

#define VERSION_CODE 3
#define TIME_SHOW_SET_VALUE 2000000 //us
#define TIME_BACKLIGHT_ON 5000000 //us
#define REPEAT_TIME 250 //ms
#define ACC_REPEAT_TIME 9/10
#define MIN_REPEAT_TIME 10 //ms
#define MIN_TEMPERATURE 100 //1/100°
#define MAX_TEMPERATURE 2600 //1/100°
#define TIME_TO_ENTRE_IN_CONFIG_MODE 2000 //ms

uint32_t SystemCoreClock;

typedef enum
{
	Sds_START,
	Sds_READ,
	Sds_END
} t_stateReadDs18B20;

static volatile int64_t ID_DS18B20=0;
static volatile int16_t pressure=0;
static volatile int16_t humidity=0;
static volatile int16_t humidityFromModbus=SMALL_EMPY*100;
static volatile int16_t temperatureDS18B20=0;
static volatile int16_t temperature=0;
static volatile int16_t temperatureDraw=0;
static volatile int16_t temperatureFromModbus=0;
static volatile int16_t temperatureSet=1000;
static volatile int16_t puissance230v;
static int16_t out[NB_OUT]={500,[1 ... NB_OUT-1]=MAX_POWER};
static volatile uint8_t stateSwitch[NB_SW]={0};
static volatile int64_t lastTimePresseSwitch[NB_SW]={0};

static volatile uint8_t flagPressed[NB_SW];//for send on modbus
static volatile uint8_t flagPressedInternal[NB_SW];//for internal use
static const uint8_t PORT_SW[NB_SW]={LIST_PORT_SW};
static const uint8_t BIT_SW[NB_SW]={LIST_BIT_SW};
static volatile uint8_t currentSensorEnable;
static volatile uint8_t fanPower; //for print on LCD
static volatile t_Time currentTimeFromModBus={0};
static volatile uint8_t pwmLedSwitch[NB_LED_SWITCH]={1};

volatile t_Config config;

void configClock()
{
	LPC_SYSCON->SYSPLLCLKSEL  = 0;   /* Select PLL Input = IRC*/
	LPC_SYSCON->SYSPLLCLKUEN  = 0x01;               /* Update Clock Source      */
	LPC_SYSCON->SYSPLLCLKUEN  = 0x00;               /* Toggle Update Register   */
	LPC_SYSCON->SYSPLLCLKUEN  = 0x01;
	while (!(LPC_SYSCON->SYSPLLCLKUEN & 0x01));     /* Wait Until Updated       */
	LPC_SYSCON->SYSPLLCTRL    = 0x03; //P=(2^0) PLL M=(3+1) M=1 > 7mA / M=2 > 10mA M=4/11mA
	LPC_SYSCON->PDRUNCFG     &= ~(1 << 7);          /* Power-up SYSPLL          */
	while (!(LPC_SYSCON->SYSPLLSTAT & 0x01));           /* Wait Until PLL Locked    */

	while (!(LPC_SYSCON->SYSPLLSTAT & 0x01));
	LPC_SYSCON->MAINCLKSEL    = 0x03;     			/* Select PLL Clock for main Clock  */
	LPC_SYSCON->MAINCLKUEN    = 0x01;               /* Update MCLK Clock Source */
	LPC_SYSCON->MAINCLKUEN    = 0x00;               /* Toggle Update Register   */
	LPC_SYSCON->MAINCLKUEN    = 0x01;
	while (!(LPC_SYSCON->MAINCLKUEN & 0x01));       /* Wait Until Updated       */

	LPC_SYSCON->SYSAHBCLKDIV  = 1; //clock divider =1

	SystemCoreClock=48000000;
}

void modBusCallBackWrite(t_Function function,uint32_t adresse,uint32_t value)
{
	static uint32_t tempDayOfWeek=0; //for receive in 2 time
	if(function == F_WRITE_COIL || function == F_WRITE_COILS)
		if(value)
			value=MAX_POWER;
	if(adresse>=ADRESSE_OUT && adresse<ADRESSE_OUT+NB_OUT)
	{
#ifdef LED_12V
		out[adresse]=value;
		if(adresse<NB_TRIAC)
		{
#ifdef TRIACK
			set_triac(value);
			currentSensorEnable=value>0;
#endif
		}
		else
		{
			setPwm(adresse-NB_TRIAC-ADRESSE_OUT,value);
		}
#endif
	}
	if(adresse>=ADDRESS_PWM_LED_SWITCH && adresse<ADDRESS_PWM_LED_SWITCH+NB_OUT)
		{
			pwmLedSwitch[adresse-ADDRESS_PWM_LED_SWITCH]=value;
		}
	else if(adresse==ADDRESS_GET_SET_MODBUS_ADDRESS)
	{
		if(value>0 && value<248 && value!=128)
		{
			config.modeBusaddress=value;
			configSave();
		}
	}
	else if(adresse==ADDRESS_BACKLIGHT_POWER)
	{
		config.backlightPower=value;
		configSave();
	}
	else if(adresse==ADDRESS_RESET)
	{
		NVIC_SystemReset();
	}
	else if(adresse==ADDRESS_TIMEOFDAY)
	{
		tempDayOfWeek=(uint32_t)value<<16;
	}
	else if(adresse==ADDRESS_TIMEOFDAY+1)
	{
		tempDayOfWeek|=value;
		currentTimeFromModBus.timeOfDay=tempDayOfWeek;
	}
	else if(adresse==ADDRESS_DAYOFWEEK)
	{
		currentTimeFromModBus.dayOfWeek=value;
	}
	else if(adresse==ADDRESS_DAYOFMONTH)
	{
		currentTimeFromModBus.dayOfMonth=value;
	}
	else if(adresse==ADDRESS_MONTH)
	{
		currentTimeFromModBus.month=value;
	}
	else if(adresse==ADDRESS_YEAR)
	{
		currentTimeFromModBus.year=value;
	}
	else if(adresse==ADDRESS_FANPOWER)
	{
		fanPower=value;
	}
	else if(adresse==ADDRESS_TEMPERATURE_FROM_MODBUS)
	{
		temperatureFromModbus=value;
	}
	else if(adresse==ADDRESS_HUMIDITY_FROM_MODBUS)
	{
		humidityFromModbus=value;
	}
}

int modBusCallBackRead(t_Function function,uint32_t adresse,uint8_t* data) //return number of bit of data
{
	if(function==F_READ_INPUT || function==F_READ_INPUT_REG) //read input
	{
		if(adresse<NB_SW)
		{
			if(function==F_READ_INPUT)
			{
				data[0]&=~(1<<adresse);

				data[0]|=(flagPressed[adresse]|stateSwitch[adresse])<<adresse;
				flagPressed[adresse]=0;

				return 1; //return size in bit
			}
			else if(function==F_READ_INPUT_REG)
			{
				if((flagPressed[adresse]|stateSwitch[adresse]))
				{
					data[0]=0xFF;
					data[1]=0xFF;
				}else{
					data[0]=0x00;
					data[1]=0x00;
				}
				flagPressed[adresse]=0;
				return 16; //return size in bit
			}
		}
		if(function==F_READ_INPUT_REG)
		{
			if(adresse==NB_SW)
			{
				data[0]=puissance230v>>8;
				data[1]=puissance230v;
				return 16; //return size in bit
			}
			else if(adresse==NB_SW+1)
			{
				data[0]=temperatureDS18B20>>8;
				data[1]=temperatureDS18B20;
				return 16; //return size in bit
			}
			else if(adresse==NB_SW+2)
			{
				data[0]=temperature>>8;
				data[1]=temperature;
				return 16; //return size in bit
			}
			else if(adresse==NB_SW+3)
			{
				data[0]=humidity>>8;
				data[1]=humidity;
				return 16; //return size in bit
			}
			else if(adresse==NB_SW+4)
			{
				data[0]=pressure>>8;
				data[1]=pressure;
				return 16; //return size in bit
			}
			else if(adresse>=1000 && adresse<1000+4)
			{
				int16_t IDtemp=ID_DS18B20>>(16*(adresse-(NB_SW+4)));
				data[0]=IDtemp>>8;
				data[1]=IDtemp;
				return 16; //return size in bit
			}
			else if(adresse==2000)
			{
				data[0]=VERSION_CODE>>8;
				data[1]=VERSION_CODE;
				return 16; //return size in bit
			}
		}
	}
	else //read outpout
	{
		if(adresse<NB_OUT)
		{
			if(function==F_READ_COIL)
			{
				data[0]&=~(1<<adresse);
				data[0]|=(out[adresse]>0)<<adresse;

				return 1; //return size in bit
			}
			else if(function==F_READ_REG)
			{
				data[0]=out[adresse]>>8;
				data[1]=out[adresse];
				return 16;//return size in bit
			}
		}
		else if(adresse==ADDRESS_GET_SET_MODBUS_ADDRESS)
		{
			data[0]=0;
			data[1]=config.modeBusaddress;
			return 16; //return size in bit
		}
	}
	return 0;

}

int main(void) {
#ifdef THERMOSTAT_LCD
	t_lcd lcd={0};
#endif
	for(int l=0;l<NB_LED_SWITCH;l++)
		pwmLedSwitch[l]=1;
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x1UL << 16); //power IOCON
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6); //power gpio
	configClock();
	initBoard();
	iapInit();

	NVIC_SetPriority(TIMER_16_0_IRQn,1);//one wire
	NVIC_SetPriority(TIMER_16_1_IRQn,1);//triac
	NVIC_SetPriority(TIMER_32_0_IRQn,2);//comm
	NVIC_SetPriority(UART_IRQn,2);//comm
	NVIC_SetPriority(SSP1_IRQn,3);
	NVIC_SetPriority(I2C_IRQn,3);
	NVIC_SetPriority(SSP0_IRQn,3);
	NVIC_SetPriority(ADC_IRQn,3);
	NVIC_SetPriority(EINT0_IRQn,3);
	NVIC_SetPriority(EINT1_IRQn,3);
	NVIC_SetPriority(EINT2_IRQn,3);
	NVIC_SetPriority(EINT3_IRQn,3);
	NVIC_SetPriority(TIMER_32_1_IRQn,0);//pwm

#ifdef BME280
	bme280InitUser();
#endif
#ifdef DS18B20
	initDS18B20();
#endif
#ifdef TLI4970
	initTLI4970();
	SET_BIT(PORT_EN_CURRENT,BIT_EN_CURRENT,1)
#endif
#ifdef TRIACK
	triacInit();
#endif
//#ifdef LED_12V
	pwmInit();
//#endif

#ifdef LED_SWITCH
	INIT_LED_SWITCH;//on universal board use MOSI
#endif

#ifdef THERMOSTAT_LCD
	lcdInit();
#endif

#ifdef SHT30
	sht30Init();
#endif

#ifdef HORLOGE_FERO
	screenInit();
	screenBare();
#endif

	modeBusInit();
	modBusSetCallbackWrite(modBusCallBackWrite);
	modBusSetCallbackRead(modBusCallBackRead);

	for(int l=0;l<NB_OUT;l++)
	{
		modBusCallBackWrite(F_WRITE_REG,l,out[l]);
	}
	// Force the counter to be placed into memory
	// Enter an infinite loop, just incrementing a counter

	SET_BIT(PORT_LED1,BIT_LED1,1);

#ifdef DS18B20
	ID_DS18B20=getIdDS18B20();
#else
	ID_DS18B20=__COUNTER__;
#endif

	uint8_t bouton_pressed=0;
	for(int l=0;l<NB_SW;l++) //get state of switch and store a flag if one are juste pressed
	{
		if(!GET_BIT(PORT_SW[l],BIT_SW[l]))
			bouton_pressed++;
	}

	config=*((t_Config*)(FLASH_SIZE/2));
	if(config.backlightPower>1000)
		config.backlightPower=1000;

	if(bouton_pressed)
	{
		config.modeBusaddress=128;
	}

	if((config.modeBusaddress>=248) || (config.modeBusaddress==0))
	{
		unsigned char add=ID_DS18B20%246;
		add++;//no 0
		if(add>=128)//no 128
			add++; //val betwen 0 an 247 exept 128
		modBusCallBackWrite(F_WRITE_REG,ADDRESS_GET_SET_MODBUS_ADDRESS,add);
	}


	//lcdMap();

	while(1) {
#ifdef DHT22
		t_DHT22Data dht22;
		static int waitDataDht22=0;
#endif
#ifdef DS18B20
		static t_stateReadDs18B20 StateDS18B20=0;
#endif
		int i;

		__WFI(); //wake up > 480Hz (pwm ...) //no __WFI(); -> 15mA/6.5V / __WFI();->11mA/6.5V ~71mW

		if(flagNew1ms)
		{

#ifdef HORLOGE_FERO
			screenComputeMs();
#endif

#ifdef THERMOSTAT_LCD
			uint16_t currentTimeOfDayMinute=currentTimeFromModBus.timeOfDay/60;
			static uint32_t repeatTime=REPEAT_TIME;
			static uint8_t backlight=0;
			static uint8_t switchInUse=0;
			static uint32_t TimePressePower=0;
			static uint8_t configMode=0;
			static int repeat=0;

			if(!configMode && flagPressedInternal[LCD_POWER_BUTTON])
			{
				flagPressedInternal[LCD_POWER_BUTTON]=0;
				if(switchInUse || backlight==0) //is is not the firs bouton we touch or baklight qre off
					backlight=!backlight;
				//else we enable backlight for TIME_BACKLIGHT_ON but no togle backlight
			}

			if(time-lastTimePresseSwitch[LCD_POWER_BUTTON]<TIME_BACKLIGHT_ON ||
					time-lastTimePresseSwitch[LCD_DOWN_BUTTON]<TIME_BACKLIGHT_ON ||
					time-lastTimePresseSwitch[LCD_MOD_BUTTON]<TIME_BACKLIGHT_ON ||
					time-lastTimePresseSwitch[LCD_UP_BUTTON]<TIME_BACKLIGHT_ON ||
					time-lastTimePresseSwitch[LCD_TIME_BUTTON]<TIME_BACKLIGHT_ON)
			{
				switchInUse=1;
				if(backlight)
					setPwm(PWM_BACKLIGHT, config.backlightPower);
				else
					setPwm(PWM_BACKLIGHT, 0);
			}
			else
			{
				switchInUse=0;
				setPwm(PWM_BACKLIGHT, 0);
			}

			if(stateSwitch[LCD_POWER_BUTTON])
			{
				TimePressePower++;
				if(TimePressePower>TIME_TO_ENTRE_IN_CONFIG_MODE)
				{
					configMode=1;
				}
			}
			else
			{
				TimePressePower=0;
			}

			memset(&lcd,0,sizeof(lcd)); //defauklt all off

			if(configMode)
			{
				lcd.point=0;
				lcd.small_digit_hour[0]=config.backlightPower/100;
				lcd.small_digit_hour[1]=config.backlightPower%100;
#ifdef LCD_CHEAP_2_TEMP_5_PLUS_2_DAY_4_HOME_IN_OUT_SUN_MOON_SNOW_LOCK_HAND_TIME
				lcd.big_digit=config.modeBusaddress;//show 2digit on big digit
				lcd.big_digit_hide_decimal=1;
				lcd.small_digit=(config.modeBusaddress*10)%100; //show lest digit t on small_digit
				lcd.small_digit_hide_decimal=1;
#else
				lcd.big_digit=config.modeBusaddress; //if scren can only draw xx0 or xx5 not realy fine
#endif
				lcd.day1to7=0;
				lcd.deg_c=0;
				lcd.deg_c2=0;
				lcd.twoPoints=0;
				if(TimePressePower<TIME_TO_ENTRE_IN_CONFIG_MODE && flagPressedInternal[LCD_POWER_BUTTON])
				{
					configMode=0;
					configSave();
				}
				repeat++;
				if(flagPressedInternal[LCD_UP_BUTTON] || (stateSwitch[LCD_UP_BUTTON]&&repeat>repeatTime))
				{
					repeatTime=repeatTime*ACC_REPEAT_TIME;
					if(repeatTime<MIN_REPEAT_TIME)
						repeatTime=MIN_REPEAT_TIME;
					flagPressedInternal[LCD_UP_BUTTON]=0;
					if(config.modeBusaddress<248)
						config.modeBusaddress+=1;
					repeat=0;
				}
				else if(flagPressedInternal[LCD_DOWN_BUTTON] || (stateSwitch[LCD_DOWN_BUTTON]&&repeat>repeatTime))
				{
					repeatTime=repeatTime*ACC_REPEAT_TIME;
					if(repeatTime<MIN_REPEAT_TIME)
						repeatTime=MIN_REPEAT_TIME;
					flagPressedInternal[LCD_DOWN_BUTTON]=0;
					if(config.modeBusaddress>1)
						config.modeBusaddress-=1;
					repeat=0;
				}
				if(flagPressedInternal[LCD_MOD_BUTTON] || (stateSwitch[LCD_MOD_BUTTON]&&repeat>repeatTime))
				{
					repeatTime=repeatTime*ACC_REPEAT_TIME;
					if(repeatTime<MIN_REPEAT_TIME)
						repeatTime=MIN_REPEAT_TIME;
					flagPressedInternal[LCD_MOD_BUTTON]=0;
					if(config.backlightPower<1000)
						config.backlightPower+=1;
					repeat=0;
				}
				else if(flagPressedInternal[LCD_TIME_BUTTON] || (stateSwitch[LCD_TIME_BUTTON]&&repeat>repeatTime))
				{
					repeatTime=repeatTime*ACC_REPEAT_TIME;
					if(repeatTime<MIN_REPEAT_TIME)
						repeatTime=MIN_REPEAT_TIME;
					flagPressedInternal[LCD_TIME_BUTTON]=0;
					if(config.backlightPower>0)
						config.backlightPower-=1;
					repeat=0;
				}
				else
				{
					if(stateSwitch[LCD_UP_BUTTON]==0 && stateSwitch[LCD_DOWN_BUTTON]==0 && stateSwitch[LCD_MOD_BUTTON]==0 && stateSwitch[LCD_TIME_BUTTON]==0)
					{
						repeat=0;
						repeatTime=REPEAT_TIME;
					}
				}
			}
			else if(stateSwitch[LCD_MOD_BUTTON]) //show local humidity external humidity + external temperature
			{
				lcd.point=1;
				lcd.small_digit_hour[0]=SMALL_EMPY; //no show
				lcd.small_digit_hour[1]=humidity/100;
				lcd.big_digit=(temperatureFromModbus+5)/10; //external temperature
				lcd.small_digit=humidityFromModbus/100;
				lcd.small_digit_hide_decimal=1;

				lcd.day1to7=0x40>>(currentTimeFromModBus.dayOfWeek-1);
				lcd.deg_c=1;
				//lcd.deg_c2=0; //humidity
				lcd.twoPoints=0;
			}
			else if(stateSwitch[LCD_TIME_BUTTON]) //show month + day and external temperature
			{
				lcd.point=1;
#ifdef LCD_CHEAP_2_TEMP_5_PLUS_2_DAY_4_HOME_IN_OUT_SUN_MOON_SNOW_LOCK_HAND_TIME
				lcd.small_digit_hour[0]=currentTimeFromModBus.month; //need write in american type date month folow bay day because 1segment can only draw 0 1 2
				lcd.small_digit_hour[1]=currentTimeFromModBus.dayOfMonth;
#else
				lcd.small_digit_hour[0]=currentTimeFromModBus.dayOfMonth;
				lcd.small_digit_hour[1]=currentTimeFromModBus.month;
#endif
				lcd.big_digit=(temperatureFromModbus+5)/10; //external temperature
				lcd.out=1;
				lcd.small_digit=humidityFromModbus/100;
				lcd.small_digit_hide_decimal=1;

				lcd.day1to7=0x40>>(currentTimeFromModBus.dayOfWeek-1);
				lcd.deg_c=1;
				lcd.deg_c2=0; //humidity
				lcd.twoPoints=1;
			}
			else
			{
				lcd.point=1;
				lcd.small_digit_hour[0]=(currentTimeOfDayMinute/60)%24;
				lcd.small_digit_hour[1]=currentTimeOfDayMinute%60;
				lcd.small_digit=temperatureSet/10;//if lcd can show 2 temperatureSet
#ifdef LCD_CHEAP_2_TEMP_5_PLUS_2_DAY_4_HOME_IN_OUT_SUN_MOON_SNOW_LOCK_HAND_TIME
				lcd.big_digit=(temperatureDraw+5)/10;//can show 2 temperature then continue to show current temperature
#endif
				repeat++;
				if(((time-lastTimePresseSwitch[LCD_UP_BUTTON])<TIME_SHOW_SET_VALUE) ||
						((time-lastTimePresseSwitch[LCD_DOWN_BUTTON])<TIME_SHOW_SET_VALUE))
				{
#ifdef LCD_CHEAP_2_TEMP_5_PLUS_2_DAY_4_HOME_IN_OUT_SUN_MOON_SNOW_LOCK_HAND_TIME

#else
					lcd.big_digit=temperatureSet/10;
#endif
					if(flagPressedInternal[LCD_UP_BUTTON] || (stateSwitch[LCD_UP_BUTTON]&&repeat>repeatTime))
					{
						repeatTime=repeatTime*ACC_REPEAT_TIME;
						if(repeatTime<MIN_REPEAT_TIME)
							repeatTime=MIN_REPEAT_TIME;
						flagPressedInternal[LCD_UP_BUTTON]=0;
						if(temperatureSet<MAX_TEMPERATURE)
							temperatureSet+=10;
						repeat=0;
					}
					else if(flagPressedInternal[LCD_DOWN_BUTTON] || (stateSwitch[LCD_DOWN_BUTTON]&&repeat>repeatTime))
					{
						repeatTime=repeatTime*ACC_REPEAT_TIME;
						if(repeatTime<MIN_REPEAT_TIME)
							repeatTime=MIN_REPEAT_TIME;
						flagPressedInternal[LCD_DOWN_BUTTON]=0;
						if(temperatureSet>MIN_TEMPERATURE)
							temperatureSet-=10;
						repeat=0;
					}
					else
					{
						if(stateSwitch[LCD_UP_BUTTON]==0 && stateSwitch[LCD_DOWN_BUTTON]==0)
						{
							repeat=0;
							repeatTime=REPEAT_TIME;
						}
					}
				}
				else
				{
					/*int abs=ABS(lcd.big_digit*10-(temperature+5));
					if(abs>3)//if delta t < 0.03°  now change temperature on screen (to avoid blinking)*/
					lcd.big_digit=(temperatureDraw+5)/10;
					if(flagPressedInternal[LCD_UP_BUTTON]||flagPressedInternal[LCD_DOWN_BUTTON])
					{
						flagPressedInternal[LCD_DOWN_BUTTON]=0;
						flagPressedInternal[LCD_UP_BUTTON]=0;
						repeatTime=REPEAT_TIME;
					}
				}
				lcd.day1to7=0x40>>(currentTimeFromModBus.dayOfWeek-1);
				lcd.twoPoints=1;
				lcd.deg_c=1;
				lcd.deg_c2=1;
			}
			if(fanPower)
				lcd.fan=0x40>>(fanPower-1);
			else
				lcd.fan=0;
			lcdSet(lcd);
#endif

			flagNew1ms--;

#ifdef TLI4970
			static int timeFromZeroCross=0;
			static int numSample;
			if(flagZeroCross)
			{
				flagZeroCross=0;
				timeFromZeroCross=0;
			}
			else
				timeFromZeroCross++;
			static int64_t moy_p=0;
			static int u=0;
			/*volatile static short logu[NB_SAMPLE_MOYENNE];
			volatile static short logi[NB_SAMPLE_MOYENNE];*/

			if(timeFromZeroCross<NB_POINT_SIN)
			{
				static uint8_t old_currentSensorEnable;
				u=SIN[timeFromZeroCross];//*3250;only after moyen //uV (sin(...)*230*2^0.5*10) sin ret 100 to -100
				SET_BIT(PORT_EN_CURRENT,BIT_EN_CURRENT,currentSensorEnable);
				if(currentSensorEnable && old_currentSensorEnable)
				{
					int i=readTLI4970();
					moy_p+=u*i;
				}
				else
				{
					moy_p=0;
				}
				old_currentSensorEnable=currentSensorEnable;
				//logu[numSample]=u;
				//logi[numSample]=i;

				numSample++;
			}
			if(numSample>=NB_SAMPLE_MOYENNE)
			{
				numSample=0;
				moy_p=moy_p*325/100;//*3.25;//watch SIN
				if(puissance230v<0)
					puissance230v=0;
				puissance230v=moy_p/NB_SAMPLE_MOYENNE/100;//1/10W max 6kW
				moy_p=0;
			}
#endif


#ifdef LED_SWITCH
			static int pwm_led_switch=0;
			pwm_led_switch=(pwm_led_switch+1)%10;
			SET_BIT(PORT_LED_SWITCH,BIT_LED_SWITCH,0);
			for(int l=0;l<4;l++)
			{
				SET_IN(PORT_SW[l],BIT_SW[l]);
			}
#endif
			for(int l=0;l<NB_SW;l++) //get state of switch and store a flag if one are juste pressed
			{
				static int old_stateSwitch[NB_SW];
#ifdef THERMOSTAT_LCD
				stateSwitch[l]=!GET_BIT(PORT_SW[l],BIT_SW[l]);
#else
				stateSwitch[l]=!GET_BIT(PORT_SW[l],BIT_SW[l]);
#endif
				if(stateSwitch[l])
				{
					lastTimePresseSwitch[l]=time;
				}
				if(old_stateSwitch[l]==0 && stateSwitch[l])
				{
					flagPressed[l]=1;
					flagPressedInternal[l]=1;
				}
				old_stateSwitch[l]=stateSwitch[l];
			}
#ifdef LED_SWITCH
			SET_BIT(PORT_LED_SWITCH,BIT_LED_SWITCH,1);
			for(int l=0;l<4;l++)
			{
				//if(pwm_led_switch==0)
				{
					SET_OUT(PORT_SW[l],BIT_SW[l]);
					SET_BIT(PORT_SW[l],BIT_SW[l],pwm_led_switch>=pwmLedSwitch[l]);
				}
			}
#endif

		}

#ifdef DHT22
		if(waitDataDht22 && getDataDHT22(&dht22)>0)
		{
			waitDataDht22=0;
			temperature=dht22.temperature;
			humidity=dht22.humidity;
		}
#endif
		if(flagNewSec)
		{
			flagNewSec=0;
#ifdef HORLOGE_FERO
			//uint32_t m=timeS/60;
			//printTime(m/60, m, timeS%60);
			//cleanScreen();
#endif

#ifdef BME280
			struct bme280_data bme280=bme280GetSensorDataUser();
			temperature=bme280.temperature;
			pressure=bme280.pressure;
			humidity=bme280.humidity;

#endif

#ifdef SHT30
			static div=0;
			div++;
			if((div&0x7)==0)
			{
				sht30Data sht30=sht30Read();
				temperature=sht30.temperature;
				humidity=sht30.humidity;
				if(temperature==-4500)
					sht30Init();
			}
#endif

#ifdef DHT22
			if(time_s % 2==0) //every 2 seconde
			{
				if(startDHT22()>0)
					waitDataDht22=1;

			}
#endif

#ifdef DS18B20
			if(time_s % 2==1) //every 2 seconde
			{
				if(RunDS18B20()>-1)
					StateDS18B20=Sds_START;
			}
#endif

			if(ABS(temperatureDraw-temperature)>3) //wait a delta of 0.03° for update drawed temp
			{
				temperatureDraw=temperature;
			}
		}
#ifdef DS18B20
		if(StateDS18B20==Sds_START)
		{
			if(getDataOneWire(NULL)>-1) //one wire transaction have end
			{
				if(readDS18B20()>-1)
					StateDS18B20=Sds_READ;
			}
		}
		if(StateDS18B20==Sds_READ)
		{
			if(getDataOneWire(NULL)>-1) //one wire transaction have end
			{
				if(getDS18B20(&temperatureDS18B20)>-1)
					StateDS18B20=Sds_END;
			}
		}
#endif
		i++ ;
		modeBusCompute();
	}
	return 0 ;
}

void configSave()
{
	uint8_t buf[256]={0xFF};
	memcpy(buf,(uint8_t*)&config,sizeof(config));
	volatile int ret=iapPrepareSector(FLASH_SIZE/2/SECTORSIZE,FLASH_SIZE/2/SECTORSIZE);
	ret=iapEraseSector(FLASH_SIZE/2/SECTORSIZE,FLASH_SIZE/2/SECTORSIZE);
	ret=iapPrepareSector(FLASH_SIZE/2/SECTORSIZE,FLASH_SIZE/2/SECTORSIZE);
	ret=iapCopyRamToFlash((void*)buf,(void*)(FLASH_SIZE/2),sizeof(buf));
	ret=ret;
}
