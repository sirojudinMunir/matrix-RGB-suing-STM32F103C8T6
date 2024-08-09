/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sirojuLIB_MPU9250.h"
#include "sirojuLIB_character.h"
#include "math.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_SAND 30
#define MAX_FOODS 6
#define TIMER_USED 9

#define BUZZER_OFF
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */

/********************************************************************************************/

enum axis
{
	x, y, z
};

enum directions
{
	up, down, left, right
};

enum foods_menu_
{
	_watermelon, _orange, _kiwi
};

enum face
{
	_greeting, _normal, _sleep, _wake_up, _angry, _happy, _laugh, _impressed,
	_low_bat, _charg, _emotic_select,
	_hungry, _food_select, _eat_time, _happy_aft_eat,
	_maze, _snake, _sand, _victory, _game_over,
	_pat_cat,
};

enum emotic_mode
{
	_emotic_senang, _emotic_sedih, _emotic_takut, _emotic_jijik, _emotic_marah, _emotic_terkejut, _emotic_hampa
};

/********************************************************************************************/

_Bool 		kunyah[16][15], font_buff[16][15],
			blink = 0, last_blink = 0, sleepy, sleep_mouth, s_det[5],
			dir_wm_motion = 0,
			eat_mouth = 0,
			enter_snake_g = 0, snake_game_over = 0,
			enter_maze = 0, game_over_anm = 0, maze_mode, mf_blink = 0,
			end_text = 0, food_s = 0,
			charg = 0, last_charg = 0, start_bat_read = 0,
			bz_n = 0, nada_lompat = 0, bz_state = 0, emo_s_state = 0;

char 		text_buffer[200];

int8_t 		sand_pos[MAX_SAND][2], last_sand_pos[MAX_SAND][2];

uint8_t 	line = 0, pwm = 0,
			color_buffer[16][15],
			ball_shadow_temp[15][16],
			wm_count = 0,
			eye[2], last_eye[2] = {10, 10}, s_count[5], face_emotion = _greeting, last_face_emotion,
			dimmer_count = 0,
			eat_h = 0, food_count = 0,
			pat_count = 0,
			laugh_count = 0,
			snake_dir = 0, last_snake_dir = 0, snake_pos, snake_food, snake_tail_pos[200], snake_tail = 0,
			maze_pos[2], last_maze_pos[2], game_over_count = 0, maze_dest[2], maze_finish_dest,
			plugin_motion_step = 0, plugin_motion_count = 0, eat_count = 0,
			emo_select_count = _emotic_senang;

uint16_t 	r_bit, g_bit, b_bit, text_r = 0, pixel_r = 0, delay_anm = 200,
			t_song_intro = 200, n_nada = 0;

uint32_t 	timer[TIMER_USED], time_wm_motion = 20;

float 		accel_raw[3], gyro_raw[3],
			sand_pos_temp[MAX_SAND][2], v_ball[MAX_SAND][2],
			v_bat;

/********************************************************************************************/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/********************************************************************************************/

void read_battery ()
{
	uint32_t adc;
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1);
	adc = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	float volt = adc * 0.001904761904761904;
	v_bat = v_bat * 0.5 +  volt * 0.5;
}

/********************************************************************************************/

void buzzer (int8_t n)
{
	TIM4->ARR = (float)(1136.36363636363 / pow (2.0, (float)n / 12.0));
}

void buzzer_on ()
{
#ifdef BUZZER_ON
	if (!bz_state)
	{
		bz_state = 1;
		HAL_TIM_OC_Start(&htim4, TIM_CHANNEL_4);
	}
#endif
}

void buzzer_off ()
{
#ifdef BUZZER_ON
	if (bz_state)
	{
		bz_state = 0;
		HAL_TIM_OC_Stop(&htim4, TIM_CHANNEL_4);
		GPIOB->BRR = GPIO_PIN_9;
	}
#endif
}

/********************************************************************************************/

void shift_reg (uint16_t r, uint16_t g, uint16_t b)
{
	for (uint8_t i = 0; i < 15; i++)
	{
		DATA_R_GPIO_Port->BSRR = DATA_R_Pin<<((r >> i & 1)?0:16);
		DATA_G_GPIO_Port->BSRR = DATA_G_Pin<<((g >> i & 1)?0:16);
		DATA_B_GPIO_Port->BSRR = DATA_B_Pin<<((b >> i & 1)?0:16);
		CLK_GPIO_Port->BSRR = CLK_Pin;
		CLK_GPIO_Port->BSRR = CLK_Pin<<16;
	}
	LOAD_GPIO_Port->BSRR = LOAD_Pin;
	LOAD_GPIO_Port->BSRR = LOAD_Pin<<16;
}

/********************************************************************************************/

void set_colom (uint16_t c)
{
	if (c < 9)
	{
		GPIOB->ODR |= 0xf038;
		GPIOA->ODR = (GPIOA->ODR | 0x07fc) & (~(4 << c) | 0xf803);
	}
	else if (c > 11)
	{
		GPIOA->ODR |= 0x07fc;
		GPIOB->ODR = (GPIOB->ODR | 0xf038) & (~(1 << c) | 0x0fff);
	}
	else
	{
		GPIOA->ODR |= 0x07fc;
		GPIOB->ODR = (GPIOB->ODR | 0xf038) & (~(8 << (c-9)) | 0xffc7);
	}
}

#if 0

void scanning ()
{
	r_bit = 0;
	g_bit = 0;
	b_bit = 0;
	for (uint8_t i = 0; i < 15; i++)
	{
		r_bit |= (((color_buffer[line][i] >> 5) & 7) > pwm) << i;
		g_bit |= (((color_buffer[line][i] >> 3) & 3) > pwm) << i;
		b_bit |= ((color_buffer[line][i] & 7) > pwm) << i;
	}
	if (line != last_line)
	{
		last_line = line;
		set_colom (line);
	}
	shift_reg (r_bit, g_bit, b_bit);
	pwm++;
	if (pwm > 7)
	{
		pwm = 0;
		line++;
		if (line >= 16) line = 0;
	}
}

#else

void manual_scan ()
{
	for (line = 0; line < 16; line++)
	{
		set_colom (line);
		for (pwm = 0; pwm < 18; pwm++)
		{
			r_bit = 0;
			g_bit = 0;
			b_bit = 0;
			for (uint8_t i = 0; i < 15; i++)
			{
				r_bit |= (((color_buffer[line][i] >> 5) & 7) > pwm) << i;
				g_bit |= (((color_buffer[line][i] >> 3) & 3) > pwm) << i;
				b_bit |= ((color_buffer[line][i] & 7) > pwm) << i;
			}
			shift_reg (r_bit, g_bit, b_bit);
		}
	}
}
#endif

void led_test ()
{
	for (uint8_t x = 0; x < 15; x++)
		for (uint8_t y = 0; y < 16; y++)
		{
			uint32_t d = HAL_GetTick() - timer[2];
			if (d >= 5000)
			{
				timer[2] = HAL_GetTick();
			}
			else if (d < 1000)
				color_buffer[15-y][x] = 0xff;
			else if ((d >= 1000) && (d < 2000))
				color_buffer[15-y][x] = 0xe0;
			else if ((d >= 2000) && (d < 3000))
				color_buffer[15-y][x] = 0x18;
			else if ((d >= 3000) && (d < 4000))
				color_buffer[15-y][x] = 0x07;
			else if ((d >= 4000) && (d < 5000))
				color_buffer[15-y][x] = 0x00;
		}
}


/********************************************************************************************/

uint8_t circle_array (const uint8_t *data, uint8_t x, uint8_t y)
{
	uint8_t result;
	switch (y)
	{
	case 0:
		if (x > 4 && x < 10)
			result = data[x-5];
	break;
	case 1:
		if (x > 2 && x < 12)
			result = data[x+2];
	break;
	case 2:
		if (x > 1 && x < 13)
			result = data[x+12];
	break;
	case 3:
		if (x > 0 && x < 14)
			result = data[x+24];
	break;
	case 4:
		if (x > 0 && x < 14)
			result = data[x+37];
	break;
	case 5:
		result = data[x+51];
	break;
	case 6:
		result = data[x+66];
	break;
	case 7:
		result = data[x+81];
	break;
	case 8:
		result = data[x+96];
	break;
	case 9:
		result = data[x+111];
	break;
	case 10:
		result = data[x+126];
	break;
	case 11:
		if (x > 0 && x < 14)
			result = data[x+140];
	break;
	case 12:
		if (x > 0 && x < 14)
			result = data[x+153];
	break;
	case 13:
		if (x > 1 && x < 13)
			result = data[x+165];
	break;
	case 14:
		if (x > 2 && x < 12)
			result = data[x+175];
	break;
	case 15:
		if (x > 4 && x < 10)
			result = data[x+182];
	break;
	}
	return result;
}

