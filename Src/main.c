/**********************************************************************************************

A sample demo that shows flappy bird clone.


The drawing of the sprites and backgrounds are rendered into a 2D layer using the 2D DMA hardware acceleration.
Rendering the sprites, however are  written using direct memory access to the frame buffer without a hardware DMA.
Rendering the background images, are written using the DMA hardware.

The source also contains a 3D Flat Shading Engine, that draws a 3D Cube with some lighting information.

Last Modified 11.6.2018
Started 04.2014

Authors:

Islam Gamal  2D Graphics Programming in the engine
Ahmed Tolba  3D Graphics Programming in the engine 

***********************************************************************************************/


#include <stdint.h>

#include "main.h"


#include "Graphics.h"
#include "Sprites.h"
#include <stdlib.h>
 
#define BUFFER          LCD_BACKGROUND_LAYER
#define SCREEN          LCD_FOREGROUND_LAYER
#define SCREEN_WIDTH     240
#define SCREEN_HEIGHT    320
#define BPP 						 2
#define FRAMEBUFFER_SIZE                              (SCREEN_WIDTH * SCREEN_HEIGHT * BPP)
#define TFTW             240     // screen width
#define TFTH             320     // screen height
#define TFTW2            120     // half screen width
#define TFTH2            160     // half screen height
// game constant
#define PIPE_SPEED        1
#define GRAVITY         9.8
#define JUMP_FORCE     1.15
#define SKIP_TICKS     20.0     // 1000 / 50fps
#define MAX_FRAMESKIP     5
// bird size
#define BIRDW             16     // bird width
#define BIRDH             16     // bird height
#define BIRDW2            8     // half width
#define BIRDH2            8     // half height
// pipe size
#define PIPEW            67     // pipe width
#define  PIPEH            145
#define GAPHEIGHT        36     // pipe gap height
#define PIPE1_MINY        176
#define PIPE1_MAXY        248
#define PIPE2_MINY        -72
#define PIPE2_MAXY         0
#define GAP_MIN            32
#define GAP_MAX           176
// floor size
#define FLOORH           20     // floor height (from bottom of the screen)
// grass size
#define GRASSH            4     // grass height (inside floor, starts at floor y)
 
#define GAMEH           (TFTH - FLOORH)

//Macros
#define RANGE_RAND(min , max) (rand()%( max+1 - min) +min)

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
void LCD_Config(void);
 
enum framebuffer
{
  FRAMEBUFFER1,FRAMEBUFFER2
};

static enum framebuffer active = FRAMEBUFFER1;
void LTDC_switch_framebuffer(void);
volatile uint8_t *LTDC_get_backbuffer_address(void);

struct bird
{
	sprite spr ; 	
} bird;

// pipe structure
 struct pipe 
{
	sprite spr ;
	int  gap_y;
	unsigned int col;
} pipe1 , pipe2;

	int flip = 1;

int gap ;
BITMAP back_bitmap = {.data = back_img ,  .w = 240 , .h = 320 }	;


 
typedef struct {
	uint32_t addr[2];
	uint32_t width;
	uint32_t height;
	uint32_t front;
} Screen;

Screen* ct_screen_init();
 
Screen* ct_screen_init() {
	BSP_LCD_Init();
	Screen *screen = (Screen*) malloc(sizeof(Screen));
	screen->width = BSP_LCD_GetXSize();
	screen->height = BSP_LCD_GetYSize();
	screen->addr[0] = LCD_FRAME_BUFFER;
	screen->addr[1] = LCD_FRAME_BUFFER + screen->width * screen->height * 2;
	screen->front = 0;
	BSP_LCD_LayerDefaultInit(0, screen->addr[0]);
	BSP_LCD_LayerDefaultInit(1, screen->addr[1]);
	BSP_LCD_SetLayerVisible(0, ENABLE);
	BSP_LCD_SetLayerVisible(1, DISABLE);
	BSP_LCD_SelectLayer(0);
	return screen;
}
 

