
//#include "stm32l0xx_hal.h"   
//#include "stm32f1xx_hal.h"   

#include "stm32f1xx_hal.h"
#include <string.h>

#define SELF_TEST_X_GYRO 		0x00
#define SELF_TEST_Y_GYRO 		0x01
#define SELF_TEST_Z_GYRO 		0x02
#define SELF_TEST_X_ACCEL 	0x0D
#define SELF_TEST_Y_ACCEL 	0x0E
#define SELF_TEST_Z_ACCEL 	0x0F
#define XG_OFFSET_H 				0x13
#define XG_OFFSET_L 				0x14
#define YG_OFFSET_H 				0x15
#define YG_OFFSET_L 				0x16
#define ZG_OFFSET_H 				0x17
#define ZG_OFFSET_L 				0x18
#define SMPLRT_DIV 					0x19
#define CONFIG 							0x1A
#define GYRO_CONFIG 				0x1B
#define ACCEL_CONFIG 				0x1C
#define ACCEL_CONFIG_2  		0x1D
#define LP_ACCEL_ODR 				0x1E
#define WOM_THR 						0x1F
#define FIFO_EN 						0x23
#define I2C_MST_CTRL 				0x24
#define I2C_SLV0_ADDR 			0x25
#define I2C_SLV0_REG 				0x26
#define I2C_SLV0_CTRL 			0x27
#define I2C_SLV1_ADDR 			0x28
#define I2C_SLV1_REG 				0x29
#define I2C_SLV1_CTRL 			0x2A
#define I2C_SLV2_ADDR 			0x2B
#define I2C_SLV2_REG 				0x2C
#define I2C_SLV2_CTRL 			0x2D
#define I2C_SLV3_ADDR 			0x2E
#define I2C_SLV3_REG 				0x2F
#define I2C_SLV3_CTRL 			0x30
#define I2C_SLV4_ADDR 			0x31
#define I2C_SLV4_REG 				0x32
#define I2C_SLV4_DO 				0x33
#define I2C_SLV4_CTRL 			0x34
#define I2C_SLV4_DI 				0x35
#define I2C_MST_STATUS 			0x36
#define INT_PIN_CFG 				0x37
#define INT_ENABLE 					0x38
#define INT_STATUS 					0x3A
#define ACCEL_XOUT_H 				0x3B
#define ACCEL_XOUT_L 				0x3C
#define ACCEL_YOUT_H 				0x3D
#define ACCEL_YOUT_L 				0x3E
#define ACCEL_ZOUT_H 				0x3F
#define ACCEL_ZOUT_L 				0x40
#define TEMP_OUT_H 					0x41
#define TEMP_OUT_L 					0x42
#define GYRO_XOUT_H 				0x43
#define GYRO_XOUT_L 				0x44
#define GYRO_YOUT_H 				0x45
#define GYRO_YOUT_L 				0x46
#define GYRO_ZOUT_H 				0x47
#define GYRO_ZOUT_L 				0x48
#define EXT_SENS_DATA_00 		0x49
#define EXT_SENS_DATA_01 		0x4A
#define EXT_SENS_DATA_02 		0x4B
#define EXT_SENS_DATA_03 		0x4C
#define EXT_SENS_DATA_04 		0x4D
#define EXT_SENS_DATA_05 		0x4E
#define EXT_SENS_DATA_06 		0x4F
#define EXT_SENS_DATA_07 		0x50
#define EXT_SENS_DATA_08 		0x51
#define EXT_SENS_DATA_09 		0x52
#define EXT_SENS_DATA_10 		0x53
#define EXT_SENS_DATA_11 		0x54
#define EXT_SENS_DATA_12 		0x55
#define EXT_SENS_DATA_13 		0x56
#define EXT_SENS_DATA_14 		0x57
#define EXT_SENS_DATA_15 		0x58
#define EXT_SENS_DATA_16 		0x59
#define EXT_SENS_DATA_17 		0x5A
#define EXT_SENS_DATA_18 		0x5B
#define EXT_SENS_DATA_19 		0x5C
#define EXT_SENS_DATA_20 		0x5D
#define EXT_SENS_DATA_21 		0x5E
#define EXT_SENS_DATA_22 		0x5F
#define EXT_SENS_DATA_23 		0x60
#define I2C_SLV0_DO 				0x63
#define I2C_SLV1_DO 				0x64
#define I2C_SLV2_DO 				0x65
#define I2C_SLV3_DO 				0x66
#define I2C_MST_DELAY_CTRL 	0x67
#define SIGNAL_PATH_RESET  	0x68
#define MOT_DETECT_CTRL 		0x69
#define USER_CTRL 					0x6A
#define PWR_MGMT_1 					0x6B
#define PWR_MGMT_2 					0x6C
#define FIFO_COUNTH 				0x72
#define FIFO_COUNTL 				0x73
#define FIFO_R_W 						0x74
#define WHO_AM_I 						0x75
#define XA_OFFSET_H 				0x77
#define XA_OFFSET_L 				0x78
#define YA_OFFSET_H 				0x7A
#define YA_OFFSET_L 				0x7B
#define ZA_OFFSET_H 				0x7D
#define ZA_OFFSET_L 				0x7E