void send_to_buffer (const uint8_t *data)
{
	  for (uint8_t y = 0; y < 16; y++)
		  for (uint8_t x = 0; x < 15; x++)
			  color_buffer[15-y][x] = circle_array(data, x, y);
}

/********************************************************************************************/

_Bool circle_array_bit (const _Bool *data, uint8_t x, uint8_t y)
{
	_Bool result;
	switch (y)
	{
	case 0:
		if (x > 4 && x < 10)
			result = data[x-5];
	break;
	case 1:
		if (x > 2 && x < 12)
			result = data[x+2];
	break;
	case 2:
		if (x > 1 && x < 13)
			result = data[x+12];
	break;
	case 3:
		if (x > 0 && x < 14)
			result = data[x+24];
	break;
	case 4:
		if (x > 0 && x < 14)
			result = data[x+37];
	break;
	case 5:
		result = data[x+51];
	break;
	case 6:
		result = data[x+66];
	break;
	case 7:
		result = data[x+81];
	break;
	case 8:
		result = data[x+96];
	break;
	case 9:
		result = data[x+111];
	break;
	case 10:
		result = data[x+126];
	break;
	case 11:
		if (x > 0 && x < 14)
			result = data[x+140];
	break;
	case 12:
		if (x > 0 && x < 14)
			result = data[x+153];
	break;
	case 13:
		if (x > 1 && x < 13)
			result = data[x+165];
	break;
	case 14:
		if (x > 2 && x < 12)
			result = data[x+175];
	break;
	case 15:
		if (x > 4 && x < 10)
			result = data[x+182];
	break;
	}
	return result;
}

void emotic (uint8_t emo_face)
{
  for (uint8_t y = 0; y < 16; y++)
	  for (uint8_t x = 0; x < 15; x++)
	  {
			switch (emo_face)
			{
			case _emotic_senang:
				if (circle_array_bit(senang[1], x, y))
				{
				  if (y < 6) color_buffer[15-y][x] = 0x2f;
				  else color_buffer[15-y][x] = 0xe0;
				}
				else color_buffer[15-y][x] = 0x00;
				break;
			case _emotic_sedih:
				if (circle_array_bit(syedih, x, y))
				{
				  if (y < 6 && (x != 3 || x != 11)) color_buffer[15-y][x] = 0x2f;
				  else if (y > 4 && (x == 3 || x == 11)) color_buffer[15-y][x] = 0x07;
				  else if (y > 8) color_buffer[15-y][x] = 0xe0;
				}
				else color_buffer[15-y][x] = 0x00;
				break;
			case _emotic_takut:
				  if (circle_array_bit(lapar[0], x, y))
				  {
					  if (y < 9) color_buffer[15-y][x] = 0x2f;
					  else color_buffer[15-y][x] = 0xe0;
				  }
				  else color_buffer[15-y][x] = 0x00;
				break;
			case _emotic_jijik:
				if (circle_array_bit(jijik, x, y))
				{
				  if (y < 8)
				  {
					  if ((x == 4 || x == 11) && y == 6) color_buffer[15-y][x] = 0xff;
					  else
					  {
						  if ((x == 3 || x == 4 || x == 10 || x == 11) && (y == 6 || y == 7)) color_buffer[15-y][x] = 0x08;
						  else color_buffer[15-y][x] = 0x7b;
					  }
				  }
				  else color_buffer[15-y][x] = 0xe0;
				}
				else color_buffer[15-y][x] = 0x00;
				break;
			case _emotic_marah:
				if (circle_array_bit(kezel_parah, x, y))
				{
				  if (y < 9)
				  {
					  if ((x == 4 || x == 11) && y == 6) color_buffer[15-y][x] = 0xff;
					  else
					  {
						  if ((x == 3 || x == 4 || x == 10 || x == 11) && (y == 6 || y == 7)) color_buffer[15-y][x] = 0xe0;
						  else color_buffer[15-y][x] = 0xe9;
					  }
				  }
				  else color_buffer[15-y][x] = 0xe0;
				}
				else color_buffer[15-y][x] = 0x00;
				break;
			case _emotic_terkejut:
				if (circle_array_bit(terkejot, x, y))
				{
				  if (y < 10)
				  {
					  if ((x == 4 || x == 12) && y == 5) color_buffer[15-y][x] = 0xff;
					  else
					  {
						  if ((x == 3 || x == 11) && (y == 6 || y == 7)) color_buffer[15-y][x] = 0x07;
						  else color_buffer[15-y][x] = 0x2f;
					  }
				  }
				  else color_buffer[15-y][x] = 0xe0;
				}
				else color_buffer[15-y][x] = 0x00;
				break;
			case _emotic_hampa:
				if (circle_array_bit(hampa, x, y)) color_buffer[15-y][x] = 0x2f;
				else color_buffer[15-y][x] = 0x00;
				break;
			}
	  }
}

/********************************************************************************************/

void update_eye_pos ()
{
	  //for eye motion -------------------------------------------------
	  eye[x] = 3 - accel_raw[x] / 5.0;
	  eye[y] = 4 + accel_raw[y] / 5.0;

	  if (eye[y] == 3 || eye[y] == 6)
	  {
		  if (eye[x] < 2) eye[x] = 2;
		  if (eye[x] > 4) eye[x] = 4;
	  }
	  else

	  {
		  if (eye[x] < 1) eye[x] = 1;
		  if (eye[x] > 5) eye[x] = 5;
	  }
	  if (eye[x] == 1 || eye[x] == 5)
	  {
		  if (eye[y] < 4) eye[y] = 4;
		  if (eye[y] > 5) eye[y] = 5;
	  }
	  else
	  {
		  if (eye[y] < 3) eye[y] = 3;
		  if (eye[y] > 6) eye[y] = 6;
	  }
}

/********************************************************************************************/

void eye_motion ()
{
	  //blink----------------------------------------
	  if (!blink)
	  {
		  if (HAL_GetTick() - timer[0] >= 4000)
		  {
			  timer[0] = HAL_GetTick();
			  blink = 1;
			  buzzer_on ();
			  buzzer (18);
		  }
	  }
	  else
	  {
		  if (HAL_GetTick() - timer[0] >= 100)
		  {
			  timer[0] = HAL_GetTick();
			  blink = 0;
			  buzzer_off ();
		  }
		  for (uint8_t y = 0; y < 16; y++)
			  for (uint8_t x = 0; x < 15; x++)
			  {
				  if (circle_array_bit(eye_blink, x, y)) color_buffer[15-y][x] = 0x2f;
				  else color_buffer[15-y][x] = 0x00;
			  }
	  }
	  //update display ---------------------
	  if (eye[x] != last_eye[x] || eye[y] != last_eye[y] || last_blink != blink)
	  {
		  if (eye[x] != last_eye[x] || eye[y] != last_eye[y]) timer[1] = HAL_GetTick();
		  if (eye[x] != last_eye[x]) timer[6] = HAL_GetTick();
		  for (uint8_t y = 0; y < 16; y++)
			  for (uint8_t x = 0; x < 15; x++)
			  {
				  if ((x == eye[0] || x == eye[0]+8) && (y == eye[1] || y == eye[1]+1))
					  color_buffer[15-y][x] = 0x00;
				  else
				  	  color_buffer[15-y][x] = circle_array(emotion, x, y);
			  }
		  last_eye[x] = eye[x];
		  last_eye[y] = eye[y];
		  last_blink = blink;
	  }
}

/********************************************************************************************/

