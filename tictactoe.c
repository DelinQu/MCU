#include <STC15F2K60S2.h>
#include <intrins.h>
/*---------�궨��---------*/
#define uint unsigned int
#define uchar unsigned char
#define cstAdcPower 0X80     						/*ADC��Դ����*/
#define cstAdcFlag 0X10      						/*��A/Dת����ɺ�cstAdcFlagҪ�������*/
#define cstAdcStart 0X08     						/*��A/Dת����ɺ�cstAdcStart���Զ����㣬����Ҫ��ʼ��һ��ת��������Ҫ��λ*/
#define cstAdcSpeed90 0X60   						/*ADCת���ٶ� 90��ʱ������ת��һ��*/
#define cstAdcChs17 0X07     						/*ѡ��P1.7��ΪA/D����*/
#define cstUart2Ri  0x01                			//�����ж������־λ
#define cstUart2Ti  0x02               				//�����ж������־λ
#define cstNoneParity 0			       				//��У��
#define PARITYBIT cstNoneParity         			//����У��λ
													/*���ڲ��������*/
#define cstFosc 11059200L               			//ϵͳʱ��Ƶ�� 
#define cstBaud2 9600                				//���ڲ�����    
#define cstT2HL (65536-(cstFosc/4/cstBaud2)) 		//��ʱ����ʼʱ��


/*---------���ű�������---------*/
sbit sbtLedSel = P2 ^ 3;     						/*����ܺͷ��������ѡ��λ*/
sbit sbtSel0 = P2 ^ 0;
sbit sbtSel1 = P2 ^ 1;
sbit sbtSel2 = P2 ^ 2;
sbit sbtKey1 = P3 ^ 2;
sbit sbtKey2 = P3 ^ 3;
sbit sbtKey3 = P1 ^ 7;
sbit sbtBeep = P3 ^ 4;								//����������
sbit sbtM485_TRN  = P3 ^ 7 ;   						//����MAX485ʹ�����ţ�Ϊ1ʱ���ͣ�Ϊ0ʱ����


/*****�������*****/
bit btSendBusy;              						//Ϊ1ʱæ���������ݣ���Ϊ0ʱ��
uchar datas;										//���������������������֮������ݽ���
uchar code dig[]={0,3,4,5,1,7,2,6};						//�����λѡ
uchar digIndex=0;									//�����λѡ�±�
uchar i,j,x,y;										//ѭ���±�(i,j) �������꣨x,y��
uchar led=0x01;
uchar arrRoll[]={0x73,0x38,0x77,0x6e,0x00,0x00,0x39,0x76,0x79,0x6D,0x6D,0x00,0x00};
uchar arrSegSelect[22] = {													//��ѡ����ʾ0-fc
0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f,0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71,0x44,0x50,0x40,0x42,0x60,0x00}; 
uchar Board[4][8]={	
			{0,0,0,0,0x0a,0xb,0x13,0x14},
			{1,0,0,0,0x0a,0xb,0x13,0x14},
			{2,0,0,0,0x0a,0xb,0x12,0x12},
			{3,0,0,0,0x0a,0xb,0x10,0x11}};									//����
