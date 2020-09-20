#include <STC15F2K60S2.h>
#include <intrins.h>
/*---------宏定义---------*/
#define uint unsigned int
#define uchar unsigned char
#define cstAdcPower 0X80     						/*ADC电源开关*/
#define cstAdcFlag 0X10      						/*当A/D转换完成后，cstAdcFlag要软件清零*/
#define cstAdcStart 0X08     						/*当A/D转换完成后，cstAdcStart会自动清零，所以要开始下一次转换，则需要置位*/
#define cstAdcSpeed90 0X60   						/*ADC转换速度 90个时钟周期转换一次*/
#define cstAdcChs17 0X07     						/*选择P1.7作为A/D输入*/
#define cstUart2Ri  0x01                			//接收中断请求标志位
#define cstUart2Ti  0x02               				//发送中断请求标志位
#define cstNoneParity 0			       				//无校验
#define PARITYBIT cstNoneParity         			//定义校验位
													/*串口波特率相关*/
#define cstFosc 11059200L               			//系统时钟频率 
#define cstBaud2 9600                				//串口波特率    
#define cstT2HL (65536-(cstFosc/4/cstBaud2)) 		//定时器初始时间


/*---------引脚别名定义---------*/
sbit sbtLedSel = P2 ^ 3;     						/*数码管和发光二极管选择位*/
sbit sbtSel0 = P2 ^ 0;
sbit sbtSel1 = P2 ^ 1;
sbit sbtSel2 = P2 ^ 2;
sbit sbtKey1 = P3 ^ 2;
sbit sbtKey2 = P3 ^ 3;
sbit sbtKey3 = P1 ^ 7;
sbit sbtBeep = P3 ^ 4;								//蜂鸣器引脚
sbit sbtM485_TRN  = P3 ^ 7 ;   						//定义MAX485使能引脚，为1时发送，为0时接收


/*****定义变量*****/
bit btSendBusy;              						//为1时忙（发送数据），为0时闲
uchar datas;										//数据向量，用于两个玩家之间的数据交互
uchar code dig[]={0,3,4,5,1,7,2,6};						//数码管位选
uchar digIndex=0;									//数码管位选下标
uchar i,j,x,y;										//循环下标(i,j) 棋盘坐标（x,y）
uchar led=0x01;
uchar arrRoll[]={0x73,0x38,0x77,0x6e,0x00,0x00,0x39,0x76,0x79,0x6D,0x6D,0x00,0x00};
uchar arrSegSelect[22] = {													//段选，显示0-fc
0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f,0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71,0x44,0x50,0x40,0x42,0x60,0x00}; 
uchar Board[4][8]={	
			{0,0,0,0,0x0a,0xb,0x13,0x14},
			{1,0,0,0,0x0a,0xb,0x13,0x14},
			{2,0,0,0,0x0a,0xb,0x12,0x12},
			{3,0,0,0,0x0a,0xb,0x10,0x11}};									//棋盘