void sleep_motion ()
{
	for (uint8_t y = 0; y < 16; y++)
	  for (uint8_t x = 0; x < 15; x++)
	  {
		  if (circle_array_bit(eye_blink, x, y)) color_buffer[15-y][x] = 0x2f;
		  else
		  {
			  if (sleep_mouth)
			  {
				  if (x == 7 && y > 9 && y < 15) color_buffer[15-y][x] = 0x20;
				  else color_buffer[15-y][x] = 0x00;
			  }
			  else
			  {
				  if (x == 7 && y == 12) color_buffer[15-y][x] = 0x20;
				  else color_buffer[15-y][x] = 0x00;
			  }
		  }
	  }

	if (HAL_GetTick() - timer[3] >= 1000)
	{
		timer[3] = HAL_GetTick();
		if (sleep_mouth) sleep_mouth = 0;
		else sleep_mouth = 1;
	}
}

/********************************************************************************************/

void wakeup_motion ()
{
	if (HAL_GetTick() - timer[3] >= time_wm_motion)
	{
		timer[3] = HAL_GetTick();
		wm_count++;
		if (wm_count > 45)
		{
			wm_count = 0;
			buzzer_off ();
			face_emotion = _normal;
			blink = 1;
		}
		if (wm_count <= 40)
		{
			if (dir_wm_motion)
			{
				eye[y]--;
				if (eye[y] == 3) dir_wm_motion = 0;
			}
			else
			{
				eye[y]++;
				if (eye[y] == 6) dir_wm_motion = 1;
			}
			if (wm_count % 2)
			{
				buzzer_on ();
				buzzer ((wm_count % 4)? 7: 6);
			}
			else
			{
				buzzer_off ();
			}
		}
		else
		{
			time_wm_motion = 50;
			if (blink)
			{
				blink = 0;
			}
			else
			{
				blink = 1;
			}
		}
	}
	eye_motion ();
}

/********************************************************************************************/

void food_menu ()
{
	  mpu9250_readAccelScale (&accel_raw[y], &accel_raw[x], &accel_raw[z]);
	  if (accel_raw[z] > 9)
	  {
		  if (accel_raw[z] > fabs(accel_raw[x]))
		  {
				for (uint8_t y = 0; y < 16; y++)
					for (uint8_t x = 0; x < 15; x++)
					{
						kunyah[y][x] = 0;
						if (y < 9)
						{
						  if (circle_array_bit(senang[0], x, y)) color_buffer[15-y][x] = 0x2f;
						  else color_buffer[15-y][x] = 0x00;
						}
						else color_buffer[15-y][x] = circle_array(makanan[food_count], x, y);
					}
				buzzer_off ();
				face_emotion = _eat_time;
		  }
	  }
	  else
	  {
		  if (accel_raw[x] >= 9)
		  {
			  if (!food_s) food_s = 1;
		  }
		  else if (accel_raw[x] <= -9)
		  {
			  if (food_s)
			  {
				  food_s = 0;
				  timer[7] = HAL_GetTick();
				  buzzer_on ();
				  buzzer (5);
				  food_count++;
				  if (food_count >= MAX_FOODS) food_count = 0;
					for (uint8_t y = 0; y < 16; y++)
						for (uint8_t x = 0; x < 15; x++)
							color_buffer[15-y][x] = circle_array(makanan[food_count], x, y);
			  }
		  }
	  }
	  if (HAL_GetTick() - timer[7] >= 50)
	  {
		  buzzer_off ();
	  }
}

/********************************************************************************************/

void eat_motion ()
{
	mpu9250_readAccelScale (&accel_raw[y], &accel_raw[x], &accel_raw[z]);

	if (accel_raw[x] >= 9)
	{
		if (eat_h == 0) eat_h = 1;
	}
	else if (accel_raw[x] <= -9)
	{
		if (eat_h == 1)
		{
			eat_h = 2;
			timer[1] = HAL_GetTick();
		}
	}

	if (eat_h == 2)
	{
		if (HAL_GetTick() - timer[2] >= 500)
		{
			timer[2] = HAL_GetTick();
			if (eat_mouth)
			{
				eat_mouth = 0;
				eat_h = 0;
				eat_count++;
			}
			else
			{
				eat_mouth = 1;
			}
			if (eat_count > 9)
			{
				timer[5] = HAL_GetTick();
				timer[2] = HAL_GetTick();
				eat_count = 0;
				face_emotion = _happy_aft_eat;
			}

			  switch (eat_count)
			  {
			  case 0:
				  kunyah[9][6] = 1;
		  	  	  kunyah[9][7] = 1;
		  	  	  kunyah[9][8] = 1;
		  	  	  kunyah[10][6] = 1;
		  	  	  kunyah[10][7] = 1;
		  	  	  kunyah[10][8] = 1;
		  	  	  kunyah[11][7] = 1;
			  break;
			  case 1:
				  kunyah[9][2] = 1;
		  	  	  kunyah[9][3] = 1;
		  	  	  kunyah[9][4] = 1;
		  	  	  kunyah[9][5] = 1;
		  	  	  kunyah[10][3] = 1;
		  	  	  kunyah[10][4] = 1;
			  break;
			  case 2:
				  kunyah[9][9] = 1;
		  	  	  kunyah[9][10] = 1;
		  	  	  kunyah[9][11] = 1;
		  	  	  kunyah[9][12] = 1;
				  kunyah[10][9] = 1;
		  	  	  kunyah[10][10] = 1;
		  	  	  kunyah[10][11] = 1;
		  	  	  kunyah[10][12] = 1;
		  	  	  kunyah[11][10] = 1;
		  	  	  kunyah[11][11] = 1;
			  break;
			  case 3:
				  kunyah[10][5] = 1;
				  kunyah[11][4] = 1;
				  kunyah[11][5] = 1;
				  kunyah[11][6] = 1;
				  kunyah[12][6] = 1;
				  kunyah[12][7] = 1;
				  break;
			  case 4:
				  kunyah[11][8] = 1;
				  kunyah[11][9] = 1;
				  kunyah[12][8] = 1;
				  kunyah[12][9] = 1;
				  kunyah[12][10] = 1;
				  kunyah[13][9] = 1;
				  break;
			  case 5:
				  kunyah[9][1] = 1;
				  kunyah[10][1] = 1;
				  kunyah[10][2] = 1;
				  kunyah[11][2] = 1;
				  kunyah[11][3] = 1;
				  kunyah[12][2] = 1;
				  kunyah[12][3] = 1;
				  break;
			  case 6:
				  kunyah[12][4] = 1;
				  kunyah[12][5] = 1;
				  kunyah[13][3] = 1;
				  kunyah[13][4] = 1;
				  kunyah[13][5] = 1;
				  kunyah[13][6] = 1;
				  kunyah[14][5] = 1;
				  break;
			  case 7:
				  kunyah[9][13] = 1;
				  kunyah[10][13] = 1;
				  kunyah[11][12] = 1;
				  break;
			  case 8:
				  kunyah[12][11] = 1;
				  kunyah[12][12] = 1;
				  kunyah[13][10] = 1;
				  kunyah[13][11] = 1;
				  break;
			  case 9:
				  kunyah[13][7] = 1;
				  kunyah[13][8] = 1;
				  kunyah[14][6] = 1;
				  kunyah[14][7] = 1;
				  kunyah[14][8] = 1;
				  kunyah[14][9] = 1;
				  break;
			  }

			for (uint8_t y = 0; y < 16; y++)
			  for (uint8_t x = 0; x < 15; x++)
			  {
				  if (kunyah[y][x])
				  {
					  if (eat_mouth)
					  {
						  if ((x > 5 && x < 9) && (y > 8 && y < 14))
							  color_buffer[15-y][x] = ((food_count == _watermelon)?0xe9:0xe0);
					  }
					  else color_buffer[15-y][x] = 0x00;
				  }
				  else
				  {
					  if (y < 9)
					  {
						  if (circle_array_bit(senang[0], x, y)) color_buffer[15-y][x] = 0x2f;
						  else color_buffer[15-y][x] = 0x00;
					  }
					  else color_buffer[15-y][x] = circle_array(makanan[food_count], x, y);
				  }
				  if (eat_mouth)
				  {
					  if ((y == 7 && x == 7) || (y == 8 && (x > 5 && x < 9)))
						  color_buffer[15-y][x] = ((food_count == _watermelon)?0xe9:0xe0);
				  }
			  }
		}
	}
}

