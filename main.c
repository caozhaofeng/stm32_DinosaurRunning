
/*by逐辰十七* 2025-9-8 恐龙快跑
工程模版来自铁头山羊
参考学习视频:https://www.bilibili.com/video/BV17ojzzvEe3/?spm_id_from=333.1387.favlist.content.click&vd_source=c4ea31ebc7600abbfadb0a4771cb2ffe
添加了云朵生成,障碍物小鸟,小恐龙下蹲,模拟重力加速度
通过ADC生成随机数,进而进行不同障碍物的随机生成,碰撞体积判定优化.

接线:SCL-------PB6
SDA-------PB7
ADC 初始化 -------PA1
板载 初始化-------PC13
小恐龙跳跃按键-------PA0
小恐龙趴下按键 -------PA2


*/

#include "stm32f10x.h"
#include "si2c.h"
#include "oled.h"
#include "hyjk16.h"
#include "delay.h"
#include "stdlib.h"

SI2C_TypeDef si2c;
OLED_TypeDef oled;

const uint8_t Dinosaur1[]={// 'Dinosaur1',小恐龙1号状态 24x24px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x07, 0xfc, 0x00, 0x07, 0xfc, 0x00, 0x0f, 0xfe, 0x00, 0x0f, 0x3e, 0x00, 0x0f, 0x3e, 0x20, 0x0f, 
0xfe, 0x30, 0x0f, 0xfe, 0x38, 0x1f, 0xe0, 0x3f, 0x3f, 0xfc, 0x3f, 0xff, 0xc0, 0x1f, 0xff, 0x80, 
0x0f, 0xff, 0xe0, 0x0f, 0xff, 0xa0, 0x07, 0xff, 0x00, 0x03, 0xfe, 0x00, 0x00, 0xe6, 0x00, 0x00, 
0xce, 0x00, 0x00, 0xcc, 0x00, 0x00, 0xee, 0x00};
const uint8_t Dinosaur2[]={// 'Dinosaur2',小恐龙2号状态 24x24px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x07, 0xfc, 0x00, 0x07, 0xfc, 0x00, 0x0f, 0xfe, 0x00, 0x0f, 0x3e, 0x00, 0x0f, 0x3e, 0x20, 0x0f, 
0xfe, 0x30, 0x0f, 0xfe, 0x38, 0x1f, 0xe0, 0x3f, 0x3f, 0xfc, 0x3f, 0xff, 0xc0, 0x1f, 0xff, 0x80, 
0x0f, 0xff, 0xe0, 0x0f, 0xff, 0xa0, 0x07, 0xff, 0x00, 0x03, 0xfe, 0x00, 0x00, 0xf6, 0x00, 0x00, 
0xc6, 0x00, 0x00, 0xc7, 0x00, 0x01, 0xc3, 0x80};
const uint8_t Dinosaur3[]={// 'Dinosaur3',小恐龙3号下蹲状态 24x12px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0xfc, 0xbe, 0x3f, 0xff, 0xfe, 0x1f, 0xff, 0xfe, 0x0f, 
0xff, 0xf0, 0x07, 0xfc, 0xe0, 0x03, 0xfc, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 
0x00, 0x00, 0x00, 0x00};

const uint8_t Obstacle1[]={// 'obstacle1',障碍物1小仙人掌, 7x12px
0x30, 0x30, 0x32, 0xb2, 0xfa, 0x7e, 0x3e, 0x3c, 0xbc, 0xf8, 0x38, 0x38};
const uint8_t Obstacle2[]={// 'obstacle2',障碍物2 大仙人掌,, 8x16px
0x00, 0x18, 0x18, 0x5a, 0x5a, 0x5a, 0x5a, 0x7a, 0x3a, 0x1c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x08};
const uint8_t Obstacle3[]={// 'obstacle3',障碍物3 小鸟,18x12px
0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x03, 0xf0, 0x00, 0x01, 
0xfe, 0x00, 0x01, 0xfe, 0x00, 0x01, 0xf8, 0x00, 0x01, 0x80, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 
0x00, 0x00, 0x00, 0x00};