uchar scoreBoard[8]={0,0,0x12,0xa,0xb,0x12,0,0};							//�ȷְ�
uchar curPlayer=0x0a;														//��ǰ���
uchar myPlayer=0x0a;														//�������
uchar myScore=0,otherScore=0;												//�ȷ�
uchar gameNum;																//��������
uchar blinkIndex=1;															//��˸λ
uchar point=4;																//Ĭ��С���������
uchar count=0;																//��ʱ������������	
uchar start=0,over=0;														//��Ϸ�Ƿ�ʼ/����	
uchar winner;																//Ӯ��
bit isSelect=0;															//�Ѿ�ѡ�����	
bit btBeepFlag;															//����������־
uchar ucTimerH,ucTimerL;   													//���嶨ʱ������װֵ
uchar code arrMusicBM[] =     												//���ִ��룬����Ϊ��ͬһ�׸衷����ʽΪ: ����, ����
{
	//������ʮλ�����ǵ��и߰˶ȣ�1����߰˶ȣ�2�����а˶ȣ�3����߰˶�
	//��λ������׵�����������0x15����Ͱ˶ȵ�S0��0x21�����а˶ȵ�DO��
	//�������Ǵ������������磺0x10����һ�ģ�0x20�������ģ�0x08����1/2��
	//������ʮλ�����ǵͰ˶ȣ��а˶Ȼ��Ǹ߰˶ȣ�1����Ͱ˶ȣ�2�����а˶ȣ�3����߰˶�
	//��λ������׵�����������0x15����Ͱ˶ȵ�S0��0x21�����а˶ȵ�DO��
	//�������Ǵ������������磺0x10����һ�ģ�0x20�������ģ�0x0����1/2��
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
    //����������Ϊ���������ڶ�ʱ���е���װֵ����һ���Ǹ�λ���ڶ����ǵ�λ
    0xf8, 0x8c,   //�Ͱ˶ȣ���1
    0xf9, 0x5b,
    0xfa, 0x15,   //��3
    0xfa, 0x67,
    0xfb, 0x04,   //��5
    0xfb, 0x90,
    0xfc, 0x0c,   //��7
    0xfc, 0x44,   //����C��
    0xfc, 0xac,   //��2
    0xfd, 0x09,
    0xfd, 0x34,   //��4
    0xfd, 0x82,
    0xfd, 0xc8,   //��6
    0xfe, 0x06,
    0xfe, 0x22,   //�߰˶ȣ���1
    0xfe, 0x56,
    0xfe, 0x6e,   //��3
    0xfe, 0x9a,
    0xfe, 0xc1,   //��5
    0xfe, 0xe4,
    0xff, 0x03    //��7
};

/*---------����2��ʼ���������ʷ�������---------*/
void Uart2Init( void )
{
    S2CON = 0x10 ;      //������У��λ�������п�2���գ����п�2�Ŀ��ƼĴ���
    T2L = cstT2HL ;     //���ò�������װֵ
    T2H = cstT2HL >> 8 ;
    AUXR |= 0x14 ;      //T2Ϊ1Tģʽ����������ʱ��2
}
//��ʼ������
void refreshChessBoard(){
	//��ʼ������
	for(i=1;i<4;i++){
		for(j=0;j<4;j++)
			Board[i][j]=0;
	}
	
	//��ʼ�������Ϣ
	for(i=1;i<4;i++){
		Board[i][4]=myPlayer;						//��ʾ�ҵ����
		Board[i][5]=myPlayer^0x01;					//��ʾ�Է����		
		Board[i][0]=i;								//�к�
	}
	
	//��ʼ��С����
	if(myPlayer==0x0b) 
		point=5;
	else
		point=4;

	//��ʼ�����
	x=2;
	y=2;
}
//ˢ�±ȷְ�
void refreshBoard(){
	scoreBoard[3]=myPlayer;			//�����ʾ�����ҵĵ÷�
	scoreBoard[4]=myPlayer^0x01;

	scoreBoard[1]=myScore;			//���±ȷ�
	scoreBoard[7]=otherScore;
}
void init(){
	P2M1=0x00;//P2����	
	P2M0=0xff;
	
	P0M1=0x00;//P0����
	P0M0=0xff;

	P3M0 = 0x10;		//P3.4����
	sbtLedSel = 0;      //ѡ���������Ϊ���
	P1ASF = 0x80;       //P1.7��Ϊģ�⹦��A/Dʹ��
	ADC_RES = 0;        //ת���������
	ADC_CONTR = 0x8F;   //cstAdcPower = 1
	CLK_DIV = 0X00;     //ADRJ = 0    ADC_RES��Ÿ߰�λ���
	btBeepFlag = 0;			//��������ʼ��Ϊ0
	
	
	x=2;								//�����ʼ��Ϊ��2,2��
	y=2;
	digIndex = 0;				//��ʼ���±�͸�������ֵ
	winner=0;
	start=0;						//��ʼ������	
	over=0;
	myScore=0;
	myScore=0;
	count = 0;				//��������
	myPlayer=0x0a;
	datas=0;				//���ݳ�ʼ��
	gameNum=0;

	//��ʱ��T0
	TMOD=0x00;						//16λ����װģʽ
	ET0=1;								//������ʱ��T0�ж�
	TH0=(65535-1000)/256;	//��ʱ1ms
	TL0=(65535-1000)%256;
	TR0=1;								//T0������ʱ	
	
	//��ʱ��T1
	TH1 = 0xD8;
	TL1 = 0xEF;
	ET1 = 1;
	TR1 = 0;						//��ʱ�����ڹر�״̬�����������ֵ�ʱ�򣬽��Ὺ��
	EA=1;
	
	//485��ʼ��  				����������
	sbtM485_TRN = 0 ;   //��ʼΪ����״̬
	P_SW2 |= 0x01 ;     //�л�����2�Ĺܽŵ�P4.6,P4.7
	Uart2Init() ;
	btSendBusy = 1 ;
	IE2 |= 0x01 ;       //�����п�2�ж�
	IP2 |= 0x01 ;       //���ô��п��жϣ������ȼ�
	
	datas=0;						//��ʼ����������Ϊ0
}
/*---------��ʱ5ms�Ӻ���--------*/
void Delay5ms()     //@11.0592MHz  ��ʱ5ms
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
/*---------��ʱ100ms�Ӻ���--------*/
void Delay100ms()       //@11.0592MHz  ��ʱ100ms
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
/*---------��ʱ2000ms------------*/
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

