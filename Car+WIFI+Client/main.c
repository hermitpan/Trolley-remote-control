/*
	极客领航教程 -嵌入式项目篇
	项目名称：智能小车远程控制系统
	
	硬件设计：
	主控芯片：51单片机
	电机驱动：L298N
	WIFI模块：ESP8266
	电机：两个直流电机
	
	软件设计：
	1、51单片机引脚基本使用
	2、串口使用，初始化，中断函数，用于ESP8266数据传输
	3、定时器使用，初始化，中断函数
	4、PWM设置，利用定时器，设置占空比
*/
#include <REGX52.H>
#include<intrins.h>	
#define u8 unsigned char	
#define u16 unsigned int
	
//使用定时器来输出PWM信号，通过调节占空比实现调试
extern void Motor_Left(bit ReverOrCoro, u8 DutyCycle); 
extern void Motor_Right(bit ReverOrCoro, u8 DutyCycle); 

//使能端，利用PWM波实现调速
sbit ENA = P1^4;
sbit ENB = P1^5;

u8 number; //占空比变量
u8 receiveTable[10]; //接收ESP8266传来的数据
u8 i = 0; //控制变量

void UartInit(void);//串口初始化函数
void Delay_ms(u16 n);//延时子函数
void Sent_ZF(u8 receiveTable);//发送一个字节
void AT_Send_String(u8 *string);//发送字符串
void ESP8266_Init();//ESP8266初始化
void Timer0Config(); //定时器0配置
void Car_Control(); //小车控制函数，根据wifi指令

void Car_Control()
{
	
		if(receiveTable[9]==0x39) //当送入字符9时，停止
		{
				Motor_Left(0, 0), Motor_Right(0, 0);
		}
		
		if(receiveTable[9]==0x31) //当送入字符1时，前进
		{
				Motor_Left(1, number), Motor_Right(0, number);
		}
	
		if(receiveTable[9]==0x32) //当送入字符2时，后退
		{
				Motor_Left(0, number), Motor_Right(1, number);
		}
	
		if(receiveTable[9]==0x33) //当送入字符3时，左转
		{
				Motor_Left(1, number), Motor_Right(0, 0);		
		}
		
		if(receiveTable[9]==0x34) //当送入字符4时，右转
		{
				Motor_Left(1, 0), Motor_Right(0, number);
		}
		
		if(receiveTable[9]==0x35) //当送入字符5时，加速
		{
				number+=20;  //加速
			  if(number >= 100)
				{
					number = 100;
				}
		}
		
		if(receiveTable[9]==0x36) //当送入字符6时，减速
		{
				number -= 20;  //减速
			  if(number <= 40)
				{
				 	number = 40;
				}
		}

}

void main()
{
	Delay_ms(5000); //先延时5s左右
	Timer0Config(); //定时器0初始化
  UartInit(); //串口初始化
  ESP8266_Init(); //ESP8266模块初始化
	number = 60; //默认占空比
	while(1)
	{
		if(receiveTable[9]==0x63) //当为字符 c 时，进入遥控模式
		{
			AT_Send_String("AT+CIPSEND=0,2\r\n"); //向序号0的客户端发送2字节的数据
			Delay_ms(500);
			AT_Send_String("ok\r\n"); //发送的数据内容
			Delay_ms(500);
			while(1)
			{
				Car_Control(); //调用控制函数
			}				
		}
		
	} //while对应 }
} //main函数对应 }

//串口初始化函数
void UartInit(void)		//串口初始化函数
{
	TMOD &= 0x0F; //清除定时器1的配置
	TMOD |= 0x20;	//定时器工作方式2,8位自动重载(0010 0000)
	TL1 = 0xfd;	 //装入初值
	TH1 = 0xfd;	//装入初值	
  IP = 0; //中断优先级相同
	TR1 = 1;	//启动定时器1
	REN=1;    //允许串行口接收数据
	SM0=0;    //工作方式1,10位异步收发
	SM1=1;   
	EA = 1;   //打开全局中断控制
	ES=1;     //打开串行口中断
}

//定时器0初始化函数
void Timer0Config()
{
	TMOD &= 0xF0; //清除定时器0的模式
	TMOD |= 0x01; //设置定时器0模式
	TH0 = 0xFF; //送入初值 (65536 - 130) / 256   65536 - 65406
	TL0 = 0x7E; //(65536 - 130) % 256
	EA = 1; //打开总中断
	ET0 = 1; //打开定时器0中断允许
	TR0 = 1; //定时器0允许控制位
		/*总结定时器初始化的步骤：
		1、选择定时器的模式，设置TMOD 上面是TMOD=0X01;
		   选择模式了，就是确定了定时和计数的寄存器是多少位
	  2、给定时器送入初值，确定每次寄存器溢出的时间
		   TH0=0Xd8;	
	     TL0=0Xf0;	
		3、打开中断允许，就是ET0，EA
		4、开始计时  TR0=1;
	  */
}

//串口发送数据 单个字节
void Sent_ZF(u8 dat)  //发送一个字节
{
	ES = 0; //不允许串行口接受、发送中断
	TI=0; //循环等待发送结束，当发送结束时，TI=1了
	SBUF = dat; //将dat数据存入SBUF中，为一字节
	while(!TI); //当发送完成，TI = 1，那么!TI为假，结束循环
  TI = 0; //TI复位
  ES = 1; //允许串行口接受、发送中断
}

//串口发送数据，字符串
void AT_Send_String(u8 *string)  //发送字符串
{
  while(*string) //当到字符尾时，'\0' 会结束循环
  {
    Sent_ZF(*string++);//调用发送单个字节函数
		Delay_ms(5);
  }
}

//延时函数
void Delay_ms(u16 n)
{
	unsigned int i,j;
	for(i=0;i<n;i++)
	for(j=0;j<123;j++);
}


//ESP8266 初始化函数
void ESP8266_Init()   //esp8266 9600波特率
{
  Delay_ms(1000);
  AT_Send_String("AT+CWMODE=2\r\n"); // = 2 为AP模式,= 1 为客户端模式
  Delay_ms(1000);
	AT_Send_String("AT+CWSAP=esp8266,0123456789,11,4\r\n");//建立WiFi热点
	Delay_ms(1000);
  AT_Send_String("AT+RST\r\n"); //重启模块
	Delay_ms(1000);
  AT_Send_String("AT+CIPMUX=1\r\n");  //设置为多连接模式，启动模块
	Delay_ms(1000);
  AT_Send_String("AT+CIPSERVER=1,8090\r\n"); //服务器的设置端口
}

//串口中断函数 接收数据
void uart() interrupt 4  
{ 
	if(RI == 1) //说明接收到了数据
	{
	  RI = 0; //清除串口接收标志位，只有清除了，下一次才能接收数据
	  receiveTable[i] = SBUF; //将数据存入数组中
  	if(receiveTable[0] == '+') 
		//判断是否为无效数据，由于WiFi模块会自动加上“+PID. ”开头的字符串
		//有效的数据是第十位
		//假如外部通过ESP8266发送数据 5 到51上
		//那么51串口接收到"+PID...5"   5是第十位
		//所以receiveTable[9]才是我们真正的数据
		{
			i++;
		}
		else
		{
		  i = 0;
		}
		
		if(i == 10)
		{
		  i = 0;
		}	
	}
}