/********************************************************************************************/

void laugh_motion ()
{
	if (HAL_GetTick() - timer[2] >= 200)
	{
		timer[2] = HAL_GetTick();

		_Bool k = laugh_count % 2;
		for (uint8_t y = 0; y < 16; y++)
		  for (uint8_t x = 0; x < 15; x++)
		  {
			  if (circle_array_bit(tertawa[k], x, y))
			  {
				  if (y < 6) color_buffer[15-y][x] = 0x2f;
				  else
				  {
					  if (k)
					  {
						  if (y == 7) color_buffer[15-y][x] = 0xff;
						  else color_buffer[15-y][x] = 0xe0;
					  }
					  else
					  {
						  if (y == 9) color_buffer[15-y][x] = 0xff;
						  else color_buffer[15-y][x] = 0xe0;
					  }
				  }
			  }
			  else color_buffer[15-y][x] = 0x00;
		  }

		if (k)
		{
			buzzer_on ();
			buzzer (laugh_song[laugh_count/2]);
		}
		else
		{
			buzzer_off ();
		}

		laugh_count++;
		if (laugh_count > 20)
		{
			laugh_count = 0;
			buzzer_off ();
			face_emotion = _normal;
		}
	}
}

/********************************************************************************************/

void impressed_motion ()
{
	mpu9250_readAccelScale (&accel_raw[y], &accel_raw[x], &accel_raw[z]);
	update_eye_pos ();
	eye_motion ();
	switch (laugh_count)
	{
	case 0:
		color_buffer[4][7] = 0xe0;
		break;
	case 1:
		color_buffer[5][7] = 0xe0;
		break;
	case 2:
		color_buffer[5][7] = 0xe0;
		color_buffer[4][7] = 0xe0;
		break;
	default:
		color_buffer[5][7] = 0xe0;
		color_buffer[4][7] = 0xe0;
		color_buffer[3][7] = 0xe0;
		break;
	}
	if (HAL_GetTick() - timer[2] >= 200)
	{
		timer[2] = HAL_GetTick();
		laugh_count++;
		if (laugh_count > 10)
		{
			laugh_count = 0;
			face_emotion = _normal;
		}
	}
}

/********************************************************************************************/

void charg_plugin_motion ()
{
	if (HAL_GetTick() - timer[2] >= 500)
	{
		timer[2] = HAL_GetTick();

		for (uint8_t y = 0; y < 16; y++)
		  for (uint8_t x = 0; x < 15; x++)
		  {
			  if (circle_array_bit(plugin_face[plugin_motion_step], x, y))
			  {
				  if (x > 1 && x < 11 && y > 3 && y < 7) color_buffer[15-y][x] = 0x18;
				  else
				  {
					  if (y < 8) color_buffer[15-y][x] = 0xff;
					  else color_buffer[15-y][x] = 0xe0;
				  }

			  }
			  else color_buffer[15-y][x] = 0x00;
		  }

		plugin_motion_step++;
		if (plugin_motion_step > 4)
		{
			plugin_motion_step = 0;
			plugin_motion_count++;
			if (plugin_motion_count > 2)
			{
				plugin_motion_count = 0;
				face_emotion = _normal;
			}
		}
	}
	if (!charg)
	{
		face_emotion = _normal;
		plugin_motion_step = 0;
		plugin_motion_count = 0;
		timer[2] = HAL_GetTick();
	}
}

/********************************************************************************************/

void maze_game ()
{
	uint8_t mc;
	mpu9250_readAccelScale (&accel_raw[y], &accel_raw[x], &accel_raw[z]);
	float v = fabs (sqrt(accel_raw[x] * accel_raw[x] + accel_raw[y] * accel_raw[y])) * 60;
	if (v > 430) v = 430;
	if (HAL_GetTick() - timer[2] >= (500 - v))
	{
		timer[2] = HAL_GetTick();
		mc = maze_pos[x] + maze_pos[y]*15;
		if (accel_raw[x] > 2)
		{
			if ((maze[maze_mode][mc - 1] == 0) && (mc % 15 != 0)) maze_pos[x]--;
		}
		else if (accel_raw[x] < -2)
		{
			if ((maze[maze_mode][mc + 1] == 0) && (mc % 15 != 14)) maze_pos[x]++;
		}
		if (accel_raw[y] < -2)
		{
			if ((maze[maze_mode][mc - 15] == 0) && (mc / 15 != 0)) maze_pos[y]--;
		}
		else if (accel_raw[y] > 2)
		{
			if ((maze[maze_mode][mc + 15] == 0) && (mc / 15 != 15)) maze_pos[y]++;
		}
		if (maze[maze_mode][maze_pos[x] + maze_pos[y]*15])
		{
			maze_pos[x] = last_maze_pos[x];
			maze_pos[y] = last_maze_pos[y];
		}
		if (last_maze_pos[x] != maze_pos[x] || last_maze_pos[y] != maze_pos[y])
		{
			color_buffer[15-last_maze_pos[y]][last_maze_pos[x]] = 0x00;
			color_buffer[15-maze_pos[y]][maze_pos[x]] = 0x20;
			last_maze_pos[x] = maze_pos[x];
			last_maze_pos[y] = maze_pos[y];
		}
	}
	if (HAL_GetTick() - timer[1] >= 200)
	{
		timer[1] = HAL_GetTick();
		if (mf_blink) mf_blink = 0;
		else mf_blink = 1;
		color_buffer[15-maze_dest[y]][maze_dest[x]] = (uint8_t)mf_blink*0x18;
	}
	if (HAL_GetTick() - timer[3] >= 5000)
	{
		timer[3] = HAL_GetTick();
		color_buffer[15-maze_dest[y]][maze_dest[x]] = 0x00;
		color_buffer[15-maze_pos[y]][maze_pos[x]] = 0x20;
		maze_finish_dest = (uint8_t)((float)HAL_GetTick() / fabs(accel_raw[z])) % 4;
		switch (maze_finish_dest)
		{
		case 0: maze_dest[x] = 14; maze_dest[y] = 7; break;
		case 1: maze_dest[x] = 5; maze_dest[y] = 0;  break;
		case 2: maze_dest[x] = 5; maze_dest[y] = 15; break;
		case 3: maze_dest[x] = 8; maze_dest[y] = 15; break;
		}
	}
	if (maze_pos[x] == maze_dest[x] && maze_pos[y] == maze_dest[y]) face_emotion = _victory;
}

/********************************************************************************************/

void rng_snake_food ()
{
	snake_food = (uint32_t)((float)(HAL_GetTick() / fabs(accel_raw[x]+3)) * fabs(accel_raw[z]+4)) % 240;
	_Bool new_food = 1;
	while (new_food)
	{
		snake_food+=3;
		if (snake_food >= 235) snake_food = 5;
		if (snake_game_field[snake_food] == 0 && snake_food != snake_pos)
		{
			new_food = 0;
			for (uint8_t i = 0; i < snake_tail; i++)
				if (snake_food == snake_tail_pos[i]) new_food = 1;
		}
	}
}

/********************************************************************************************/