uchar scoreBoard[8]={0,0,0x12,0xa,0xb,0x12,0,0};							//比分板
uchar curPlayer=0x0a;														//当前玩家
uchar myPlayer=0x0a;														//己方玩家
uchar myScore=0,otherScore=0;												//比分
uchar gameNum;																//比赛局数
uchar blinkIndex=1;															//闪烁位
uchar point=4;																//默认小数点在左侧
uchar count=0;																//定时器辅助计数用	
uchar start=0,over=0;														//游戏是否开始/结束	
uchar winner;																//赢家
bit isSelect=0;															//已经选择完毕	
bit btBeepFlag;															//按键声音标志
uchar ucTimerH,ucTimerL;   													//定义定时器的重装值
uchar code arrMusicBM[] =     												//音乐代码，歌曲为《同一首歌》，格式为: 音符, 节拍
{
	//音符的十位代表是低中高八度，1代表高八度，2代表中八度，3代表高八度
	//个位代表简谱的音符，例如0x15代表低八度的S0，0x21代表中八度的DO。
	//节拍则是代表音长，例如：0x10代表一拍，0x20代表两拍，0x08代表1/2拍
	//音符的十位代表是低八度，中八度还是高八度，1代表低八度，2代表中八度，3代表高八度
	//个位代表简谱的音符，例如0x15代表低八度的S0，0x21代表中八度的DO。
	//节拍则是代表音长，例如：0x10代表一拍，0x20代表两拍，0x0代表1/2拍
	0x23,0x04,0x23,0x08,0x23,0x08,0x21,0x04,0x23,0x08,0x25,0x10,0x15,0x10,
	0x21,0x08,0xff,0x04,0x15,0x04,0xff,0x08,0x13,0x08,0xff,0x04,0x16,0x08,0x17,0x04,0xff,0x04,0x17,0x04,
	0x16,0x08,0x15,0x06,0x23,0x05,0x25,0x05,0x26,0x08,0x24,0x04,0x25,0x04,0xff,0x04,0x23,0x08,0x21,0x04,
	0x22,0x04,0x17,0x08,0xff,0x04,0x21,0x08,0xff,0x04,0x15,0x04,0xff,0x08,0x13,0x08,0xff,0x04,0x16,0x08,
	0x17,0x04,0xff,0x04,0x17,0x04,0x16,0x08,0x15,0x06,0x23,0x05,0x25,0x05,0x26,0x08,0x24,0x04,0x25,0x04,
	0xff,0x04,0x23,0x08,0x21,0x04,0x22,0x04,0x17,0x08,0xff,0x04,0xff,0x08,0x25,0x04,0x24,0x04,0x24,0x04,
	0x23,0x08,0x23,0x04,0xff,0x04,0x15,0x04,0x16,0x04,0x21,0x04,0xff,0x04,0x16,0x04,0x21,0x04,0x22,0x04,
	0xff,0x08,0x25,0x04,0x24,0x04,0x24,0x04,0x23,0x08,0x23,0x04,0xff,0x04,0x31,0x08,0x31,0x04,0x31,0x08,
	0xff,0x08,0xff,0x08,0x25,0x04,0x24,0x04,0x24,0x04,0x23,0x08,0x23,0x04,0xff,0x04,0x15,0x04,0x16,0x04,
	0x21,0x04,0xff,0x04,0x16,0x04,0x21,0x04,0x22,0x04,0xff,0x08,0x25,0x08,0xff,0x04,0x24,0x08,0xff,0x04,
	0x23,0x08,0xff,0x08,0xff,0x10,0x00,0x00
};
uchar code arrMusicFail[] ={
		0x21,0x08,0x24,0x10,0x24,0x08,0x24,0x08,0x24,0x08,0x23,0x08,0x22,0x09,0x21,0x08,0x00,0x00
};
uchar code arrMusicSuccess[] ={
		0x23,0x04,0x23,0x08,0x23,0x04,0xff,0x04,0x21,0x04,0x23,0x08,0x25,0x10,0x00,0x00
};
uchar code arrMusicToTimerNum[] =  
{
    //此数组数据为各个音符在定时器中的重装值，第一列是高位，第二列是低位
    0xf8, 0x8c,   //低八度，低1
    0xf9, 0x5b,
    0xfa, 0x15,   //低3
    0xfa, 0x67,
    0xfb, 0x04,   //低5
    0xfb, 0x90,
    0xfc, 0x0c,   //低7
    0xfc, 0x44,   //中央C调
    0xfc, 0xac,   //中2
    0xfd, 0x09,
    0xfd, 0x34,   //中4
    0xfd, 0x82,
    0xfd, 0xc8,   //中6
    0xfe, 0x06,
    0xfe, 0x22,   //高八度，高1
    0xfe, 0x56,
    0xfe, 0x6e,   //高3
    0xfe, 0x9a,
    0xfe, 0xc1,   //高5
    0xfe, 0xe4,
    0xff, 0x03    //高7
};