/*---------��ʱ�Ӻ���---------*/
void DelayMs( unsigned int xms )
{
    uint i, j;
    for( i = xms; i > 0; i-- )
        for( j = 124; j > 0; j-- );
}
/*---------��ȡADֵ�Ӻ���--------*/
unsigned char GetADC()
{
    uchar ucAdcRes;
    ADC_CONTR = cstAdcPower | cstAdcStart | cstAdcSpeed90 | cstAdcChs17;//û�н�cstAdcFlag��1�������ж�A/D�Ƿ����
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    while( !( ADC_CONTR & cstAdcFlag ) ); //�ȴ�ֱ��A/Dת������
    ADC_CONTR &= ~cstAdcFlag;           	//cstAdcFlagE�����0
    ucAdcRes = ADC_RES;                 	//��ȡAD��ֵ
    return ucAdcRes;											//����ת��ֵ
}

/*---------��ȡ��������ֵ�Ӻ���--------*/
uchar NavKeyCheck()
{
    unsigned char key;
    key = GetADC();     //��ȡAD��ֵ
    if( key != 255 )    //�а�������ʱ
    {
        Delay5ms();
        key = GetADC();
        if( key != 255 )            //�������� ���а�������
        {
            key = key & 0xE0;       //��ȡ��3λ������λ����
            key = _cror_( key, 5 ); //ѭ������5λ ��ȡA/Dת������λֵ����С���
            return key;
        }
    }
    return 0x07;        //û�а�������ʱ����ֵ0x07
}