void snake_game ()
{
	uint16_t time = (400 - (snake_tail*4));
	if (time < 50) time = 50;
	if (HAL_GetTick() - timer[2] >= time)
	{
		timer[2] = HAL_GetTick();
		mpu9250_readAccelScale (&accel_raw[y], &accel_raw[x], &accel_raw[z]);
		if (last_snake_dir == up || last_snake_dir == down)
		{
			if (accel_raw[x] > 2) snake_dir = left;
			else if (accel_raw[x] < -2) snake_dir = right;
		}
		else if (last_snake_dir == right || last_snake_dir == left)
		{
			if (accel_raw[y] < -2) snake_dir = up;
			else if (accel_raw[y] > 2) snake_dir = down;
		}
		last_snake_dir = snake_dir;


		switch (snake_dir)
		{
		case up:
			if (snake_pos / 15 == 0) game_over_anm = 1;
			else snake_pos -= 15;
			break;
		case down:
			if (snake_pos / 15 == 15) game_over_anm = 1;
			else snake_pos += 15;
			break;
		case right:
			if (snake_pos % 15 == 14) game_over_anm = 1;
			else snake_pos ++;
			break;
		case left:
			if (snake_pos % 15 == 0) game_over_anm = 1;
			else snake_pos --;
			break;
		}

		color_buffer[15-(snake_pos/15)][snake_pos % 15] = 0x01;
		color_buffer[15-(snake_tail_pos[snake_tail]/15)][snake_tail_pos[snake_tail] % 15] = 0x00;

		for (int16_t i = snake_tail; i >= 0; i--)
		{
			if (snake_pos == snake_tail_pos[i]) game_over_anm = 1;
			if (i == 0) snake_tail_pos[0] = snake_pos;
			else snake_tail_pos[i] = snake_tail_pos[i-1];
		}

		if (snake_pos == snake_food)
		{
			snake_tail++;
			rng_snake_food ();
		}

		if (snake_game_field[snake_pos]) game_over_anm = 1;
		if (game_over_anm)
		{
			face_emotion = _game_over;
			timer[2] = HAL_GetTick();
		}
	}
	color_buffer[15-(snake_food/15)][snake_food % 15] =
			((HAL_GetTick() - timer[2] < (200 - snake_tail))?0x08:0x20);
}

/********************************************************************************************/

void enter_game ()
{
	buzzer_off ();
	mpu9250_readAccelScale (&accel_raw[y], &accel_raw[x], &accel_raw[z]);
	if (accel_raw[z] >= 9)
	{
		if (accel_raw[z] > fabs(accel_raw[x]))
		{
			for (uint8_t i = 0; i < MAX_SAND; i++)
			{
			  sand_pos_temp[i][x] = (i%10)+3;
			  sand_pos_temp[i][y] = 6 + i/10;
			  sand_pos_temp[i][x] = (i%10)+3;
			  sand_pos_temp[i][y] = 6 + i/10;
			  v_ball[i][x] = 0;
			  v_ball[i][y] = 0;
			}
			timer[2] = HAL_GetTick();
			face_emotion = _sand;
		}
	}
	if (accel_raw[x] >= 9)
	{
		if (enter_snake_g)
		{
			enter_snake_g = 0;
			face_emotion = _snake;
			snake_pos = 127;
			snake_tail = 0;
			rng_snake_food ();
			for (uint8_t y = 0; y < 16; y++)
			  for (uint8_t x = 0; x < 15; x++)
				  color_buffer[15-y][x] = 0x00;
			timer[2] = HAL_GetTick();
		}
		else
		{
			if (enter_maze == 0) enter_maze = 1;
		}
	}
	else if (accel_raw[x] <= -9)
	{
		if (enter_maze == 1)
		{
			enter_maze = 0;
			face_emotion = _maze;
			maze_pos[x] = 0;
			maze_pos[y] = 5;
			maze_mode = HAL_GetTick() % 2;
			maze_finish_dest = HAL_GetTick() % 4;
			for (uint8_t y = 0; y < 16; y++)
			  for (uint8_t x = 0; x < 15; x++)
			  {
				  if (maze[maze_mode][x+y*15]) color_buffer[15-y][x] = 0x001;
				  else color_buffer[15-y][x] = 0x00;
			  }
			timer[3] = HAL_GetTick() - 5000;
		}
		else
		{
			if (enter_snake_g == 0) enter_snake_g = 1;
		}
	}
}

/********************************************************************************************/

void sand_motion ()
{
	mpu9250_readAccelScale (&accel_raw[y], &accel_raw[x], &accel_raw[z]);
	for (uint8_t i = 0; i < MAX_SAND; i++)
	{
	  if (fabs(accel_raw[x]) >= 0.5) v_ball[i][x] += accel_raw[x]*0.02;
	  if (fabs(accel_raw[y]) >= 0.5) v_ball[i][y] += accel_raw[y]*0.02;
	  sand_pos_temp[i][x] -= v_ball[i][x]*0.5;
	  sand_pos_temp[i][y] += v_ball[i][y]*0.5;
	  //limit-------------------------------
	  if (sand_pos[i][y] > 4 && sand_pos[i][y] < 11)
	  {
		  if (sand_pos_temp[i][x] < 0)
		  {
			  sand_pos_temp[i][x] = 0;
			  v_ball[i][x] = 0;
		  }
		  if (sand_pos_temp[i][x] > 14)
		  {
			  sand_pos_temp[i][x] = 14;
			  v_ball[i][x] = 0;
		  }
	  }
	  else if (sand_pos[i][y] == 3 || sand_pos[i][y] == 4 || sand_pos[i][y] == 11 || sand_pos[i][y] == 12)
	  {
		  if (sand_pos_temp[i][x] < 1)
		  {
			  sand_pos_temp[i][x] = 1;
			  v_ball[i][x] = 0;
		  }
		  if (sand_pos_temp[i][x] > 13)
		  {
			  sand_pos_temp[i][x] = 13;
			  v_ball[i][x] = 0;
		  }
	  }
	  else if (sand_pos[i][y] == 2 || sand_pos[i][y] == 13)
	  {
		  if (sand_pos_temp[i][x] < 2)
		  {
			  sand_pos_temp[i][x] = 2;
			  v_ball[i][x] = 0;
		  }
		  if (sand_pos_temp[i][x] > 12)
		  {
			  sand_pos_temp[i][x] = 12;
			  v_ball[i][x] = 0;
		  }
	  }
	  else if (sand_pos[i][y] == 1 || sand_pos[i][y] == 14)
	  {
		  if (sand_pos_temp[i][x] < 3)
		  {
			  sand_pos_temp[i][x] = 3;
			  v_ball[i][x] = 0;
		  }
		  if (sand_pos_temp[i][x] > 11)
		  {
			  sand_pos_temp[i][x] = 11;
			  v_ball[i][x] = 0;
		  }
	  }
	  else if (sand_pos[i][y] == 0 || sand_pos[i][y] == 15)
	  {
		  if (sand_pos_temp[i][x] < 5)
		  {
			  sand_pos_temp[i][x] = 5;
			  v_ball[i][x] = 0;
		  }
		  if (sand_pos_temp[i][x] > 9)
		  {
			  sand_pos_temp[i][x] = 9;
			  v_ball[i][x] = 0;
		  }
	  }

	  if (sand_pos[i][x] > 4 && sand_pos[i][x] < 10)
	  {
		  if (sand_pos_temp[i][y] < 0)
		  {
			  sand_pos_temp[i][y] = 0;
			  v_ball[i][y] = 0;
		  }
		  if (sand_pos_temp[i][y] > 15)
		  {
			  sand_pos_temp[i][y] = 15;
			  v_ball[i][y] = 0;
		  }
	  }
	  else if (sand_pos[i][x] == 3 || sand_pos[i][x] == 4 || sand_pos[i][x] == 10 || sand_pos[i][x] == 11)
	  {
		  if (sand_pos_temp[i][y] < 1)
		  {
			  sand_pos_temp[i][y] = 1;
			  v_ball[i][y] = 0;
		  }
		  if (sand_pos_temp[i][y] > 14)
		  {
			  sand_pos_temp[i][y] = 14;
			  v_ball[i][y] = 0;
		  }
	  }
	  else if (sand_pos[i][x] == 2 || sand_pos[i][x] == 12)
	  {
		  if (sand_pos_temp[i][y] < 2)
		  {
			  sand_pos_temp[i][y] = 2;
			  v_ball[i][y] = 0;
		  }
		  if (sand_pos_temp[i][y] > 13)
		  {
			  sand_pos_temp[i][y] = 13;
			  v_ball[i][y] = 0;
		  }
	  }
	  else if (sand_pos[i][x] == 1 || sand_pos[i][x] == 13)
	  {
		  if (sand_pos_temp[i][y] < 3)
		  {
			  sand_pos_temp[i][y] = 3;
			  v_ball[i][y] = 0;
		  }
		  if (sand_pos_temp[i][y] > 12)
		  {
			  sand_pos_temp[i][y] = 12;
			  v_ball[i][y] = 0;
		  }
	  }
	  else if (sand_pos[i][x] == 0 || sand_pos[i][x] == 14)
	  {
		  if (sand_pos_temp[i][y] < 5)
		  {
			  sand_pos_temp[i][y] = 5;
			  v_ball[i][y] = 0;
		  }
		  if (sand_pos_temp[i][y] > 10)
		  {
			  sand_pos_temp[i][y] = 10;
			  v_ball[i][y] = 0;
		  }
	  }

	  sand_pos[i][x] = sand_pos_temp[i][x];
	  sand_pos[i][y] = sand_pos_temp[i][y];
	  for (uint8_t n = 0; n < MAX_SAND; n++)
	  {
		  if (n != i)
		  {
			  if ((sand_pos[i][x] == sand_pos[n][x]) && (sand_pos[i][y] == sand_pos[n][y]))
			  {
				  sand_pos[i][x] = last_sand_pos[i][x];
				  sand_pos_temp[i][x] = sand_pos[i][x];
				  v_ball[i][x] = 0;
				  sand_pos[i][y] = last_sand_pos[i][y];
				  sand_pos_temp[i][y] = sand_pos[i][y];
				  v_ball[i][y] = 0;
			  }
		  }
	  }
	  if (last_sand_pos[i][x] != sand_pos[i][x] || last_sand_pos[i][y] != sand_pos[i][y])
	  {
		  ball_shadow_temp[last_sand_pos[i][x]][last_sand_pos[i][y]] = 0;
		  uint8_t color;
		  if (i % 5 == 0) color = 0x07;
		  else if (i % 5 == 1) color = 0xe0;
		  else if (i % 5 == 2) color = 0x18;
		  else if (i % 5 == 3) color = 0xe7;
		  else if (i % 5 == 4) color = 0xf8;
		  ball_shadow_temp[sand_pos[i][x]][sand_pos[i][y]] = color;
		  last_sand_pos[i][x] = sand_pos[i][x];
		  last_sand_pos[i][y] = sand_pos[i][y];
	  }
	}
	for (uint8_t y = 0; y < 16; y++)
		for (uint8_t x = 0; x < 15; x++)
			color_buffer[15-y][x] = ball_shadow_temp[x][y];
	//back-----------------------------------
	if (accel_raw[x] >= 10)
	{
		if (!s_det[3])
		{
			s_det[3] = 1;
		}
	}
	else if (accel_raw[x] <= -10)
	{
		if (s_det[3])
		{
			s_det[3] = 0;
			timer[2] = HAL_GetTick();
			s_count[3]++;
			if (s_count[3] > 10)
			{
				s_count[3] = 0;
				face_emotion = _normal;
			}
		}
	}
	else
	{
		if (s_count[3] != 0)
			if (HAL_GetTick() - timer[2] > 1000)
				s_count[3] = 0;
	}
}

