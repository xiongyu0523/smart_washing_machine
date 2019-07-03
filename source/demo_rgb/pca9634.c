#include "board.h"
#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "fsl_debug_console.h"

#define PCA9634_ALLCALL_ADDR 	0x15
#define PCA9634_SOFRST_ADDR		0x03

unsigned char pca_write_reg(unsigned char regaddr, unsigned char data){
	unsigned char wv = PCA9634_ALLCALL_ADDR;
	unsigned char ret = 1;
	wv<<=1;
	wv&=0xfe;
	ret = i2c_send_slave_addr(wv);
	if(ret != 0){
		i2c_stop_signal();
	}else{
		ret = i2c_write_byte(regaddr);
		ret = i2c_write_byte(data);
		i2c_stop_signal();
	}
	return ret;
}

unsigned char pca_read_reg(unsigned char regaddr, unsigned char *red_value){
	unsigned char wv = PCA9634_ALLCALL_ADDR;
	unsigned char ret = 1;
	wv<<=1;
	wv&=0xfe;
	ret = i2c_send_slave_addr(wv);
	if(ret != 0){
		i2c_stop_signal();
	}else{
		ret = i2c_write_byte(regaddr);
		wv |= 1;
		i2c_back2idle_state();
		ret = i2c_send_slave_addr(wv);
		*red_value = i2c_read_byte(0);
		i2c_stop_signal();
	}
	return ret;
}


unsigned char pca_reset(void ){
	unsigned char wv = PCA9634_SOFRST_ADDR;
	unsigned char ret = 1;
	wv<<=1;
	wv&=0xfe;
	ret = i2c_send_slave_addr(wv);
	if(ret != 0){
		i2c_stop_signal();
		PRINTF("PCA9634 Reset failed, no ACK\r\n");
	}else{
		ret = i2c_write_byte(0xa5);
		ret = i2c_write_byte(0x5a);
		i2c_stop_signal();
		PRINTF("PCA9634 Software reset completed\r\n");
	}
	
	return ret;
}
unsigned char pca9634_mode_configure(void ){

	unsigned char ret = 1;

	ret = pca_write_reg(0,1);
	ret = pca_write_reg(1,8);
	ret = pca_write_reg(0x0c,0xff);

		/* GPIO configuration on GPIO_B1_12 (pin D13) */
	 gpio_pin_config_t gpio2_pinD13_config = {
	   .direction = kGPIO_DigitalOutput,
	   .outputLogic = 0U,
	   .interruptMode = kGPIO_NoIntmode
	 };
	 /* Initialize GPIO functionality on GPIO_B1_12 (pin D13) */
	 GPIO_PinInit(GPIO1, 18, &gpio2_pinD13_config);
	PRINTF("PCA9634 config completed\r\n");

	return ret;
}



void pca9634_init(void ){

	IOMUXC_SetPinMux(
		IOMUXC_GPIO_AD_B1_02_GPIO1_IO18,		/* GPIO_AD_B0_09 is configured as GPIO1_IO09 */
		0U);									/* Software Input On Field: Input Path is determined by functionality */
	IOMUXC_SetPinConfig(
		IOMUXC_GPIO_AD_B1_02_GPIO1_IO18,		/* GPIO_AD_B0_09 PAD functional properties : */
		0x10B0u);								/* Slew Rate Field: Slow Slew Rate
											   Drive Strength Field: R0/6
											   Speed Field: medium(100MHz)
											   Open Drain Enable Field: Open Drain Disabled
											   Pull / Keep Enable Field: Pull/Keeper Enabled
											   Pull / Keep Select Field: Keeper
											   Pull Up / Down Config. Field: 100K Ohm Pull Down
											   Hyst. Enable Field: Hysteresis Disabled */
												
	i2c_master_init();
	unsigned char ret = pca_reset();
	PRINTF("PCA9634 Init result %d\r\n",ret);
	pca9634_mode_configure();
	
}





unsigned char pca_blue_value_set(unsigned char bv){
	unsigned char wv = PCA9634_ALLCALL_ADDR;
	unsigned char ret = 1;
	wv<<=1;
	wv&=0xfe;
	ret = i2c_send_slave_addr(wv);
	if(ret != 0){
                i2c_stop_signal();
		PRINTF("PCA9634 blue set failed, no ACK\r\n");
	}else{
		ret = i2c_write_byte(0x02);
		ret = i2c_write_byte(bv);
                i2c_stop_signal();
		PRINTF("PCA9634 blue value %d set success\r\n",bv);
	}
	return ret;

	

}
unsigned char pca_green_value_set(unsigned char gv){
	unsigned char wv = PCA9634_ALLCALL_ADDR;
	unsigned char ret = 1;
	wv<<=1;
	wv&=0xfe;
	ret = i2c_send_slave_addr(wv);
	if(ret != 0){
                i2c_stop_signal();
		PRINTF("PCA9634 green set failed, no ACK\r\n");
	}else{
		ret = i2c_write_byte(0x03);
		ret = i2c_write_byte(gv);
                i2c_stop_signal();
		PRINTF("PCA9634 green value %d set success\r\n",gv);
	}

	return ret;


}

unsigned char pca_red_value_set(unsigned char rv){

	unsigned char wv = PCA9634_ALLCALL_ADDR;
	unsigned char ret = 1;
	wv<<=1;
	wv&=0xfe;
	ret = i2c_send_slave_addr(wv);
	if(ret != 0){
                i2c_stop_signal();
		PRINTF("PCA9634 red set failed, no ACK\r\n");
	}else{
		i2c_write_byte(0x04);
		i2c_write_byte(rv);
		i2c_stop_signal();
		PRINTF("PCA9634 red value %d set success\r\n",rv);
	}
	return ret;



}