/*------------������Ч---------------*/
void tick1(){			//��	
		if(curPlayer==0x0b) TH0=(65535-500)/256;	//���B���ӵ�ʱ��ı�����
		btBeepFlag=1;
		Delay100ms();	//��ʱ100ms
		btBeepFlag=0;	//ֹͣ������
		TH0=(65535-1000)/256;	//��ʱ1ms
}
void tick2(){			//�ε�
		if(curPlayer==0x0b) TH0=(65535-500)/256;	//���B���ӵ�ʱ��ı�����
		btBeepFlag=1;
		Delay50ms();	//��ʱ100ms
		btBeepFlag=0;	//ֹͣ������	
		Delay30ms();
		btBeepFlag=1;
		Delay50ms();	//��ʱ100ms
		btBeepFlag=0;	//ֹͣ������
		TH0=(65535-1000)/256;	//��ʱ1ms
}
void tick3(){			//��ʼ��Ч
		TH0=(65535-200)/256;
		btBeepFlag=1;
		Delay50ms();	//��ʱ100ms
		btBeepFlag=0;	//ֹͣ������	
		Delay30ms();
		btBeepFlag=1;
		Delay50ms();	//��ʱ100ms
		btBeepFlag=0;	//ֹͣ������
		TH0=(65535-1000)/256;	//��ʱ1ms
}
void tick4(){			//��	
		btBeepFlag=1;
		Delay5ms();		//��ʱ100ms
		btBeepFlag=0;	//ֹͣ������
		TH0=(65535-1000)/256;	//��ʱ1ms
}
/*-----���ݷ��ͺ���----*/
void sendDatas()
{
    sbtM485_TRN = 1 ;					//MAX485ʹ�����ţ�����485����
    S2BUF = datas | (myPlayer<<7);		//����������д�뻺����,������Դ��,A���������0��ͷ��B�����1��ͷ��1010��1011
    while( btSendBusy );				//�ȴ����ݷ������
    btSendBusy = 1 ;					//�ٴ�����Ϊ1����ʾ����æ
    sbtM485_TRN = 0 ;					//�ر�485����
	datas=0;
}
/*---------ȡַ�Ӻ���---------*/
//ȡ��tem������arrMusicToTimerNum�����е�λ��ֵ
uchar GetPosition( uchar tem ) 
{
    uchar ucBase, ucOffset, ucPosition;     //����������������λ��
    ucBase = tem / 16;            //��4λ������ֵ,��ַ
    ucOffset = tem % 16;          //��4λ��������ƫ����
    if( ucBase == 1 )              //������ֵΪ1ʱ�����ǵͰ˶ȣ���ַΪ0
        ucBase = 0;
    else if( ucBase == 2 )          //������ֵΪ2ʱ�������а˶ȣ���ַΪ14
        ucBase = 14;
    else if( ucBase == 3 )          //������ֵΪ3ʱ�����Ǹ߰˶ȣ���ַΪ28
        ucBase = 28;
    //ͨ����ַ����ƫ���������ɶ�λ��������arrMusicToTimerNum�����е�λ��
ucPosition = ucBase + ( ucOffset - 1 ) * 2; 
    return ucPosition;            //������һ��λ��ֵ
}