/********************************************************************************************/

void running_text (char *str, uint16_t ms)
{
	if (HAL_GetTick() - timer[2] >= ms)
	{
		timer[2] = HAL_GetTick();
		uint16_t n = 0, m = 0;
		_Bool space = 0, next = 0;
		for (uint8_t x = 0; x < 15; x++)
		{
			space = 0;
			for (uint8_t i = 4; i < 13; i++)
				font_buff[i][x] = 0;
			for (uint8_t y = 0; y < 7; y++)
			{
				char ch = str[text_r+n];
				_Bool c = font[(str[text_r + n]-32)*42 + y*6 + (((x + pixel_r) - m) % 6)];
				if (c) space = 1;
				if (ch == ' ') font_buff[y+4][x] = 0;
				else
				{
					if (ch == 'g' || ch == 'j' || ch == 'p' || ch == 'q' || ch == 'y') font_buff[y+6][x] = c;
					else font_buff[y+4][x] = c;
				}
			}
			if (space == 0)
			{
				n++;
				m = x + pixel_r + 1;
				if (x == 0) next = 1;
			}
		}
		pixel_r++;
		if (next)
		{
			text_r++;
			pixel_r = 0;
			if (str[text_r+5] == 0)
			{
				text_r = 0;
				end_text = 1;
			}
		}

		for (uint8_t y = 0; y < 16; y++)
		  for (uint8_t x = 0; x < 15; x++)
		  {
			  if (font_buff[y][x]) color_buffer[15-y][x] = 0x001;
			  else color_buffer[15-y][x] = 0x00;
		  }
	}
}

/********************************************************************************************/

void write_running_text (char *str)
{
	uint16_t n = 0;
	for (uint16_t i = 0; str[i] != 0; i++)
	{
		text_buffer[i+10] = str[i];
		n++;
	}
	for (uint16_t i = 0; i < 10; i++)
	{
		text_buffer[i] = ' ';
		text_buffer[i+n+10] = ' ';
	}
	text_buffer[n+20] = 0;
}

/********************************************************************************************/

void victory_game ()
{
	if (HAL_GetTick() - timer[2] >= 400)
	{
		timer[2] = HAL_GetTick();
		if (!game_over_anm)
		{
			game_over_anm = 1;
			for (uint8_t y = 0; y < 16; y++)
			  for (uint8_t x = 0; x < 15; x++)
			  {
				  if (circle_array_bit(you_win, x, y))
				  {
					  if (y > 7) color_buffer[15-y][x] = 0x22;
					  else color_buffer[15-y][x] = 0xe8;
				  }
				  else color_buffer[15-y][x] = 0x00;
			  }
		}
		else
		{
			game_over_anm = 0;
			game_over_count++;
			if (game_over_count > 4)
			{
				game_over_count = 0;
				face_emotion = _normal;
			}
			for (uint8_t y = 0; y < 16; y++)
			  for (uint8_t x = 0; x < 15; x++)
				  color_buffer[15-y][x] = 0;
		}
	}
}

/********************************************************************************************/

void game_over_dis ()
{
	if (snake_game_over == 0)
	{
		if (HAL_GetTick() - timer[2] >= 300)
		{
			timer[2] = HAL_GetTick();
			if (!game_over_anm)
			{
				game_over_anm = 1;
				for (uint8_t y = 0; y < 16; y++)
				  for (uint8_t x = 0; x < 15; x++)
				  {
					  if (circle_array_bit(game_over, x, y))  color_buffer[15-y][x] = 0x20;
					  else  color_buffer[15-y][x] = 0;
				  }
			}
			else
			{
				game_over_anm = 0;
				for (uint8_t y = 0; y < 16; y++)
				  for (uint8_t x = 0; x < 15; x++)
					  color_buffer[15-y][x] = 0;

				game_over_count++;
				if (game_over_count > 2)
				{
					game_over_count = 0;
					snake_game_over = 1;
					sprintf (text_buffer,"     GAME OVER. SCORE:%03d     ", snake_tail);
				}
			}
		}
	}
	else
	{
		running_text (text_buffer, 50);
		if (end_text)
		{
			end_text = 0;
			snake_game_over = 0;
			face_emotion = _normal;
		}
	}
}

/********************************************************************************************/