/*---------串口2初始化及波特率发生函数---------*/
void Uart2Init( void )
{
    S2CON = 0x10 ;      //定义无校验位，允许串行口2接收，串行口2的控制寄存器
    T2L = cstT2HL ;     //设置波特率重装值
    T2H = cstT2HL >> 8 ;
    AUXR |= 0x14 ;      //T2为1T模式，并启动定时器2
}
//初始化棋盘
void refreshChessBoard(){
	//初始化棋盘
	for(i=1;i<4;i++){
		for(j=0;j<4;j++)
			Board[i][j]=0;
	}
	
	//初始化玩家信息
	for(i=1;i<4;i++){
		Board[i][4]=myPlayer;						//显示我的玩家
		Board[i][5]=myPlayer^0x01;					//显示对方玩家		
		Board[i][0]=i;								//行号
	}
	
	//初始化小数点
	if(myPlayer==0x0b) 
		point=5;
	else
		point=4;

	//初始化光标
	x=2;
	y=2;
}
//刷新比分板
void refreshBoard(){
	scoreBoard[3]=myPlayer;			//左侧显示的是我的得分
	scoreBoard[4]=myPlayer^0x01;

	scoreBoard[1]=myScore;			//更新比分
	scoreBoard[7]=otherScore;
}
void init(){
	P2M1=0x00;//P2推挽	
	P2M0=0xff;
	
	P0M1=0x00;//P0推挽
	P0M0=0xff;

	P3M0 = 0x10;		//P3.4推挽
	sbtLedSel = 0;      //选择数码管作为输出
	P1ASF = 0x80;       //P1.7作为模拟功能A/D使用
	ADC_RES = 0;        //转换结果清零
	ADC_CONTR = 0x8F;   //cstAdcPower = 1
	CLK_DIV = 0X00;     //ADRJ = 0    ADC_RES存放高八位结果
	btBeepFlag = 0;			//蜂鸣器初始化为0
	
	
	x=2;								//坐标初始化为（2,2）
	y=2;
	digIndex = 0;				//初始化下标和辅助计数值
	winner=0;
	start=0;						//开始，结束	
	over=0;
	myScore=0;
	myScore=0;
	count = 0;				//滚动辅助
	myPlayer=0x0a;
	datas=0;				//数据初始化
	gameNum=0;

	//定时器T0
	TMOD=0x00;						//16位可重装模式
	ET0=1;								//开启定时器T0中断
	TH0=(65535-1000)/256;	//定时1ms
	TL0=(65535-1000)%256;
	TR0=1;								//T0开启定时	
	
	//定时器T1
	TH1 = 0xD8;
	TL1 = 0xEF;
	ET1 = 1;
	TR1 = 0;						//定时器处于关闭状态，当播放音乐的时候，将会开启
	EA=1;
	
	//485初始化  				波特率生成
	sbtM485_TRN = 0 ;   //初始为接收状态
	P_SW2 |= 0x01 ;     //切换串口2的管脚到P4.6,P4.7
	Uart2Init() ;
	btSendBusy = 1 ;
	IE2 |= 0x01 ;       //开串行口2中断
	IP2 |= 0x01 ;       //设置串行口中断：高优先级
	
	datas=0;						//初始化交互数据为0
}
/*---------延时5ms子函数--------*/
void Delay5ms()     //@11.0592MHz  延时5ms
{
    unsigned char i, j;
    i = 54;
    j = 199;
    do
    {
        while ( --j );
    }
    while ( --i );
}

void Delay30ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 2;
	j = 67;
	k = 183;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}
void Delay50ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 3;
	j = 26;
	k = 223;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}
/*---------延时100ms子函数--------*/
void Delay100ms()       //@11.0592MHz  延时100ms
{
    unsigned char i, j, k;
    _nop_();
    _nop_();
    i = 5;
    j = 52;
    k = 195;
    do
    {
        do
        {
            while ( --k );
        }
        while ( --j );
    }
    while ( --i );
}
/*---------延时2000ms------------*/
void Delay2000ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 85;
	j = 12;
	k = 155;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

/*---------延时子函数---------*/
void DelayMs( unsigned int xms )
{
    uint i, j;
    for( i = xms; i > 0; i-- )
        for( j = 124; j > 0; j-- );
}
/*---------获取AD值子函数--------*/
unsigned char GetADC()
{
    uchar ucAdcRes;
    ADC_CONTR = cstAdcPower | cstAdcStart | cstAdcSpeed90 | cstAdcChs17;//没有将cstAdcFlag置1，用于判断A/D是否结束
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    while( !( ADC_CONTR & cstAdcFlag ) ); //等待直到A/D转换结束
    ADC_CONTR &= ~cstAdcFlag;           	//cstAdcFlagE软件清0
    ucAdcRes = ADC_RES;                 	//获取AD的值
    return ucAdcRes;											//返回转换值
}