/*---------�������ֹ��ܺ���---------*/
void PlayMusic(uchar time,uchar arrMusic[])
{
    uchar ucNoteTmp, ucRhythmTmp, tem; 	// ucNoteTmpΪ������ucRhythmTmpΪ����
    uchar i = 0;
		ucNoteTmp = arrMusic[0];    					//�������������,��ʱ1��,�ص���ʼ����һ��
    while( ucNoteTmp != 0x00 && ((!start)||over))	//������֮ǰ||��Ϸ����֮��ſ��Բ�������
    {
        ucNoteTmp = arrMusic[i];    //�������������,��ʱ1��,�ص���ʼ����һ��
        if( ucNoteTmp == 0xff )  //��������ֹ��,��ʱ100ms,����ȡ��һ����
        {
            i = i + 2;
            DelayMs( 100 );
            TR1 = 0;
        }
        else                     //���������ȡ�����ͽ���
        {
            //ȡ����ǰ������arrMusicToTimerNum�����е�λ��ֵ
						tem = GetPosition( arrMusic[i] );              
						//��������Ӧ�ļ�ʱ����װ��ֵ����ucTimerH��ucTimerL
						ucTimerH = arrMusicToTimerNum[tem];  
            ucTimerL = arrMusicToTimerNum[tem + 1];
            i++;
            TH1 = ucTimerH;           //��ucTimerH��ucTimerL�����ʱ��
            TL1 = ucTimerL;
            ucRhythmTmp = arrMusic[i];      //ȡ�ý���
            i++;
        }
        TR1 = 1;                          //����ʱ��1
        DelayMs( ucRhythmTmp * time );    //�ȴ��������, ͨ��P3^4�������Ƶ
        TR1 = 0;                          //�ض�ʱ��1
    }
}
//LED�ƹ�
void displayLED(){
	led=0x01;
	sbtLedSel=1;						//����LED��ʾ
	j=0;
	for(i=0;i<40;i++){				
		if(led==0x80){
			j=!j;
		}
		else if(led==0x01){
			j=!j;
		}

		if(j)						//�������ƶ�
			led=led<<1;
		else
			led=led>>1;
		P0=led;
		DelayMs(200);
	}
	sbtLedSel=0;
}
/*-------------�ж���Ӯ-------------*/
uchar isOver(){				
    //����һ���ֳ�ʤ�������
	for(i=1;i<4;i++){
        if(Board[i][1]!=0&&Board[i][1]==Board[i][2]&&Board[i][1]==Board[i][3]) return Board[i][1];	//��
        if(Board[1][i]!=0&&Board[1][i]==Board[2][i]&&Board[1][i]==Board[3][i]) return Board[1][i];	//��
    }	
    if(Board[1][1]!=0&&Board[1][1]==Board[2][2]&&Board[1][1]==Board[3][3]) return Board[1][1];		//��б�Խ���
    if(Board[1][3]!=0&&Board[1][3]==Board[2][2]&&Board[2][2]==Board[3][1]) return Board[1][3];		//��б�Խ���
    
    //���̿�
    for(i=1;i<4;i++)
        for(j=1;j<4;j++)
            if(Board[i][j]==0)	//���ڿ�λ��˵����û��������
                return 0;
    //���һ��������������ˣ�ƽ��
    return 0x10;				
}
void displayChessBoard(){				//չʾ����
	P0=0;	
	blinkIndex=y;						//������˸							
	if(digIndex==y)						//�����ǰλѡλ����˸λһ�£�˵����Ҫ������˸���� 
	{
		if(count>20){					//����,����
			if(count==50) count=0;
		}
		else							//Ϩ��
			digIndex++;		
	}
	if (digIndex==point && start) 				//���
		P0 = arrSegSelect[Board[x][digIndex]] | 0x80;
	else
		P0 = arrSegSelect[Board[x][digIndex]];
	P2=dig[digIndex];					//λѡ	
}				
void displayScore(){					//��ʾ�ȷ�
	P0=0;
	P2=digIndex;						//λѡ
	P0 = arrSegSelect[scoreBoard[digIndex]];		//��ѡ
}
//����ܹ�����ʾ
void displayRoll(uchar arr[]){
	//����һ��
	if(count>20){	
		count=0;										//��Ƶ����
		j=arr[0];
		for(i=0;i<12;i++) arr[i]=arr[i+1];
		arr[12]=j;
	}
	P0=0;
	P2=digIndex;										//λѡ
	P0 = arr[digIndex];									//��ѡ
}
/*---------��Ϸ��ʼ----------------*/
void gameStart(){
	PlayMusic(190,arrMusicBM);					//���ſ�������
	while(!start);								//�ȴ���Ϸ��ʼ
	Delay5ms();
	tick3();
	
	//��˸�����
	//���¶�ʱΪ45ms
	TH0 = ( 65535 - 45000 ) / 256; 				//��ʱ��0�ĸ߰�λ����
	TL0 = ( 65535 - 45000 ) % 256; 				//��ʱ��0�ĵͰ�λ���ã���������������ö�ʱ��0�ĳ�ʼֵ��1ms

	DelayMs(4000);

	//���¶�ʱΪ45ms
	TH0 = ( 65535 - 1000 ) / 256; 				//��ʱ��0�ĸ߰�λ����
	TL0 = ( 65535 - 1000 ) % 256; 				//��ʱ��0�ĵͰ�λ���ã���������������ö�ʱ��0�ĳ�ʼֵ��1ms		

	//��������
	refreshChessBoard();
}
/*-------------��Ϸ��������--------------*/
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
		refreshBoard();					//ˢ�±ȷ�
			
		//���¶�ʱΪ45ms
		TH0 = ( 65535 - 45000 ) / 256; //��ʱ��0�ĸ߰�λ����
		TL0 = ( 65535 - 45000 ) % 256; //��ʱ��0�ĵͰ�λ���ã���������������ö�ʱ��0�ĳ�ʼֵ��1ms
		Delay2000ms();
		//���¶�ʱΪ45ms
		TH0 = ( 65535 - 1000 ) / 256; //��ʱ��0�ĸ߰�λ����
		TL0 = ( 65535 - 1000 ) % 256; //��ʱ��0�ĵͰ�λ���ã���������������ö�ʱ��0�ĳ�ʼֵ��1ms	
		
		//��ʼ�µ�һ����Ϸ
		gameNum++;						//��������
		curPlayer=0x0a;					//���¶�λ��ǰ���
		winner=0;
		start=0;
		over=0;
		refreshChessBoard();			//ˢ�³�ʼ������

		if(gameNum!=3){
			gameStart();
		}
		else							//��Ϸ����
		{
			PlayMusic(200,arrMusicBM);	//���ű�������
			start=1;
			over=1;			
		}						
}
/*----------���Ӻ���------------*/
void playChess(){
	tick2();
	if(over) return;											//��Ϸ�Ѿ����������޷�����
	Board[x][y]=curPlayer;										//����
	curPlayer^=0x01;											//0000 1010  ! 0000 1011ȡ�����һλ������curPlayer										
	over=isOver();												//�ж���Ӯ
	if(over) {
		winner=over;											//��ȡ��ϷӮ��
	}
	else{
		if(curPlayer==myPlayer)									//�����˸
			point=4;									
		else
			point=5;											//�Ҳ������˸
	}
}

