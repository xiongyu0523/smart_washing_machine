#include "board.h"
#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "fsl_gpio.h"

#define USE_DSB_BLOCK					0


#define I2C_CLK_PIN                     IOMUXC_GPIO_AD_B0_09_GPIO1_IO09
#define I2C_DATA_PIN                    IOMUXC_GPIO_AD_B0_10_GPIO1_IO10
#define I2C_MASTER_CLK_GPIO             GPIO1
#define I2C_MASTER_CLK_GPIO_PIN         (9U)
#define I2C_MASTER_DATA_GPIO            GPIO1
#define I2C_MASTER_DATA_GPIO_PIN        (10U)
#define I2C_SETUP_TIME					5000
#define I2C_HOLD_TIME					10000

#if USE_DSB_BLOCK
#define I2C_SCL_LOW						do{\
											__DSB();\
											GPIO_PinWrite(I2C_MASTER_CLK_GPIO,I2C_MASTER_CLK_GPIO_PIN,0);\
										}while(0)
										
#define I2C_SCL_HIGH					do{\
											GPIO_PinWrite(I2C_MASTER_CLK_GPIO,I2C_MASTER_CLK_GPIO_PIN,1);\
											__DSB();\
											__ISB();\
										}while(0)

#define I2C_SDA_LOW						do{\
											GPIO_PinWrite(I2C_MASTER_DATA_GPIO,I2C_MASTER_DATA_GPIO_PIN,0);\
											__DSB();\
											__ISB();\
										}while(0)
										
#define I2C_SDA_HIGH					do{\
											GPIO_PinWrite(I2C_MASTER_DATA_GPIO,I2C_MASTER_DATA_GPIO_PIN,1);\
											__DSB();\
											__ISB();\
										}while(0)

#else
#define I2C_SCL_LOW						GPIO_PinWrite(I2C_MASTER_CLK_GPIO,I2C_MASTER_CLK_GPIO_PIN,0)
#define I2C_SCL_HIGH					GPIO_PinWrite(I2C_MASTER_CLK_GPIO,I2C_MASTER_CLK_GPIO_PIN,1)

#define I2C_SDA_LOW						GPIO_PinWrite(I2C_MASTER_DATA_GPIO,I2C_MASTER_DATA_GPIO_PIN,0)
#define I2C_SDA_HIGH					GPIO_PinWrite(I2C_MASTER_DATA_GPIO,I2C_MASTER_DATA_GPIO_PIN,1)
#endif


void i2c_master_init(void ){
  
  IOMUXC_SetPinMux(
    I2C_CLK_PIN,        /* GPIO_AD_B0_09 is configured as GPIO1_IO09 */
    0U);                                    /* Software Input On Field: Input Path is determined by functionality */
  IOMUXC_SetPinConfig(
    I2C_CLK_PIN,        /* GPIO_AD_B0_09 PAD functional properties : */
    0x10B0u);                               /* Slew Rate Field: Slow Slew Rate
                                               Drive Strength Field: R0/6
                                               Speed Field: medium(100MHz)
                                               Open Drain Enable Field: Open Drain Disabled
                                               Pull / Keep Enable Field: Pull/Keeper Enabled
                                               Pull / Keep Select Field: Keeper
                                               Pull Up / Down Config. Field: 100K Ohm Pull Down
                                               Hyst. Enable Field: Hysteresis Disabled */
  IOMUXC_SetPinMux(
    I2C_DATA_PIN,        /* GPIO_AD_B0_09 is configured as GPIO1_IO09 */
    0U);                                    /* Software Input On Field: Input Path is determined by functionality */
  IOMUXC_SetPinConfig(
    I2C_DATA_PIN,        /* GPIO_AD_B0_09 PAD functional properties : */
    0x10B0u);                               /* Slew Rate Field: Slow Slew Rate
                                               Drive Strength Field: R0/6
                                               Speed Field: medium(100MHz)
                                               Open Drain Enable Field: Open Drain Disabled
                                               Pull / Keep Enable Field: Pull/Keeper Enabled
                                               Pull / Keep Select Field: Keeper
                                               Pull Up / Down Config. Field: 100K Ohm Pull Down
                                               Hyst. Enable Field: Hysteresis Disabled */

   /* GPIO configuration on GPIO_B1_12 (pin D13) */
	 gpio_pin_config_t gpio2_pinD13_config = {
	   .direction = kGPIO_DigitalOutput,
	   .outputLogic = 1U,
	   .interruptMode = kGPIO_NoIntmode
	 };
	 /* Initialize GPIO functionality on GPIO_B1_12 (pin D13) */
	 GPIO_PinInit(I2C_MASTER_CLK_GPIO, I2C_MASTER_CLK_GPIO_PIN, &gpio2_pinD13_config);
	 GPIO_PinInit(I2C_MASTER_DATA_GPIO, I2C_MASTER_DATA_GPIO_PIN, &gpio2_pinD13_config);

}