/*---------获取导航按键值子函数--------*/
uchar NavKeyCheck()
{
    unsigned char key;
    key = GetADC();     //获取AD的值
    if( key != 255 )    //有按键按下时
    {
        Delay5ms();
        key = GetADC();
        if( key != 255 )            //按键消抖 仍有按键按下
        {
            key = key & 0xE0;       //获取高3位，其他位清零
            key = _cror_( key, 5 ); //循环右移5位 获取A/D转换高三位值，减小误差
            return key;
        }
    }
    return 0x07;        //没有按键按下时返回值0x07
}

/*------------按键音效---------------*/
void tick1(){			//滴	
		if(curPlayer==0x0b) TH0=(65535-500)/256;	//玩家B落子的时候改变音调
		btBeepFlag=1;
		Delay100ms();	//延时100ms
		btBeepFlag=0;	//停止发声音
		TH0=(65535-1000)/256;	//定时1ms
}
void tick2(){			//滴滴
		if(curPlayer==0x0b) TH0=(65535-500)/256;	//玩家B落子的时候改变音调
		btBeepFlag=1;
		Delay50ms();	//延时100ms
		btBeepFlag=0;	//停止发声音	
		Delay30ms();
		btBeepFlag=1;
		Delay50ms();	//延时100ms
		btBeepFlag=0;	//停止发声音
		TH0=(65535-1000)/256;	//定时1ms
}
void tick3(){			//开始音效
		TH0=(65535-200)/256;
		btBeepFlag=1;
		Delay50ms();	//延时100ms
		btBeepFlag=0;	//停止发声音	
		Delay30ms();
		btBeepFlag=1;
		Delay50ms();	//延时100ms
		btBeepFlag=0;	//停止发声音
		TH0=(65535-1000)/256;	//定时1ms
}
void tick4(){			//滴	
		btBeepFlag=1;
		Delay5ms();		//延时100ms
		btBeepFlag=0;	//停止发声音
		TH0=(65535-1000)/256;	//定时1ms
}
/*-----数据发送函数----*/
void sendDatas()
{
    sbtM485_TRN = 1 ;					//MAX485使能引脚，开启485串口
    S2BUF = datas | (myPlayer<<7);		//将发送数据写入缓存区,带上来源机,A玩家则是以0开头，B玩家以1开头，1010，1011
    while( btSendBusy );				//等待数据发送完毕
    btSendBusy = 1 ;					//再次设置为1，表示发送忙
    sbtM485_TRN = 0 ;					//关闭485串口
	datas=0;
}
/*---------取址子函数---------*/
//取出tem音符在arrMusicToTimerNum数组中的位置值
uchar GetPosition( uchar tem ) 
{
    uchar ucBase, ucOffset, ucPosition;     //定义曲调，音符和位置
    ucBase = tem / 16;            //高4位是曲调值,基址
    ucOffset = tem % 16;          //低4位是音符，偏移量
    if( ucBase == 1 )              //当曲调值为1时，即是低八度，基址为0
        ucBase = 0;
    else if( ucBase == 2 )          //当曲调值为2时，即是中八度，基址为14
        ucBase = 14;
    else if( ucBase == 3 )          //当曲调值为3时，即是高八度，基址为28
        ucBase = 28;
    //通过基址加上偏移量，即可定位此音符在arrMusicToTimerNum数组中的位置
ucPosition = ucBase + ( ucOffset - 1 ) * 2; 
    return ucPosition;            //返回这一个位置值
}