const uint8_t DinosaurBlood[]={//心形血量,7x6px
0x6c, 0xfe, 0xfe, 0x7c, 0x38, 0x10};
const uint8_t Cloud1[]={// // 'Cloud1', 云朵1 20x10px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x3f, 0xfe, 0x00, 0x3f, 0xff, 0x80, 0x3f, 
0xff, 0x80, 0x0f, 0xff, 0x00, 0x07, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t Cloud2[]={// // 'Cloud2', 云朵2 20x10px
0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x0f, 0xfc, 0x00, 0x3f, 0xfe, 0x00, 0x3f, 0xff, 0x00, 0x3f, 
0xff, 0x00, 0x1f, 0xfe, 0x00, 0x07, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t Cloud3[]={// // 'Cloud3', 云朵3 20x10px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x01, 0xfe, 0x00, 0x01, 0xff, 0x00, 0x03, 
0xfe, 0x00, 0x07, 0xfc, 0x00, 0x07, 0xf0, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00};


void My_SoftwareI2C_Init(void);
void My_OLEDScreen_Init(void);
int i2c_write_bytes(uint8_t addr,const uint8_t*pdata,uint16_t size);
void App_Button_Init(void);
void App_OnBoardLED_Init (void);
void ADC1_Init(void);
uint16_t ADC1_ReadNoise(void);
void Random_Init(void);
uint32_t Get_Random(void);
uint32_t Get_RandomRange(uint32_t min, uint32_t max);


void jump (void);
void impact1(void);
void impact2(void);
void impact3(void);
void CloudMove(void);
void DinosaurDown(void);
void Show_Blood(uint8_t blood);


int DinosaurY= 26;         //恐龙y轴
int ObstacleX = 118;    //障碍物x轴   
int gameover = 0;   // 游戏结束标志
uint16_t Jump_Flag = 0 ; //跳跃标志
uint16_t Back_Flag = 0 ;  //回落标志
uint16_t Score =0 ;         //分数
uint16_t Gravity =0 ;      //模拟重力加速度
uint8_t thread = 0;        //恐龙两种状态
uint8_t Dinosaur_Blood = 3 ;      //恐龙血量
uint16_t DinosaurDown_Flag = 0 ;     //恐龙趴下状态
uint32_t Obstacle_Random = 0;    //随机数障碍物标志

uint32_t Cloud_Random = 0;       //随机生成云朵种类
uint32_t Cloud_ActiveFlag =0 ;   //判断云朵是否在场上,在则不生成新云朵
int CloudX ;                //云朵x轴
int CloudY ;                //云朵y轴


int main(void)
{
	//初始化
	My_SoftwareI2C_Init();
	My_OLEDScreen_Init();
  App_Button_Init();
  App_OnBoardLED_Init();
	//初始化随机数
	SystemInit();
  ADC1_Init();
  Random_Init();

while(1)
{		
 if(gameover == 1)
	 {OLED_Clear(&oled);
		OLED_SetBrush(&oled,BRUSH_TRANSPARENT);  //透明画刷
		OLED_SetPen(&oled, PEN_COLOR_WHITE,1);    //白色画笔
		OLED_SetCursor(&oled,35,32);         //光标（35，32)
		OLED_DrawString(&oled,"Game Over");    //打印Game Over
    OLED_SendBuffer(&oled);
    }

		
		else
		{
		OLED_Clear(&oled);

		//画地线
		OLED_SetCursor(&oled,0,50);
    OLED_LineTo(&oled,127,50);
		//画云朵
		CloudMove();
		//画血条
		Show_Blood(Dinosaur_Blood);
			
		//得分
		Score = Score+1 ;
		OLED_SetCursor(&oled,5,10);
		OLED_Printf(&oled,"Score:%d",Score);	
		//按键是否决定跳跃
		jump();	
			
		//选取打印障碍物并判断是否碰撞	 
		ObstacleX= ObstacleX-5;
		if(ObstacleX <= -50 )
		{Obstacle_Random = Get_RandomRange(1, 100);
		ObstacleX =118;}

		if(Obstacle_Random % 4 == 0 || Obstacle_Random % 4 == 1)
		{
		OLED_SetCursor(&oled,ObstacleX,38);
		OLED_DrawBitmap(&oled,7,12,Obstacle1);
		impact1();
		}
		if(Obstacle_Random%4==2)
		{
		OLED_SetCursor(&oled,ObstacleX,34);
		OLED_DrawBitmap(&oled,8,16,Obstacle2);
		impact2();
		}
		if(Obstacle_Random%4==3)
		{
		OLED_SetCursor(&oled,ObstacleX,25);
		OLED_DrawBitmap(&oled,18,12,Obstacle3);
    impact3();
		}
		  
		 
		  //恐龙的三种状态
		 DinosaurDown();
		 if(DinosaurDown_Flag == 1)
		 {
			OLED_SetCursor(&oled,0,38);
			OLED_DrawBitmap(&oled,24,12,Dinosaur3);
		 }
		 
			else
			{
						thread = thread +1 ;
					if(thread>=8){thread = 0 ;}
					
					if(thread<= 3 )
					{
						OLED_SetCursor(&oled,0,DinosaurY);
						OLED_DrawBitmap(&oled,24,24,Dinosaur1);

					}
					else
					{
						OLED_SetCursor(&oled,0,DinosaurY);
						OLED_DrawBitmap(&oled,24,24,Dinosaur2);
					}
			}
		}
			
			OLED_SendBuffer(&oled);		
		Delay(10);
		
 }
}



//碰撞判断 碰撞物一
void impact1(void)
{
	if(ObstacleX <= 10&&DinosaurY >= 14 && ObstacleX >= 8)
	{
		ObstacleX = 180 ;DinosaurY =25;
		Jump_Flag = 0;  Back_Flag = 1;
		Delay(500);
		Dinosaur_Blood = Dinosaur_Blood-1;
		if(Dinosaur_Blood == 0){gameover = 1;}
	}
}
//碰撞判断,碰撞物二
void impact2(void)
{
	if(ObstacleX <= 12&&DinosaurY >= 10 && ObstacleX >= 8)
	{
		ObstacleX = 180 ;DinosaurY =25;
		Jump_Flag = 0;  Back_Flag = 1;
		Delay(500);
		Dinosaur_Blood = Dinosaur_Blood-1;
		if(Dinosaur_Blood == 0){gameover = 1;}	
	}
}

//碰撞判断,碰撞物三
void impact3(void)
{
	if(ObstacleX <= 12 &&DinosaurY <= 36 && ObstacleX >= 6 && DinosaurDown_Flag==0)
	{
		ObstacleX = 180 ;DinosaurY =25;
		Jump_Flag = 0;  Back_Flag = 1;
		Delay(500);
		Dinosaur_Blood = Dinosaur_Blood-1;
		if(Dinosaur_Blood == 0){gameover = 1;}
	}
}


//判断是否随机生成云朵
void CloudMove(void)
{
	//判断场上是否有云,没有云
    if (Cloud_ActiveFlag == 0) 
		 {
        int Cloud_Generate = Get_RandomRange(0, 100);
        if (Cloud_Generate<= 1 )   // 1%概率生成
				{  
					Cloud_ActiveFlag = 1;
					CloudX = 128;  // 屏幕最右边（假设128宽）
					CloudY= Get_RandomRange(5, 15); // 随机高度
					OLED_SetCursor(&oled,CloudX,CloudY);
					OLED_DrawBitmap(&oled,20,10,Cloud3);
				}
     }
		 
		 //判断场上是否有云,生成云
		 if (Cloud_ActiveFlag == 1)
      { 
			 CloudX =CloudX- 1;
			 OLED_SetCursor(&oled,CloudX,CloudY);
			 Cloud_Random = Get_RandomRange(1, 100);
			 if(Obstacle_Random%3==0){OLED_DrawBitmap(&oled,20,10,Cloud1);}
			 if(Obstacle_Random%3==1){OLED_DrawBitmap(&oled,20,10,Cloud2);}
			 if(Obstacle_Random%3==2){OLED_DrawBitmap(&oled,20,10,Cloud3);}				 
			}
		 if (CloudX< -10) {Cloud_ActiveFlag = 0;}
}

//生成心形血量
void Show_Blood(uint8_t blood)
{
    uint8_t i;
    for(i = 0; i < blood; i++)
    {
        // 每个心间隔 9px，右上角对齐
        int x = 128 - (i+1) * 9;
        int y = 0;   
        OLED_SetCursor(&oled, x, y);
        OLED_DrawBitmap(&oled, 7, 6, DinosaurBlood);
    }
}

////恐龙跳跃
void jump (void)
{
				//按键恐龙跳起来,并回落过程
			if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == Bit_RESET && Jump_Flag == 0 && Back_Flag == 0)
			{
				Delay(10);
				if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == Bit_RESET && Jump_Flag == 0 && Back_Flag == 0)
				{Jump_Flag = 1;} //跳起来状态
			}
			if(Jump_Flag == 1)
			{
				Gravity=6;
				Gravity=Gravity-2;
				DinosaurY=DinosaurY-Gravity ;
			}
			if(DinosaurY <= 0)  //回落判断,限制高度
			{
				Jump_Flag = 0;
				Back_Flag = 1 ;
			}
			if(Back_Flag == 1)
			{
			Gravity=0;
			Gravity=Gravity+3;

				DinosaurY = DinosaurY+Gravity;
			}
			if(DinosaurY >= 26)  //回落判断
			{
				Back_Flag = 0 ;
				DinosaurY = 26 ;
			}	
}