static void i2c_internal_delay(int delay_loop){

	while(delay_loop--){
		__NOP;
	}
}

static void i2c_clock(void ){
#if USE_DSB_BLOCK
	I2C_SCL_LOW;
	i2c_internal_delay(I2C_HOLD_TIME);
	I2C_SCL_HIGH;
	i2c_internal_delay(I2C_HOLD_TIME);

#else

	i2c_internal_delay(I2C_HOLD_TIME);
	I2C_SCL_HIGH;
	i2c_internal_delay(I2C_HOLD_TIME);
	I2C_SCL_LOW;
	i2c_internal_delay(I2C_SETUP_TIME);
#endif

}

static void i2c_ack(void ){
#if USE_DSB_BLOCK
	I2C_SCL_LOW;
	I2C_SDA_LOW;
	i2c_internal_delay(I2C_SETUP_TIME);
	I2C_SCL_HIGH;
	i2c_internal_delay(I2C_SETUP_TIME);
#else

	I2C_SDA_LOW;
	i2c_internal_delay(I2C_SETUP_TIME);
	I2C_SCL_HIGH;
	i2c_internal_delay(I2C_HOLD_TIME);
	I2C_SCL_LOW;
	i2c_internal_delay(I2C_SETUP_TIME);
	I2C_SDA_HIGH;
	i2c_internal_delay(I2C_SETUP_TIME);

	//Release CLK Line
	I2C_SCL_HIGH;
	i2c_internal_delay(I2C_HOLD_TIME);
#endif	

}




static void i2c_noack(void ){
#if USE_DSB_BLOCK
	I2C_SCL_LOW;
	I2C_SDA_HIGH;
	i2c_internal_delay(I2C_HOLD_TIME);
	//Release CLK Line
	I2C_SCL_HIGH;
	i2c_internal_delay(I2C_HOLD_TIME);

#else

	I2C_SDA_HIGH;
	i2c_internal_delay(I2C_SETUP_TIME);
	I2C_SCL_HIGH;
	i2c_internal_delay(I2C_HOLD_TIME);
	I2C_SCL_LOW;
	i2c_internal_delay(I2C_SETUP_TIME);
	I2C_SDA_HIGH;
	i2c_internal_delay(I2C_SETUP_TIME);

	//Release CLK Line
	I2C_SCL_HIGH;
	i2c_internal_delay(I2C_HOLD_TIME);
#endif


}
static void i2c_write_bit(uint8_t bitv){
  bitv = (bitv > 0)?1:0;

  #if USE_DSB_BLOCK
	I2C_SCL_LOW;
	GPIO_PinWrite(I2C_MASTER_DATA_GPIO,I2C_MASTER_DATA_GPIO_PIN,bitv);
	i2c_internal_delay(I2C_HOLD_TIME);
	I2C_SCL_HIGH;
	i2c_internal_delay(I2C_HOLD_TIME);
  #else
  I2C_SCL_LOW;
  i2c_internal_delay(I2C_SETUP_TIME);
  GPIO_PinWrite(I2C_MASTER_DATA_GPIO,I2C_MASTER_DATA_GPIO_PIN,bitv);
  I2C_SCL_HIGH;
  i2c_internal_delay(I2C_HOLD_TIME);
  I2C_SCL_LOW;
  i2c_internal_delay(I2C_SETUP_TIME);
  #endif
}

static unsigned char i2c_read_bit(void ){
  //I2C_SCL_LOW;
  //i2c_internal_delay(I2C_SETUP_TIME);
  //I2C_SDA_HIGH;//release to idle
  //i2c_internal_delay(I2C_SETUP_TIME);
#if USE_DSB_BLOCK
	I2C_SCL_LOW;
	i2c_internal_delay(I2C_HOLD_TIME);

	I2C_SCL_HIGH;
	i2c_internal_delay(I2C_SETUP_TIME);
	unsigned char rb = GPIO_ReadPinInput(I2C_MASTER_DATA_GPIO,I2C_MASTER_DATA_GPIO_PIN);
	i2c_internal_delay(I2C_SETUP_TIME);
#else
  I2C_SCL_LOW;
  i2c_internal_delay(I2C_SETUP_TIME);
  
  I2C_SCL_HIGH;
  i2c_internal_delay(I2C_SETUP_TIME);
  unsigned char rb = GPIO_ReadPinInput(I2C_MASTER_DATA_GPIO,I2C_MASTER_DATA_GPIO_PIN);
  i2c_internal_delay(I2C_SETUP_TIME);
  
  I2C_SCL_LOW;
  i2c_internal_delay(I2C_SETUP_TIME);
#endif
  return rb;
}