/*---------播放音乐功能函数---------*/
void PlayMusic(uchar time,uchar arrMusic[])
{
    uchar ucNoteTmp, ucRhythmTmp, tem; 	// ucNoteTmp为音符，ucRhythmTmp为节拍
    uchar i = 0;
		ucNoteTmp = arrMusic[0];    					//如果碰到结束符,延时1秒,回到开始再来一遍
    while( ucNoteTmp != 0x00 && ((!start)||over))	//当开机之前||游戏结束之后才可以播放音乐
    {
        ucNoteTmp = arrMusic[i];    //如果碰到结束符,延时1秒,回到开始再来一遍
        if( ucNoteTmp == 0xff )  //若碰到休止符,延时100ms,继续取下一音符
        {
            i = i + 2;
            DelayMs( 100 );
            TR1 = 0;
        }
        else                     //正常情况下取音符和节拍
        {
            //取出当前音符在arrMusicToTimerNum数组中的位置值
						tem = GetPosition( arrMusic[i] );              
						//把音符相应的计时器重装载值赋予ucTimerH和ucTimerL
						ucTimerH = arrMusicToTimerNum[tem];  
            ucTimerL = arrMusicToTimerNum[tem + 1];
            i++;
            TH1 = ucTimerH;           //把ucTimerH和ucTimerL赋予计时器
            TL1 = ucTimerL;
            ucRhythmTmp = arrMusic[i];      //取得节拍
            i++;
        }
        TR1 = 1;                          //开定时器1
        DelayMs( ucRhythmTmp * time );    //等待节拍完成, 通过P3^4口输出音频
        TR1 = 0;                          //关定时器1
    }
}
//LED灯光
void displayLED(){
	led=0x01;
	sbtLedSel=1;						//开启LED显示
	j=0;
	for(i=0;i<40;i++){				
		if(led==0x80){
			j=!j;
		}
		else if(led==0x01){
			j=!j;
		}

		if(j)						//向左右移动
			led=led<<1;
		else
			led=led>>1;
		P0=led;
		DelayMs(200);
	}
	sbtLedSel=0;
}
/*-------------判断输赢-------------*/
uchar isOver(){				
    //假设一定分出胜负的情况
	for(i=1;i<4;i++){
        if(Board[i][1]!=0&&Board[i][1]==Board[i][2]&&Board[i][1]==Board[i][3]) return Board[i][1];	//行
        if(Board[1][i]!=0&&Board[1][i]==Board[2][i]&&Board[1][i]==Board[3][i]) return Board[1][i];	//列
    }	
    if(Board[1][1]!=0&&Board[1][1]==Board[2][2]&&Board[1][1]==Board[3][3]) return Board[1][1];		//左斜对角线
    if(Board[1][3]!=0&&Board[1][3]==Board[2][2]&&Board[2][2]==Board[3][1]) return Board[1][3];		//左斜对角线
    
    //棋盘空
    for(i=1;i<4;i++)
        for(j=1;j<4;j++)
            if(Board[i][j]==0)	//存在空位，说明还没有下完棋
                return 0;
    //最后一种情况，棋盘满了，平局
    return 0x10;				
}
void displayChessBoard(){				//展示棋盘
	P0=0;	
	blinkIndex=y;						//设置闪烁							
	if(digIndex==y)						//如果当前位选位和闪烁位一致，说明需要进行闪烁操作 
	{
		if(count>20){					//计满,点亮
			if(count==50) count=0;
		}
		else							//熄灭
			digIndex++;		
	}
	if (digIndex==point && start) 				//光标
		P0 = arrSegSelect[Board[x][digIndex]] | 0x80;
	else
		P0 = arrSegSelect[Board[x][digIndex]];
	P2=dig[digIndex];					//位选	
}				
void displayScore(){					//显示比分
	P0=0;
	P2=digIndex;						//位选
	P0 = arrSegSelect[scoreBoard[digIndex]];		//段选
}
//数码管滚动显示
void displayRoll(uchar arr[]){
	//滚动一次
	if(count>20){	
		count=0;										//分频归零
		j=arr[0];
		for(i=0;i<12;i++) arr[i]=arr[i+1];
		arr[12]=j;
	}
	P0=0;
	P2=digIndex;										//位选
	P0 = arr[digIndex];									//段选
}
/*---------游戏开始----------------*/
void gameStart(){
	PlayMusic(190,arrMusicBM);					//播放开机音乐
	while(!start);								//等待游戏开始
	Delay5ms();
	tick3();
	
	//闪烁数码管
	//重新定时为45ms
	TH0 = ( 65535 - 45000 ) / 256; 				//定时器0的高八位设置
	TL0 = ( 65535 - 45000 ) % 256; 				//定时器0的低八位设置，这里总体就是设置定时器0的初始值是1ms

	DelayMs(4000);

	//重新定时为45ms
	TH0 = ( 65535 - 1000 ) / 256; 				//定时器0的高八位设置
	TL0 = ( 65535 - 1000 ) % 256; 				//定时器0的低八位设置，这里总体就是设置定时器0的初始值是1ms		

	//更新棋盘
	refreshChessBoard();
}
/*-------------游戏结束处理--------------*/
void gameOver(){
		if(gameNum==3) return;
		if(winner==myPlayer){
			PlayMusic(100,arrMusicSuccess);
			myScore++;
			//displayLED();
		}
		else{
			PlayMusic(100,arrMusicFail);
			otherScore++;
		}
		refreshBoard();					//刷新比分
			
		//重新定时为45ms
		TH0 = ( 65535 - 45000 ) / 256; //定时器0的高八位设置
		TL0 = ( 65535 - 45000 ) % 256; //定时器0的低八位设置，这里总体就是设置定时器0的初始值是1ms
		Delay2000ms();
		//重新定时为45ms
		TH0 = ( 65535 - 1000 ) / 256; //定时器0的高八位设置
		TL0 = ( 65535 - 1000 ) % 256; //定时器0的低八位设置，这里总体就是设置定时器0的初始值是1ms	
		
		//开始新的一轮游戏
		gameNum++;						//局数计数
		curPlayer=0x0a;					//重新定位当前玩家
		winner=0;
		start=0;
		over=0;
		refreshChessBoard();			//刷新初始化棋盘

		if(gameNum!=3){
			gameStart();
		}
		else							//游戏结束
		{
			PlayMusic(200,arrMusicBM);	//播放背景音乐
			start=1;
			over=1;			
		}						
}
/*----------落子函数------------*/
void playChess(){
	tick2();
	if(over) return;											//游戏已经结束，则无法落子
	Board[x][y]=curPlayer;										//落子
	curPlayer^=0x01;											//0000 1010  ! 0000 1011取反最后一位，交换curPlayer										
	over=isOver();												//判断输赢
	if(over) {
		winner=over;											//获取游戏赢家
	}
	else{
		if(curPlayer==myPlayer)									//左侧闪烁
			point=4;									
		else
			point=5;											//右侧对手闪烁
	}
}