/*---------�������������Ӻ���--------*/
void NavKey_Process()
{
    uchar ucNavKeyCurrent;  						//����������ǰ��״̬
    uchar ucNavKeyPast;     						//��������ǰһ��״̬

    ucNavKeyCurrent = NavKeyCheck();    //��ȡ��ǰADCֵ
    if( ucNavKeyCurrent != 0x07 )       //���������Ƿ񱻰��� ������0x07��ʾ�а���
    {
        ucNavKeyPast = ucNavKeyCurrent;
        while( ucNavKeyCurrent != 0x07 )        //�ȴ����������ɿ�
            ucNavKeyCurrent = NavKeyCheck();
        switch( ucNavKeyPast )
        {
			case 0x01:						//��
				if(y!=3){
					y++;
					tick1();
				}
				break;
			case 0x02:						//��
				if(x!=3){
					x++;
					tick1();
				}
				break;
			case 0x03:						//����
				if(over) return;			//��Ϸ���ڽ���״̬����ֱ�ӷ���
				if(Board[x][y]==0)
					playChess();			//����	
				break;
			case 0x04:
				if(y!=1){
					y--;
					tick1();
				}
				break;							//��
			case 0x05:						//��
				if(x!=1){
					x--;
					tick1();
				}
				break;
        }
		//�ȷ��������ݣ��ٽ�����Ϸ
		datas=(1<<(ucNavKeyPast+1));				//���룬���÷�������
		sendDatas();											
    }
	if(over&&start&&gameNum<3){							
		gameOver();
	}	
    Delay100ms();
}
void selectCharacter(){
	myPlayer^=0x01;									//������ɫ��Ϣ
	for(i=0;i<4;i++){
		Board[i][4]=myPlayer;
		Board[i][5]=myPlayer^0x01;		//�ڶ������
	}
	for(i=1;i<4;i++){							//�������̵�2��
		Board[2][i]=myPlayer;					//��ʾѡ���ɫ����Ϣ
	}
	Board[2][0]=21;								//��ʾΪ0
	Board[2][4]=21;								//��ʾ�ڶ������
	Board[2][5]=21;								//��ʾ�ڶ��е����
}