uint8_t* ct_screen_backbuffer_ptr(Screen *screen) {

 	return (uint8_t*)screen->addr[!active];
}
static Screen *screen;
 
 void init_bird()
 {
	  bird.spr.x = 20;
    bird.spr.y = bird.spr.old_y = TFTH2 - BIRDH;
    bird.spr.yspeed = -JUMP_FORCE;
	  bird.spr.yaccel = GRAVITY * 0.01 ;
	  bird.spr.xspeed = 0 ;
		bird.spr.alive = 1 ;
	  bird.spr.width = 16 ;
	  bird.spr.height = 16 ;
	  bird.spr.num_frames = 4 ;
	  bird.spr.curr_frame = 0 ;
	  bird.spr.vydelay = 0 ;
    bird.spr.ydelay = 0 ; 
	 // init anim 
	 bird.spr.anim_clock = 0 ; 
	 bird.spr.anim_speed = 5 ;
	 
	 
		BITMAP bird_bitmap = {.data = bird_img , .w = 64 , .h = 16 } ;
	 // Grape bird frames
	 for(int i = 0 ; i< 4 ; i++)
       Grape_frame(&bird.spr , &bird_bitmap , 4 ,  i ) ;
 }
	
 
 void init_pipes()
 {
	  
    pipe1.spr.x = TFTW - PIPEW  ;
    pipe1.spr.y = RANGE_RAND(PIPE1_MINY , PIPE1_MAXY) ;
    pipe1.spr.xspeed = -PIPE_SPEED ;
    pipe1.spr.yspeed = -1 ;
		pipe1.spr.num_frames = 1 ;
	  pipe1.spr.width = PIPEW  ;
	  pipe1.spr.height = PIPEH  ;
    pipe1.spr.imgData = pipe1_img ;

    pipe2.spr.x = TFTW -PIPEW  ;
    pipe2.spr.y = RANGE_RAND(PIPE2_MINY , PIPE2_MAXY) ;//TFTH - pipe2.gap_y ;
    pipe2.spr.xspeed = -PIPE_SPEED ;
    pipe2.spr.yspeed = -1 ;
		pipe2.spr.num_frames = 1 ;
	  pipe2.spr.width = PIPEW  ;
	  pipe2.spr.height = PIPEH ;
    pipe2.spr.imgData = pipe2_img ;

    gap = RANGE_RAND(GAP_MIN , GAP_MAX) ;
 }
 
 void process_input()
 {
	 static int isJumping =0 ;
	  if ( BSP_PB_GetState (BUTTON_KEY) != SET && isJumping)
		 { 
			   isJumping  = 0;
		 }
		 else if(BSP_PB_GetState (BUTTON_KEY) == SET && !isJumping)
		 {
			  if (bird.spr.y > BIRDH2*0.5) bird.spr.yspeed = -JUMP_FORCE;
        // else zero velocity
        else bird.spr.yspeed = 0;
			 	isJumping = 1;

		 }
	  
 }
 
int main(void)
 {  
	/* STM32F4xx HAL library initialization:
			 - Configure the Flash prefetch, instruction and Data caches
			 - Configure the Systick to generate an interrupt each 1 msec
			 - Set NVIC Group Priority to 4
			 - Global MSP (MCU Support Package) initialization
		 */
	HAL_Init();
	
	/* Configure the system clock to 168 MHz */
	SystemClock_Config();

	/* Configure LED3 and LED4 */
	BSP_LED_Init(LED3); 
	BSP_LED_Init(LED4); 
	
	/* Configure USER Button */
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);
	
	/* Initialize LCD driver */
	LCD_Config() ;
	 
	
  init_bird() ;
	init_pipes() ;
  
	/* Infinite loop */
 while (1) 
	{
	
				process_input() ;		 
				Update_postion(&bird.spr) ;
				Update_animation(&bird.spr)	;	 
				
				//upadte pipes
   
		    pipe1.spr.x += pipe1.spr.xspeed ;
        pipe1.spr.y += pipe1.spr.yspeed ;
		    pipe1.spr.height = SCREEN_HEIGHT - pipe1.spr.y ;
        pipe2.spr.x += pipe2.spr.xspeed ;
        pipe2.spr.y += pipe2.spr.yspeed ;
		    //pipe2.spr.height = PIPEH + pipe2.spr.y ;
        if(pipe1.spr.x < -pipe1.spr.width || pipe2.spr.x < -pipe2.spr.width )
        {
           pipe1.spr.x = pipe2.spr.x = TFTW - pipe1.spr.width  ;
           gap = RANGE_RAND(GAP_MIN , GAP_MAX) ;
           pipe1.spr.y = RANGE_RAND(PIPE1_MINY , PIPE1_MAXY) ;
           pipe2.spr.y = RANGE_RAND(PIPE2_MINY , PIPE2_MAXY) ;
        }
        if(pipe1.spr.y < PIPE1_MINY || pipe1.spr.y >PIPE1_MAXY  )
        {
            pipe1.spr.yspeed *= -1 ;
        }
        if(pipe2.spr.y < PIPE2_MINY || pipe2.spr.y >PIPE2_MAXY  )
        {
            pipe2.spr.yspeed *= -1 ;
        }
				if(collision(&bird.spr , &pipe1.spr) || collision(&bird.spr , &pipe2.spr))
				{
								return 1;  // for now just stop the rendering, more on this later.
				}
				
						 
				uint8_t *fb = ct_screen_backbuffer_ptr(screen); // get the the buffer pointer to which you draw into it.
						
				BSP_LCD_SelectLayer(!active); // select the non active layer, draw into it
						
						
				BSP_LCD_DrawBitmap(!active, 0,0, (uint8_t*)back_bitmap.data);
				//blit(&back_bitmap ,fb, 0 , 0 , 0 , 0 ,SCREEN_WIDTH  , SCREEN_HEIGHT);
				DrawSprite(&pipe1.spr, fb );
				DrawSprite(&pipe2.spr, fb );
				DrawSprite(&bird.spr, fb );
					
				LTDC_switch_framebuffer(); // page flipping
 				


	}
}
 