void i2c_start_signal(void ){
#if USE_DSB_BLOCK
	I2C_SCL_HIGH;
	I2C_SDA_LOW;
#else
	I2C_SCL_HIGH;
	i2c_internal_delay(I2C_SETUP_TIME);
	
	I2C_SDA_LOW;
	i2c_internal_delay(I2C_SETUP_TIME);
	I2C_SCL_LOW;
	i2c_internal_delay(I2C_SETUP_TIME);
#endif
}


void i2c_stop_signal(void ){
#if USE_DSB_BLOCK
	I2C_SCL_LOW;
	I2C_SDA_LOW;
	i2c_internal_delay(I2C_SETUP_TIME);
	I2C_SCL_HIGH;
	I2C_SDA_HIGH;
	i2c_internal_delay(I2C_SETUP_TIME);
#else

  //A LOW-to-HIGH transition of the data line while the clock is HIGH is defined as the STOP condition
  I2C_SCL_LOW;
  i2c_internal_delay(I2C_SETUP_TIME);
  I2C_SDA_LOW;
  i2c_internal_delay(I2C_SETUP_TIME);
  I2C_SCL_HIGH;
  i2c_internal_delay(I2C_SETUP_TIME);
  I2C_SDA_HIGH;
  i2c_internal_delay(I2C_SETUP_TIME);
#endif
  
}

void i2c_back2idle_state(void ){
#if USE_DSB_BLOCK
	I2C_SCL_LOW;
	I2C_SDA_HIGH;
#else

	I2C_SCL_LOW;
	i2c_internal_delay(I2C_SETUP_TIME);
	I2C_SDA_HIGH;
	i2c_internal_delay(I2C_SETUP_TIME);
#endif
}


unsigned char i2c_send_slave_addr(unsigned char addr_rw){
	i2c_start_signal();
	
	unsigned char loop_cnt;
	for(loop_cnt=0;loop_cnt<8;loop_cnt++){
		i2c_write_bit((addr_rw<<loop_cnt)&0x80);
		
	}
	//GPIO_PinWrite(I2C_MASTER_DATA_GPIO,I2C_MASTER_DATA_GPIO_PIN,1);//release to idle
  	//i2c_internal_delay(I2C_SETUP_TIME);
	return i2c_read_bit();
}

unsigned char i2c_write_byte(unsigned char wv){
	unsigned char loop_cnt;
	for(loop_cnt=0;loop_cnt<8;loop_cnt++){
		i2c_write_bit((wv<<loop_cnt)&0x80);
	}
	if(wv&0x01){
		#if USE_DSB_BLOCK
		I2C_SDA_LOW;
		#else
		I2C_SDA_LOW;
		i2c_internal_delay(I2C_SETUP_TIME);
		#endif
	}
	//GPIO_PinWrite(I2C_MASTER_DATA_GPIO,I2C_MASTER_DATA_GPIO_PIN,1);//release to idle
  	//i2c_internal_delay(I2C_SETUP_TIME);
	return i2c_read_bit();
}


unsigned char i2c_read_byte(unsigned char ack){
	unsigned char loop_cnt,rv=0;
	/* GPIO configuration on GPIO_B1_12 (pin D13) */
	 gpio_pin_config_t gpio2_pinD13_config = {
	   .direction = kGPIO_DigitalInput,
	   .outputLogic = 1U,
	   .interruptMode = kGPIO_NoIntmode
	 };
	GPIO_PinInit(I2C_MASTER_DATA_GPIO, I2C_MASTER_DATA_GPIO_PIN, &gpio2_pinD13_config);
	for(loop_cnt=0;loop_cnt<8;loop_cnt++){
		rv<<=1;
		rv |= i2c_read_bit();
	}
	gpio2_pinD13_config.direction = kGPIO_DigitalOutput;
	GPIO_PinInit(I2C_MASTER_DATA_GPIO, I2C_MASTER_DATA_GPIO_PIN, &gpio2_pinD13_config);
	if(ack){
		i2c_ack();//Send ACK
	}else{

		i2c_noack();

	}
	return rv;
}


