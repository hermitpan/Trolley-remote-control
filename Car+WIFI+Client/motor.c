#include <REGX52.H>

sbit IN1 = P1^0; //定义右边控制引脚
sbit IN2 = P1^1;
sbit IN3 = P1^2; //定义左边控制引脚
sbit IN4 = P1^3;

void Motor_Left(bit ReverOrCoro, unsigned char DutyCycle); //左边电机控制函数
void Motor_Right(bit ReverOrCoro, unsigned char DutyCycle); //右边电机控制函数

unsigned char cnt = 0;

/*
参数定义：
	ReverOrCoro 传递正反转，1为正转，0为反转
	DutyCycle 占空比参数
*/
void Motor_Left(bit ReverOrCoro, unsigned char DutyCycle) 
{
	if(ReverOrCoro == 1)
	{
		IN1 = 1;
		if(cnt <= DutyCycle)
		{
			IN2 = 0;
		}
		else
		{
			IN2 = 1;
		}
	}
	else
	{
		IN2 = 1;
		if(cnt <= DutyCycle)
		{
			IN1 = 0;
		}
		else
		{
			IN1 = 1;
		}
	}
}

/*
参数定义：
	ReverOrCoro 传递正反转，1为正转，0为反转
	DutyCycle 占空比参数
*/
void Motor_Right(bit ReverOrCoro, unsigned char DutyCycle) //
{
	if(ReverOrCoro == 1)
	{
		IN3 = 1;
		if(cnt <= DutyCycle)
		{
			IN4 = 0;
		}
		else
		{
			IN4 = 1;
		}
	}
	else
	{
		IN4 = 1;
		if(cnt <= DutyCycle)
		{
			IN3 = 0;
		}
		else
		{
			IN3 = 1;
		}
	}
}

//定时器0 中断函数  中断号为 1
void InterruptTimer0() interrupt 1
{
	//每次溢出过后，都需要重新设置初值
	TH0 = 0xFF; //给定时器赋初值，定时130us
	TL0 = 0x7E;
	cnt++; //每次溢出cnt自加1，130us自加一次
	if(cnt >= 100) //当cnt大于等于100时，就清0
	{
		cnt = 0;
	}
	//在这里是定义一个周期为 130 * 100 = 13000us，13ms
}