void LTDC_switch_framebuffer(void)
{
	
	if (active == FRAMEBUFFER1)
  {
  	LTDC_Layer1->CFBAR = LCD_FRAME_BUFFER + BUFFER_OFFSET;
		active = FRAMEBUFFER2;

  }
  else
  {
		LTDC_Layer1->CFBAR = LCD_FRAME_BUFFER;
		active = FRAMEBUFFER1;
  }
  LTDC->SRCR = LTDC_SRCR_VBR;                     // reload shadow registers on vertical blank
  while ((LTDC->CDSR & LTDC_CDSR_VSYNCS) == 0)    // wait for reload
	;
 	
}
 

/**
	* @brief  System Clock Configuration
	*         The system Clock is configured as follow : 
	*            System Clock source            = PLL (HSE)
	*            SYSCLK(Hz)                     = 180000000
	*            HCLK(Hz)                       = 180000000
	*            AHB Prescaler                  = 1
	*            APB1 Prescaler                 = 4
	*            APB2 Prescaler                 = 2
	*            HSE Frequency(Hz)              = 8000000
	*            PLL_M                          = 8
	*            PLL_N                          = 360
	*            PLL_P                          = 2
	*            PLL_Q                          = 7
	*            VDD(V)                         = 3.3
	*            Main regulator output voltage  = Scale1 mode
	*            Flash Latency(WS)              = 5
	* @param  None
	* @retval None
	*/
static void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();
	
	/* The voltage scaling allows optimizing the power consumption when the device is 
		 clocked below the maximum system frequency, to update the voltage scaling value 
		 regarding system frequency refer to product datasheet.  */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	
	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 360;
	RCC_OscInitStruct.PLL.PLLP = 2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	/* Activate the Over-Drive mode */
	HAL_PWREx_EnableOverDrive();
		
	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
		 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

 
void LCD_Config(void)
{
	/* LCD Initialization */ 
	BSP_LCD_Init();
//	
//		/* fill framebuffers with black */
	for (int i = 0 ; i < SCREEN_WIDTH * SCREEN_HEIGHT * 2 ; i++)
		((volatile uint8_t*)LCD_FRAME_BUFFER)[i] = 0x00;
	for (int i = 0 ; i < SCREEN_WIDTH * SCREEN_HEIGHT * 2 ; i++)
			((volatile uint8_t*)LCD_FRAME_BUFFER + BUFFER_OFFSET)[i] = 0x00;
  
	screen = ct_screen_init();
	BSP_LCD_SetLayerVisible(0, ENABLE);
 

}


void Toggle_Leds(void)
{
	static uint8_t ticks = 0;
	
	if(ticks++ > 100)
	{
		BSP_LED_Toggle(LED3);
		BSP_LED_Toggle(LED4);
		ticks = 0;
	}
}

/**
	* @brief  EXTI line detection callbacks.
	* @param  GPIO_Pin: Specifies the pins connected EXTI line
	* @retval None
	*
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
 if (GPIO_Pin == KEY_BUTTON_PIN)
 {
	 ubKeyPressed = SET;
 }
}*/

#ifdef  USE_FULL_ASSERT
/**
	* @brief  Reports the name of the source file and the source line number
	*         where the assert_param error has occurred.
	* @param  file: pointer to the source file name
	* @param  line: assert_param error line source number
	* @retval None
	*/
void assert_failed(uint8_t* file, uint32_t line)
{ 
	/* User can add his own implementation to report the file name and line number,
		 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif

/**
	* @}
	*/ 

/**
	* @}
	*/
	
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