/*---------导航按键处理子函数--------*/
void NavKey_Process()
{
    uchar ucNavKeyCurrent;  						//导航按键当前的状态
    uchar ucNavKeyPast;     						//导航按键前一个状态

    ucNavKeyCurrent = NavKeyCheck();    //获取当前ADC值
    if( ucNavKeyCurrent != 0x07 )       //导航按键是否被按下 不等于0x07表示有按下
    {
        ucNavKeyPast = ucNavKeyCurrent;
        while( ucNavKeyCurrent != 0x07 )        //等待导航按键松开
            ucNavKeyCurrent = NavKeyCheck();
        switch( ucNavKeyPast )
        {
			case 0x01:						//右
				if(y!=3){
					y++;
					tick1();
				}
				break;
			case 0x02:						//下
				if(x!=3){
					x++;
					tick1();
				}
				break;
			case 0x03:						//按下
				if(over) return;			//游戏处于结束状态，则直接返回
				if(Board[x][y]==0)
					playChess();			//落子	
				break;
			case 0x04:
				if(y!=1){
					y--;
					tick1();
				}
				break;							//左
			case 0x05:						//上
				if(x!=1){
					x--;
					tick1();
				}
				break;
        }
		//先发送完数据，再结束游戏
		datas=(1<<(ucNavKeyPast+1));				//编码，设置发送数据
		sendDatas();											
    }
	if(over&&start&&gameNum<3){							
		gameOver();
	}	
    Delay100ms();
}
void selectCharacter(){
	myPlayer^=0x01;									//交换角色信息
	for(i=0;i<4;i++){
		Board[i][4]=myPlayer;
		Board[i][5]=myPlayer^0x01;		//第二个玩家
	}
	for(i=1;i<4;i++){							//更新棋盘第2行
		Board[2][i]=myPlayer;					//显示选择角色的信息
	}
	Board[2][0]=21;								//显示为0
	Board[2][4]=21;								//显示第二行玩家
	Board[2][5]=21;								//显示第二行的玩家
}