#define HXL 								0x03
#define HXH 								0x04
#define HYL 								0x05
#define HYH 								0x06
#define HZL 								0x07
#define HZH 								0x08
#define CNTL1								0x0A
#define CNTL2								0x0B
#define ASTC								0x0C
#define TS1									0x0D
#define TS2									0x0E
#define I2CDIS							0x0F
#define AK8963_ASA					0x10

#define PWR_DOWN						0x00

#define I2C_READ_FLAG				0x80
#define I2C_SLV0_EN 				0x80

#define MPU9250_ADDR 				(0x68<<1)


typedef struct
{
	uint8_t clockSource;
	uint8_t gyroFullScale;
	uint8_t accelFullScale;
	uint8_t CONFIG_DLPF;
	_Bool 	sleepModeBit; 
	
}MPU_ConfigTypeDef;

enum PM_CLKSEL_ENUM
{
	Internal_8MHz 	= 0x00,
	X_Axis_Ref			= 0x01,
	Y_Axis_Ref			= 0x02,
	Z_Axis_Ref			= 0x03,
	Ext_32_768KHz		= 0x04,
	Ext_19_2MHz			= 0x05,
	TIM_GENT_INREST	= 0x07
};

enum gyro_FullScale_ENUM
{
	FS_SEL_250 	= 0x00,
	FS_SEL_500 	= 0x01,
	FS_SEL_1000 = 0x02,
	FS_SEL_2000	= 0x03
};

enum accel_FullScale_ENUM
{
	AFS_SEL_2g	= 0x00,
	AFS_SEL_4g,
	AFS_SEL_8g,
	AFS_SEL_16g
};

enum DLPF_CFG_ENUM
{
	DLPF_260A_256G_Hz = 0x00,
	DLPF_184A_188G_Hz = 0x01,
	DLPF_94A_98G_Hz 	= 0x02,
	DLPF_44A_42G_Hz 	= 0x03,
	DLPF_21A_20G_Hz 	= 0x04,
	DLPF_10_Hz 				= 0x05,
	DLPF_5_Hz 				= 0x06
};


HAL_StatusTypeDef mpu9250_init (I2C_HandleTypeDef *mpu_i2c);
void mpu9250_config (MPU_ConfigTypeDef *config);
void mpu9250_Set_SMPRT_DIV (uint8_t SMPRTvalue);
void mpu9250_readAccelScale (float *accel_x_scale, float *accel_y_scale, float *accel_z_scale);
void mpu9250_readGyroScale (float *gyro_x_scale, float *gyro_y_scale, float *gyro_z_scale);
void enableDataReadyInt (void);
void I2C_Write8(uint8_t SUB_ADDR, uint8_t data);
void I2C_Read(uint8_t SUB_ADDR, uint8_t *data, uint8_t NofData);