void cat_animation ()
{
	if (HAL_GetTick() - timer[7] >= t_song_intro)
	{
		timer[7] = HAL_GetTick();
		if (pat_count < 60)
		{
			if (!bz_n)
			{
				bz_n = 1;
				n_nada++;
				if (n_nada > 7) n_nada = 0;
				buzzer_on ();
				buzzer (song_intro[n_nada]);
			}
			else
			{
				bz_n = 0;
				buzzer_off ();
			}
		}
		else
		{
			if (!nada_lompat)
			{
				buzzer_off ();
			}
			else
			{
				t_song_intro = 50;
				buzzer (song_intro[n_nada]);
				n_nada++;
				if (n_nada > 15)
				{
					n_nada = 0;
					t_song_intro = 200;
					bz_n = 0;
					buzzer_off ();
				}
			}
		}
	}

	if (HAL_GetTick() - timer[2] >= delay_anm)
	{
		timer[2] = HAL_GetTick();
		uint8_t k = pat_count%6;
		if (pat_count < 60)
		{
			delay_anm = 200;
			for (uint8_t y = 0; y < 16; y++)
			  for (uint8_t x = 0; x < 15; x++)
			  {
				  if (circle_array_bit(kucing[k], x, y)) color_buffer[15-y][x] = 0xe8;
				  else  color_buffer[15-y][x] = 0;
			  }
			color_buffer[10-(pat_count%2)][11] = 0x2f;
			color_buffer[9-(pat_count%2)][12] = 0x28;
			color_buffer[6][5] = 0xea;
			color_buffer[6][6] = 0xea;
			color_buffer[6][7] = 0xea;
			/*
			switch (k)
			{
			case 0:
				color_buffer[3][3] = 0x48;
				color_buffer[4][3] = 0x48;
				color_buffer[5][3] = 0x48;
				color_buffer[4][10] = 0x48;
				color_buffer[5][9] = 0x48;
				color_buffer[5][10] = 0x48;
				break;
			}
			*/
		}
		else
		{
			for (uint8_t y = 0; y < 16; y++)
			  for (uint8_t x = 0; x < 15; x++)
			  {
				  if (circle_array_bit(kucing_lompat[k], x, y)) color_buffer[15-y][x] = 0xe8;
				  else  color_buffer[15-y][x] = 0;
			  }
			if (k == 0)
			{
				delay_anm = 2000;
				color_buffer[10][8] = 0x2f;
				color_buffer[10][10] = 0x2f;
				color_buffer[9][9] = 0x28;
			}
			else if (k == 1)
			{
				delay_anm = 100;
				color_buffer[8][11] = 0x2f;
				color_buffer[7][12] = 0x28;
				buzzer_on ();
				nada_lompat = 1;
				n_nada = 8;
			}
			else if (k == 2)
			{
				color_buffer[10][12] = 0x2f;
				color_buffer[9][13] = 0x48;
			}
		}
		pat_count++;
		if (pat_count > 64)
		{
			pat_count = 0;
			t_song_intro = 200;
			face_emotion = _normal;
			buzzer_off ();
		}
	}
}

/********************************************************************************************/

void wakeup ()
{
	mpu9250_readAccelScale (&accel_raw[y], &accel_raw[x], &accel_raw[z]);
	if (accel_raw[z] > 4)
	{
	  timer[1] = HAL_GetTick();
	  eye[x] = 3;//
	  eye[y] = 3;
	  wm_count = 0;
	  face_emotion = _wake_up;
	  dir_wm_motion = 0;
	  time_wm_motion = 20;
	  timer[0] = HAL_GetTick();
	  timer[5] = HAL_GetTick();
	  timer[6] = HAL_GetTick();
	}
}

/********************************************************************************************/

void motion_detect ()
{
	  mpu9250_readAccelScale (&accel_raw[y], &accel_raw[x], &accel_raw[z]);
	  mpu9250_readGyroScale (&gyro_raw[y], &gyro_raw[x], &gyro_raw[z]);

	  //accel detect-----------------------------------------------------
	  if (accel_raw[x] > 10)
	  {
		  if (!s_det[0]) s_det[0] = 1;
	  }
	  else if (accel_raw[x] < -10)
	  {
		  if (s_det[0])
		  {
			  s_det[0] = 0;
			  s_count[0]++;
			  if (s_count[0] > 10)
			  {
				  s_count[0] = 0;
				  s_count[1] = 0;
				  s_count[2] = 0;
				  s_count[3] = 0;
				  timer[2] = HAL_GetTick();
				  buzzer_off ();
				  face_emotion = _angry;
			  }
		  }
	  }
	  if (accel_raw[x] < 8 && accel_raw[x] > 6)
	  {
		  if (!s_det[1]) s_det[1] = 1;
	  }
	  else if (accel_raw[x] > -8 && accel_raw[x] < -6)
	  {
		  if (s_det[1])
		  {
			  s_det[1] = 0;
			  s_count[1]++;
			  if (s_count[1] > 5)
			  {
				  s_count[0] = 0;
				  s_count[1] = 0;
				  s_count[2] = 0;
				  s_count[3] = 0;
				  timer[2] = HAL_GetTick();
				  buzzer_off ();
				  face_emotion = _happy;
			  }
		  }
	  }
	  if (accel_raw[y] > 5)
	  {
		  if (!s_det[2]) s_det[2] = 1;
	  }
	  else if (accel_raw[y] < -5)
	  {
		  if (s_det[2])
		  {
			  s_det[2] = 0;
			  s_count[2]++;
			  if (s_count[2] > 10)
			  {
				  s_count[0] = 0;
				  s_count[1] = 0;
				  s_count[2] = 0;
				  s_count[3] = 0;
				  buzzer_off ();
				  face_emotion = _wake_up;
			  }
		  }
	  }

	  if (fabs(accel_raw[x]) < 3 && fabs(accel_raw[y]) < 3)
	  {
		  if (gyro_raw[z] > 220 && accel_raw[z] < 0)//baru
		  {
			  if (HAL_GetTick() - timer[8] > 300)
			  {
				  emo_select_count = 0;
				  buzzer_off ();
				  timer[8] = HAL_GetTick();
				  face_emotion = _emotic_select;
				  emotic (emo_select_count);
			  }
		  }
		  else
		  {
			  timer[8] = HAL_GetTick();
			  if (gyro_raw[z] > 100)
			  {
				  if (!s_det[3]) s_det[3] = 1;
			  }
			  else if (gyro_raw[z] < -100)
			  {
				  if (s_det[3])
				  {
					  s_det[3] = 0;
					  s_count[3]++;
					  if (s_count[3] > 4)
					  {
						  s_count[0] = 0;
						  s_count[1] = 0;
						  s_count[2] = 0;
						  s_count[3] = 0;
						  face_emotion = _laugh;
						  buzzer_off ();
						  timer[2] = HAL_GetTick() - 200;
					  }
				  }
			  }
		  }

	  }
}

void greeting_text ()
{
	running_text ("     Halo... namaku emotic     ", 31);
	if (end_text)
	{
		end_text = 0;
		face_emotion = _normal;
	}
}

/********************************************************************************************/

void battery_management ()
{
	read_battery();
	if (!start_bat_read)
	{
		if (HAL_GetTick() - timer[5] >= 2000) start_bat_read = 1;
	}
	else
	{
		if (v_bat <= 3.8) face_emotion = _low_bat;
		if (last_charg != charg)
		{
			last_charg = charg;
			if (charg) face_emotion = _charg;
		}
	}
}

/********************************************************************************************/

void emotic_select_motion ()
{
	  mpu9250_readAccelScale (&accel_raw[y], &accel_raw[x], &accel_raw[z]);
	  mpu9250_readGyroScale (&gyro_raw[y], &gyro_raw[x], &gyro_raw[z]);
	  if (accel_raw[x] >= 9)
	  {
		  if (!emo_s_state) emo_s_state = 1;
	  }
	  else if (accel_raw[x] <= -9)
	  {
		  if (emo_s_state)
		  {
			  emo_s_state = 0;
			  emo_select_count++;
			  if (emo_select_count > _emotic_hampa) emo_select_count = _emotic_senang;
			  emotic (emo_select_count);
		  }
	  }
	  if (fabs(accel_raw[x]) < 3 && fabs(accel_raw[y]) < 3)
	  {
		  if (gyro_raw[z] < -220)
		  {
			  if (HAL_GetTick() - timer[8] > 300)
			  {
				  emo_select_count = 0;
				  buzzer_off ();
				  timer[8] = HAL_GetTick();
				  face_emotion = _normal;
			  }
		  }
		  else timer[8] = HAL_GetTick();
	  }
}

/********************************************************************************************/