//数据解析函数
void getData()
{
	isSelect=1;
	if(!((datas>>7)^(myPlayer&0x01))){							//当标志相同的时候，确认信息
		if(start) return;										//如果交互数据来自自己，那么拒绝接收
		else{
			myPlayer^=0x01;										//异或，保持玩家互斥
		}
	}		
	datas =datas&0x7f;											//去掉最高位的来源信息，0111 1111
	if(datas)													//如果数据不为空
	{
		//检查key1游戏开始功能
		if(datas&0x01){
			start=1;
			//tick4();
		}
		//key2没有发过来的必要，因为双方没有权利知道对方是否查看比分板,
		//但是Key2在角色选择中发挥了作用,这个时候同步棋盘
		if(datas&0x02){
			for(i=0;i<4;i++){
				Board[i][4]=myPlayer;
				Board[i][5]=myPlayer^0x01;				//第二个玩家
			}
			for(i=1;i<4;i++){							//更新棋盘第2行
				Board[2][i]=myPlayer;					//显示选择角色的信息
			}
			Board[2][0]=21;								//显示为0
			Board[2][4]=21;								//显示第二行玩家
			Board[2][5]=21;								//显示第二行的玩家
		}
		datas>>=2;
		//导航键测试
		switch(datas)						
		{
			case 0x01:						//右
				if(y!=3){
					y++;
					tick1();
				}
				break;
			case 0x02:						//下
				if(x!=3){
					x++;
					tick1();
				}
				break;
			case 0x04:						//按下
				//if(over) return;			//游戏处于结束状态，则直接返回
				if(Board[x][y]==0)
					playChess();			//落子								
				break;
			case 0x08:
				if(y!=1){
					y--;
					tick1();
				}
				break;							//左
			case 0x10:						//上
				if(x!=1){
					x--;
					tick1();
				}
				break;
		}	
		datas=0;	//清空数据发送接收，保证只执行一次
	}
}
/*---------串口2中断处理程序，数据接收---------*/
void Uart2_Process( void ) interrupt 8 using 1
{
    if( S2CON & cstUart2Ri )					//无校验&接收中断请求标志位
    {
        datas = S2BUF ;								//从串口中接收数据暂存
        S2CON &= ~cstUart2Ri;   			//接收中断标志位清0
		getData();
    }
    if( S2CON & cstUart2Ti )					//无校验&发送中断请求标志位
    {
        btSendBusy = 0 ;            	//清除忙信号
        S2CON &= ~cstUart2Ti ;      	//发送中断标志位清0
    }
}

/*---------定时器0中断处理函数---------*/
void T0_Process() interrupt 1				//中断
{
	//检测按键Key1游戏开始
	if(sbtKey1==0 && (!start) && (!over))						//按键按下并且start=0并且已经选择了游戏玩家角色，则可以提前开始开始
	{
		tick4();
		if(sbtKey1==0){
			while( !sbtKey1 );
			start=1;											//key1按下，游戏开始
			datas=0x01;											//设置Key1数据，记住，玩家信息在sendData中添加
			sendDatas();										//发送数据				
		}
	}

	//选择角色信息
	if( sbtKey2 == 0 && gameNum==0 && (!start) && (!over))		//当游戏未开始，且游戏局数为0的时候，Key2将会选择玩家
	{
		Delay5ms();                 							//延时消抖
		tick4();
		if( sbtKey2 == 0 )
		{
			while( !sbtKey2 );									//等待K1放开
			selectCharacter();
			isSelect=1;
			datas=0x02;											//发送key2
			sendDatas();										//直接发送数据
		}
	}

	/*****显示部分*****/
	digIndex++;													//数码管的数组下标
	if(digIndex==8){
		digIndex=0;												//下标归零
		count++;											
	}
	if(sbtKey2 == 0 && (start||over||gameNum))					//开始或者结束的时候可以查看棋盘									 
		displayScore();											//3局开始后，被按下，显示比分板	
	else if((!start&&!over&&gameNum==0&&!isSelect)||(start&&over))			//未开始或者已经全局结束
		displayRoll(arrRoll);									//显示滚动字母			 
	else 
		displayChessBoard();									//显示棋盘
	
	//蜂鸣器按键发声
	if(btBeepFlag && (!over))
		sbtBeep = ~sbtBeep;            							//产生方波使得蜂鸣器发声
	else if(TR1==0)
		sbtBeep = 0;                  							//TR1=0的时候，说明没有播放音乐，停止发声，并将sbtBeep端口置于低电平	
}
/*---------定时器1中断处理函数---------*/
//重新装值，并把sbtBeep值取反，产生方波
void T1_Process() interrupt 3                     //计时器控制频率
{
    sbtBeep = ~sbtBeep;
}
void main(){
		init();
		gameStart();									//游戏，开始
		while(1){
			NavKey_Process();
		}
}