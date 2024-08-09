
#include "sirojuLIB_MPU9250.h"

static float accelScalingFactor, gyroScalingFactor;
static I2C_HandleTypeDef i2cHandler;

HAL_StatusTypeDef mpu9250_init (I2C_HandleTypeDef *mpu_i2c)
{
	memcpy(&i2cHandler, mpu_i2c, sizeof(*mpu_i2c));
	//return HAL_I2C_IsDeviceReady (&i2cHandler, MPU9250_ADDR, 1, 10);
	return 0;
}

void I2C_Read(uint8_t SUB_ADDR, uint8_t *data, uint8_t NofData)
{
	HAL_I2C_Master_Transmit(&i2cHandler, MPU9250_ADDR, &SUB_ADDR, 1, 10);
	HAL_I2C_Master_Receive(&i2cHandler, MPU9250_ADDR, data, NofData, 10);
}

void I2C_Write8(uint8_t SUB_ADDR, uint8_t data)
{
	uint8_t i2cData[2];
	i2cData[0] = SUB_ADDR;
	i2cData[1] = data;
	HAL_I2C_Master_Transmit(&i2cHandler, MPU9250_ADDR, i2cData, 2,10);
}

void enableDataReadyInt ()
{
	I2C_Write8 (INT_ENABLE, 0x01);
}

void mpu9250_config (MPU_ConfigTypeDef *config)
{
	uint8_t mpu9250_buffer = 0;
	
	//I2C_Write8(USER_CTRL, 0x60);
	//I2C_Write8(PWR_MGMT_1, 0x80);
	
	I2C_Write8 (PWR_MGMT_1, 0x01);
	I2C_Write8 (USER_CTRL, 0x20);

	I2C_Write8 (PWR_MGMT_1, 0x80); 
	HAL_Delay (1);

	I2C_Write8 (PWR_MGMT_1, 0x01); 
	I2C_Write8 (PWR_MGMT_2, 0x00); 
	
	HAL_Delay(100);
	mpu9250_buffer = config ->clockSource & 0x07;
	mpu9250_buffer |= (config ->sleepModeBit << 6) &0x40; 
	I2C_Write8(PWR_MGMT_1, mpu9250_buffer);
	HAL_Delay(100);
	
	mpu9250_buffer = 0;
	mpu9250_buffer = config->CONFIG_DLPF & 0x07;
	I2C_Write8(CONFIG, mpu9250_buffer);
	
	mpu9250_buffer = 0;
	mpu9250_buffer = (config->gyroFullScale << 3) & 0x18;
	I2C_Write8(GYRO_CONFIG, mpu9250_buffer);
	
	mpu9250_buffer = 0; 
	mpu9250_buffer = (config->accelFullScale << 3) & 0x18;
	I2C_Write8(ACCEL_CONFIG, mpu9250_buffer);
	
	mpu9250_Set_SMPRT_DIV(0x04);
	
	switch (config->accelFullScale)
	{
		case AFS_SEL_2g:  accelScalingFactor = (2.0/32768.0)*9.80665;  break;
		case AFS_SEL_4g:  accelScalingFactor = (4.0/32768.0)*9.80665;  break;
		case AFS_SEL_8g:  accelScalingFactor = (8.0/32768.0)*9.80665;  break;
		case AFS_SEL_16g: accelScalingFactor = (16.0/32768.0)*9.80665; break;
	}
	
	switch (config->gyroFullScale)
	{
		case FS_SEL_250:  gyroScalingFactor = 250.0/32768.0;  break;
		case FS_SEL_500:  gyroScalingFactor = 500.0/32768.0;  break;
		case FS_SEL_1000: gyroScalingFactor = 1000.0/32768.0; break;
		case FS_SEL_2000: gyroScalingFactor = 2000.0/32768.0; break;
	}
	
	I2C_Write8 (I2C_MST_CTRL, 0x00);
	I2C_Write8 (INT_PIN_CFG, 0x02);
	I2C_Write8 (USER_CTRL, 0x00);
}

void mpu9250_Set_SMPRT_DIV(uint8_t SMPRTvalue)
{
	I2C_Write8(SMPLRT_DIV, SMPRTvalue);
}

void mpu9250_readAccelScale(float *accel_x_scale, float *accel_y_scale, float *accel_z_scale)
{
	uint8_t recData[6];
	I2C_Read (ACCEL_XOUT_H, recData, 6);
	*accel_x_scale = (int16_t)(recData[0] << 8 | recData[1]) * accelScalingFactor;// - 9.2;
	*accel_y_scale = (int16_t)(recData[2] << 8 | recData[3]) * accelScalingFactor;// - 0;
	*accel_z_scale = (int16_t)(recData[4] << 8 | recData[5]) * accelScalingFactor;// - 1.0;
}

void mpu9250_readGyroScale (float *gyro_x_scale, float *gyro_y_scale, float *gyro_z_scale)
{
	uint8_t recData[6];
	I2C_Read (GYRO_XOUT_H, recData, 6);
	*gyro_x_scale = (int16_t)(recData[0] << 8 | recData[1]) * gyroScalingFactor;
	*gyro_y_scale = (int16_t)(recData[2] << 8 | recData[3]) * gyroScalingFactor;
	*gyro_z_scale = (int16_t)(recData[4] << 8 | recData[5]) * gyroScalingFactor;
}