void animation ()
{
	battery_management ();

	  switch (face_emotion)
	  {
	  case _greeting:
		  greeting_text ();
		  break;
	  case _normal: //========================================================================================
			motion_detect ();
			update_eye_pos ();
			eye_motion ();
			if (HAL_GetTick() - timer[1] >= 8000)
			{
			  face_emotion = _sleep;
			  timer[1] = HAL_GetTick();
			  timer[6] = HAL_GetTick();
			}
			if (sleepy) sleepy = 0;
			if (HAL_GetTick() - timer[5] >= 120000) //120000
			{
				face_emotion = _hungry;
				timer[2] = HAL_GetTick();
				timer[6] = HAL_GetTick();
			}
			if (HAL_GetTick() - timer[6] >= 5000)
			{
				timer[6] = HAL_GetTick();
				timer[2] = HAL_GetTick();
				face_emotion = _impressed;
			}
		  break;
	  case _emotic_select:
		  emotic_select_motion ();
		  break;
	  case _sleep: //=========================================================================================
		  if (sleepy == 0)
		  {
			  if (HAL_GetTick() - timer[7] >= 100)
			  {
				  timer[7] = HAL_GetTick();
				  if (n_nada < 10)
				  {
					  buzzer_on ();
					  buzzer (sleepy_song[n_nada]);
					  n_nada++;
				  }
			  }
			  if (HAL_GetTick() - timer[1] >= 3000)
			  {
				  timer[1] = HAL_GetTick();
				  n_nada = 0;
				  buzzer_off ();
				  sleepy = 1;
			  }
			  send_to_buffer (ngantuk);
		  }
		  else sleep_motion ();
		  wakeup ();
		  break;
	  case _angry: //=========================================================================================
		  if (HAL_GetTick() - timer[2] >= 5000) face_emotion = _normal;
		  send_to_buffer (kesel);
		  if (HAL_GetTick() - timer[2] >= 2000)
			  enter_game ();
		  break;
	  case _happy: //=========================================================================================
		  if (HAL_GetTick() - timer[2] >= 4000)
		  {
			  timer[2] = HAL_GetTick();
			  face_emotion = _normal;
		  }
		  else
		  {
			  if (HAL_GetTick() - timer[2] > 1000)
			  {
					mpu9250_readAccelScale (&accel_raw[y], &accel_raw[x], &accel_raw[z]);
					if (accel_raw[z] >= 9) face_emotion = _pat_cat;
			  }
		  }
		  for (uint8_t y = 0; y < 16; y++)
			  for (uint8_t x = 0; x < 15; x++)
			  {
				  if (circle_array_bit(senang[((HAL_GetTick() - timer[2] < 2000)? 0: 1)], x, y))
				  {
					  if (y < 6) color_buffer[15-y][x] = 0x2f;
					  else color_buffer[15-y][x] = 0xe0;
				  }
				  else color_buffer[15-y][x] = 0x00;
			  }
		  break;
	  case _laugh:
		  laugh_motion();
		  break;
	  case _impressed:
		  impressed_motion();
		  break;
	  case _wake_up: //=======================================================================================
		  wakeup_motion ();
		  break;
	  case _low_bat: //=======================================================================================
		  buzzer_off ();
		  send_to_buffer (lelah);
		  if (v_bat >= 4.0 || charg) face_emotion = _normal;
		  break;
	  case _charg: //=========================================================================================
		  charg_plugin_motion ();
		  break;
	  case _hungry: //========================================================================================
		  if (HAL_GetTick() - timer[2] >= 200)
		  {
			  timer[2] = HAL_GetTick();
			  buzzer_off ();
			  wm_count++;
			  if (wm_count > 1) wm_count = 0;
			  for (uint8_t y = 0; y < 16; y++)
				  for (uint8_t x = 0; x < 15; x++)
				  {
					  if (circle_array_bit(lapar[wm_count%2], x, y))
					  {
						  if (y < 9) color_buffer[15-y][x] = 0x2f;
						  else color_buffer[15-y][x] = 0xe0;
					  }
					  else color_buffer[15-y][x] = 0x00;
				  }
		  }
		  mpu9250_readAccelScale (&accel_raw[y], &accel_raw[x], &accel_raw[z]);
		  if (accel_raw[x] > 10)
		  {
			  if (!food_s) food_s = 1;
		  }
		  else if (accel_raw[x] < -10)
		  {
			  if (food_s)
			  {
				  food_s = 0;
				  send_to_buffer (makanan[food_count]);
				  face_emotion = _food_select;
			  }
		  }
		  break;
	  case _food_select: //========================================================================================
		  food_menu ();
		  break;
	  case _eat_time: //========================================================================================
		  eat_motion();
		  break;
	  case _happy_aft_eat: //========================================================================================
		  if (HAL_GetTick() - timer[2] >= 2000)
		  {
			  timer[2] = HAL_GetTick();
			  buzzer_off ();
			  face_emotion = _normal;
		  }
		  if (n_nada < 6)
			  if (HAL_GetTick() - timer[7] >= 200)
			  {
				  timer[7] = HAL_GetTick();
				  buzzer_on();
				  buzzer (h_eat_song[n_nada++]);
			  }
		  for (uint8_t y = 0; y < 16; y++)
			  for (uint8_t x = 0; x < 15; x++)
			  {
				  if (circle_array_bit(senang[2], x, y))
				  {
					  if (y < 6) color_buffer[15-y][x] = 0x2f;
					  else if (y == 8) color_buffer[15-y][x] = 0xff;
					  else color_buffer[15-y][x] = 0xe0;
				  }
				  else color_buffer[15-y][x] = 0x00;
			  }
		  break;
	  case _maze: //========================================================================================
		  maze_game ();
		  break;
	  case _snake: //========================================================================================
		  snake_game ();
		  break;
	  case _sand: //========================================================================================
		  sand_motion ();
		  break;
	  case _victory: //========================================================================================
		  victory_game ();
		  break;
	  case _game_over:  //========================================================================================
		  game_over_dis ();
		  break;
	  case _pat_cat:
		  cat_animation ();
		  break;
	  }
}

/********************************************************************************************/

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == CHARG_Pin)
	{
		if (!HAL_GPIO_ReadPin(CHARG_GPIO_Port, CHARG_Pin)) charg = 1;
		else charg = 0;
	}
}

/********************************************************************************************/

/*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

}
*/

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
	HAL_Delay(100);
	MPU_ConfigTypeDef myMpuConfig;
	mpu9250_init (&hi2c1);
	myMpuConfig.accelFullScale = AFS_SEL_2g;
	myMpuConfig.clockSource = Internal_8MHz;
	myMpuConfig.CONFIG_DLPF = DLPF_10_Hz;
	myMpuConfig.gyroFullScale = FS_SEL_250;
	myMpuConfig.sleepModeBit = 0;  //1: sleep mode, 0: normal mode
	mpu9250_config (&myMpuConfig);

	send_to_buffer (emotion);

	GPIOA->ODR |= 0x07fc;
	GPIOB->ODR |= 0xf000;
	GPIOB->ODR |= 0x0038;
	for (uint8_t i = 0; i < TIMER_USED; i++)
	  timer[i] = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (HAL_GetTick() - timer[4] > 10)
	  {
		  timer[4] = HAL_GetTick();
		  read_battery ();
		  animation ();
	  }
//	  led_test ();
	  manual_scan ();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 31;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 9999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OE_GPIO_Port, OE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, C0_Pin|C1_Pin|C2_Pin|C3_Pin
                          |C4_Pin|C5_Pin|C6_Pin|C7_Pin
                          |C8_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, DATA_R_Pin|DATA_G_Pin|DATA_B_Pin|LOAD_Pin
                          |CLK_Pin|C12_Pin|C13_Pin|C14_Pin
                          |C15_Pin|C9_Pin|C10_Pin|C11_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : OE_Pin */
  GPIO_InitStruct.Pin = OE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CHARG_Pin */
  GPIO_InitStruct.Pin = CHARG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(CHARG_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : C0_Pin C1_Pin C2_Pin C3_Pin
                           C4_Pin C5_Pin C6_Pin C7_Pin
                           C8_Pin */
  GPIO_InitStruct.Pin = C0_Pin|C1_Pin|C2_Pin|C3_Pin
                          |C4_Pin|C5_Pin|C6_Pin|C7_Pin
                          |C8_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : DATA_R_Pin DATA_G_Pin DATA_B_Pin LOAD_Pin
                           CLK_Pin */
  GPIO_InitStruct.Pin = DATA_R_Pin|DATA_G_Pin|DATA_B_Pin|LOAD_Pin
                          |CLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : C12_Pin C13_Pin C14_Pin C15_Pin
                           C9_Pin C10_Pin C11_Pin */
  GPIO_InitStruct.Pin = C12_Pin|C13_Pin|C14_Pin|C15_Pin
                          |C9_Pin|C10_Pin|C11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