//���ݽ�������
void getData()
{
	isSelect=1;
	if(!((datas>>7)^(myPlayer&0x01))){							//����־��ͬ��ʱ��ȷ����Ϣ
		if(start) return;										//����������������Լ�����ô�ܾ�����
		else{
			myPlayer^=0x01;										//��򣬱�����һ���
		}
	}		
	datas =datas&0x7f;											//ȥ�����λ����Դ��Ϣ��0111 1111
	if(datas)													//������ݲ�Ϊ��
	{
		//���key1��Ϸ��ʼ����
		if(datas&0x01){
			start=1;
			//tick4();
		}
		//key2û�з������ı�Ҫ����Ϊ˫��û��Ȩ��֪���Է��Ƿ�鿴�ȷְ�,
		//����Key2�ڽ�ɫѡ���з���������,���ʱ��ͬ������
		if(datas&0x02){
			for(i=0;i<4;i++){
				Board[i][4]=myPlayer;
				Board[i][5]=myPlayer^0x01;				//�ڶ������
			}
			for(i=1;i<4;i++){							//�������̵�2��
				Board[2][i]=myPlayer;					//��ʾѡ���ɫ����Ϣ
			}
			Board[2][0]=21;								//��ʾΪ0
			Board[2][4]=21;								//��ʾ�ڶ������
			Board[2][5]=21;								//��ʾ�ڶ��е����
		}
		datas>>=2;
		//����������
		switch(datas)						
		{
			case 0x01:						//��
				if(y!=3){
					y++;
					tick1();
				}
				break;
			case 0x02:						//��
				if(x!=3){
					x++;
					tick1();
				}
				break;
			case 0x04:						//����
				//if(over) return;			//��Ϸ���ڽ���״̬����ֱ�ӷ���
				if(Board[x][y]==0)
					playChess();			//����								
				break;
			case 0x08:
				if(y!=1){
					y--;
					tick1();
				}
				break;							//��
			case 0x10:						//��
				if(x!=1){
					x--;
					tick1();
				}
				break;
		}	
		datas=0;	//������ݷ��ͽ��գ���ִֻ֤��һ��
	}
}
/*---------����2�жϴ���������ݽ���---------*/
void Uart2_Process( void ) interrupt 8 using 1
{
    if( S2CON & cstUart2Ri )					//��У��&�����ж������־λ
    {
        datas = S2BUF ;								//�Ӵ����н��������ݴ�
        S2CON &= ~cstUart2Ri;   			//�����жϱ�־λ��0
		getData();
    }
    if( S2CON & cstUart2Ti )					//��У��&�����ж������־λ
    {
        btSendBusy = 0 ;            	//���æ�ź�
        S2CON &= ~cstUart2Ti ;      	//�����жϱ�־λ��0
    }
}

/*---------��ʱ��0�жϴ�����---------*/
void T0_Process() interrupt 1				//�ж�
{
	//��ⰴ��Key1��Ϸ��ʼ
	if(sbtKey1==0 && (!start) && (!over))						//�������²���start=0�����Ѿ�ѡ������Ϸ��ҽ�ɫ���������ǰ��ʼ��ʼ
	{
		tick4();
		if(sbtKey1==0){
			while( !sbtKey1 );
			start=1;											//key1���£���Ϸ��ʼ
			datas=0x01;											//����Key1���ݣ���ס�������Ϣ��sendData�����
			sendDatas();										//��������				
		}
	}

	//ѡ���ɫ��Ϣ
	if( sbtKey2 == 0 && gameNum==0 && (!start) && (!over))		//����Ϸδ��ʼ������Ϸ����Ϊ0��ʱ��Key2����ѡ�����
	{
		Delay5ms();                 							//��ʱ����
		tick4();
		if( sbtKey2 == 0 )
		{
			while( !sbtKey2 );									//�ȴ�K1�ſ�
			selectCharacter();
			isSelect=1;
			datas=0x02;											//����key2
			sendDatas();										//ֱ�ӷ�������
		}
	}

	/*****��ʾ����*****/
	digIndex++;													//����ܵ������±�
	if(digIndex==8){
		digIndex=0;												//�±����
		count++;											
	}
	if(sbtKey2 == 0 && (start||over||gameNum))					//��ʼ���߽�����ʱ����Բ鿴����									 
		displayScore();											//3�ֿ�ʼ�󣬱����£���ʾ�ȷְ�	
	else if((!start&&!over&&gameNum==0&&!isSelect)||(start&&over))			//δ��ʼ�����Ѿ�ȫ�ֽ���
		displayRoll(arrRoll);									//��ʾ������ĸ			 
	else 
		displayChessBoard();									//��ʾ����
	
	//��������������
	if(btBeepFlag && (!over))
		sbtBeep = ~sbtBeep;            							//��������ʹ�÷���������
	else if(TR1==0)
		sbtBeep = 0;                  							//TR1=0��ʱ��˵��û�в������֣�ֹͣ����������sbtBeep�˿����ڵ͵�ƽ	
}
/*---------��ʱ��1�жϴ�����---------*/
//����װֵ������sbtBeepֵȡ������������
void T1_Process() interrupt 3                     //��ʱ������Ƶ��
{
    sbtBeep = ~sbtBeep;
}
void main(){
		init();
		gameStart();									//��Ϸ����ʼ
		while(1){
			NavKey_Process();
		}
}