//恐龙趴下
void DinosaurDown (void)
{
	//按键恐龙趴下
			if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2) == Bit_RESET )
			{
				Delay(10);
				if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2) == Bit_RESET )
				{ 
					DinosaurDown_Flag =1;
				} 
			}
			else {DinosaurDown_Flag =0;}
}



//初始化i2c
void My_SoftwareI2C_Init(void)
{
  si2c.SCL_GPIOx = GPIOB;
  si2c.SCL_GPIO_Pin = GPIO_Pin_6;
  si2c.SDA_GPIOx = GPIOB;
  si2c.SDA_GPIO_Pin = GPIO_Pin_7;
  My_SI2C_Init(&si2c);
}
	///
int i2c_write_bytes (uint8_t addr,const uint8_t*pdata,uint16_t size)
{
	return My_SI2C_SendBytes(&si2c, addr, pdata, size);
}
/////初始化我的屏幕
void My_OLEDScreen_Init(void)
{
	OLED_InitTypeDef OLED_InitStruct;
	OLED_InitStruct.i2c_write_cb = i2c_write_bytes ;
	OLED_Init(&oled,&OLED_InitStruct);
}

//初始化按键
void App_Button_Init(void)
{

		RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);
		GPIO_InitTypeDef GPIO_InitStruct;
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_2;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_Init(GPIOA, &GPIO_InitStruct);
}


//初始化板载
void App_OnBoardLED_Init (void)
{
		RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOC, ENABLE);
		GPIO_InitTypeDef GPIO_InitStruct;
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
	  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
		GPIO_Init(GPIOC, &GPIO_InitStruct);
}






// ---------------- ADC 初始化 -------PA1---------
void ADC1_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);

    // PA1 -> ADC1_IN1
    GPIO_InitTypeDef gpioInit;
    gpioInit.GPIO_Pin = GPIO_Pin_1 ;
    gpioInit.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &gpioInit);

    ADC_InitTypeDef adcInit;
    adcInit.ADC_Mode = ADC_Mode_Independent;
    adcInit.ADC_ScanConvMode = DISABLE;
    adcInit.ADC_ContinuousConvMode = DISABLE;
    adcInit.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adcInit.ADC_DataAlign = ADC_DataAlign_Right;
    adcInit.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &adcInit);

    ADC_Cmd(ADC1, ENABLE);

    // 校准
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
}

// ---------------- 读取 ADC 噪声 ----------------
uint16_t ADC1_ReadNoise(void)
{
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_239Cycles5);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    return ADC_GetConversionValue(ADC1);
}

// ---------------- 随机数初始化 ----------------
void Random_Init(void)
{
    uint16_t noise = ADC1_ReadNoise();
    srand(noise);   // 用 ADC 噪声作为随机数种子
}

// ---------------- 获取随机数 ----------------
uint32_t Get_Random(void)
{
    return rand();   // 直接返回一个随机数
}

// ---------------- 获取范围随机数 ----------------
uint32_t Get_RandomRange(uint32_t min, uint32_t max)
{
    return (rand() % (max - min + 1)) + min;
}
