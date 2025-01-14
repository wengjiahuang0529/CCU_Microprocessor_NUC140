/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 15/04/10 2:05p $
 * @brief    Change duty cycle and period of output waveform by PWM Double Buffer function.
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "NUC100Series.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/

#define PLLCON_SETTING      CLK_PLLCON_50MHz_HXT
#define PLL_CLOCK           50000000

#define RXBUFSIZE 1024 
uint8_t g_u8RecData[RXBUFSIZE]  = {0};
uint8_t input[8];

volatile uint32_t g_u32comRbytes = 0;
volatile uint32_t g_u32comRhead  = 0;
volatile uint32_t g_u32comRtail  = 0;
volatile int32_t g_bWait         = TRUE;


/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/

/**
 * @brief       PWMA IRQ Handler
 *
 * @param       None
 *
 * @return      None
 *
 * @details     ISR to handle PWMA interrupt event
 */
static int toggle = 0;
static int toggle_2 = 0;

int angle[2];//�PQ?n??��o???�� (degree)
int arr[8];
void PWMA_IRQHandler(void)
{
		//static int toggle = 0;
		printf("interruptA \n");
    // Update PWMA channel 0 period and duty
   if(toggle==0) //43*20ms ???n?i?H??????180 degree     //if((toggle > 12) && (toggle < 55))
    {		
				//PWM_ConfigOutputChannel(PWMA, PWM_CH0, 50, 7);
        PWM_SET_CNR(PWMA, PWM_CH0, 30000-1);	//i chose the clock 1.5M   ; 1.5Mhz/50hz =30k        
        PWM_SET_CMR(PWMA, PWM_CH0, ((100*(angle[0]+36))/3) - 1);			//  (30000*(((angle[0]+36)/18)/50) - 1)        2/50  1200    
				//printf("%d\n", ((100*(angle[0]+36))/3) - 1);
    }
    else if(toggle==1)																								//else if(toggle >55)
    {
				//PWM_ConfigOutputChannel(PWMA, PWM_CH0, 50, 12);
        PWM_SET_CNR(PWMA, PWM_CH0, 30000-1);
				PWM_SET_CMR(PWMA, PWM_CH0, ((100*(angle[1]+36))/3) - 1-1);			//	180degree : duty cycle=7/50  cnr=30000 cmr=4200
				//printf("%d\n", ((100*(angle[1]+36))/3) - 1);
    }
		//printf("toggle= %d\n",toggle);
    // Clear channel 0 period interrupt flag
    PWM_ClearPeriodIntFlag(PWMA, 0);
}

void PWMB_IRQHandler(void)
{
		//static int toggle = 0;
		printf("interruptB \n");
    // Update PWMA channel 1 period and duty
   if(toggle_2==0) //43*20ms ???n?i?H??????180 degree     //if((toggle > 12) && (toggle < 55))
    {		
				//PWM_ConfigOutputChannel(PWMA, PWM_CH0, 50, 7);
        PWM_SET_CNR(PWMB, PWM_CH1, 30000-1);	//i chose the clock 1.5M   ; 1.5Mhz/50hz =30k        
        PWM_SET_CMR(PWMB, PWM_CH1, ((100*(angle[0]+36))/3) - 1);			//  (30000*(((angle[0]+36)/18)/50) - 1)        2/50  1200    
				//printf("%d\n", ((100*(angle[0]+36))/3) - 1);
    }
    else if(toggle_2==1)																								//else if(toggle >55)
    {
				//PWM_ConfigOutputChannel(PWMA, PWM_CH0, 50, 12);
        PWM_SET_CNR(PWMB, PWM_CH1, 30000-1);
				PWM_SET_CMR(PWMB, PWM_CH1, ((100*(angle[1]+36))/3) - 1-1);			//	180degree : duty cycle=7/50  cnr=30000 cmr=4200
				//printf("%d\n", ((100*(angle[1]+36))/3) - 1);
    }
		//printf("toggle= %d\n",toggle);
    // Clear channel 1 period interrupt flag
    PWM_ClearPeriodIntFlag(PWMB, 1);
}

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable Internal RC clock */
    CLK_EnableXtalRC(CLK_PWRCON_OSC22M_EN_Msk);

    /* Waiting for IRC22M clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_OSC22M_STB_Msk);

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_CLKDIV_HCLK(1));

    /* Enable external 12MHz XTAL, internal 22.1184MHz */
    CLK_EnableXtalRC(CLK_PWRCON_XTL12M_EN_Msk | CLK_PWRCON_OSC22M_EN_Msk);

    /* Enable PLL and Set PLL frequency */
    CLK_SetCoreClock(PLL_CLOCK);

    /* Waiting for clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_PLL_STB_Msk | CLK_CLKSTATUS_XTL12M_STB_Msk | CLK_CLKSTATUS_OSC22M_STB_Msk);

    /* Switch HCLK clock source to PLL, STCLK to HCLK/2 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_PLL, CLK_CLKDIV_HCLK(2));

    /* Enable UART module clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Enable PWM module clock */
    CLK_EnableModuleClock(PWM01_MODULE);
    //CLK_EnableModuleClock(PWM23_MODULE);
    CLK_EnableModuleClock(PWM45_MODULE);
    //CLK_EnableModuleClock(PWM67_MODULE);
		
    /* Select UART module clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART_S_HXT, CLK_CLKDIV_UART(1));

    /* Select PWM module clock source */
    CLK_SetModuleClock(PWM01_MODULE, CLK_CLKSEL1_PWM23_S_HXT, 7);			// external 12MHz XTAL 12M/8=1.5M
    //CLK_SetModuleClock(PWM23_MODULE, CLK_CLKSEL1_PWM23_S_HXT, 0);
		CLK_SetModuleClock(PWM45_MODULE, CLK_CLKSEL1_PWM23_S_HXT, 7);

    /* Reset PWMA channel0~channel3 */
    SYS_ResetModule(PWM03_RST);
		/* Reset PWMB channel4~channel7 */
    SYS_ResetModule(PWM47_RST);
		
    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
    //SystemCoreClockUpdate();
    PllClock        = PLL_CLOCK;            // PLL
    SystemCoreClock = PLL_CLOCK / 1;        // HCLK
    CyclesPerUs     = PLL_CLOCK / 1000000;  // For SYS_SysTickDelay()

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set GPB multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFP &= ~(SYS_GPB_MFP_PB0_Msk | SYS_GPB_MFP_PB1_Msk);
    SYS->GPB_MFP |= (SYS_GPB_MFP_PB0_UART0_RXD | SYS_GPB_MFP_PB1_UART0_TXD);
		
    /* Set GPA multi-function pins for PWMA Channel0 */
    SYS->GPA_MFP = SYS_GPA_MFP_PA12_PWM0;
		SYS->GPE_MFP = SYS_GPE_MFP_PE5_PWM5;
}

void UART0_Init()
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset IP */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 Baudrate */
    UART_Open(UART0, 115200);
}

void START_End_SG90()
{
	
}	

void BITS_Transfer(int send)
{
		int i=0,j=0;
		printf("send:%c (%d)\n",send,send);
		for(j=0;j<8;j++)
		{
			arr[j]=-1;
		}
		while(send)
		{
			arr[i]=send%2;
			send /= 2;
			i++;
		}
		while(i<=7 && arr[i]==-1)
		{
			arr[i]=0;
			//printf("arr[%d]:%d\n",i,arr[i]);
			i++;
		}		
		printf("ASCII_Binary:");
			for(j=7;j>=0;j--)
			{
				printf("%d",arr[j]);
				if(arr[j]==0){
					//??�X????L?????w???m
				}
				else if(arr[j]==1){
					//??�X????L?????w???m
				}
			}
		printf("\n");
}


void UART_Transfer_HANDLE()
{
		int i;
    uint8_t u8InChar = 0xFF;
    uint32_t u32IntSts = UART0->ISR; //ask
		uint8_t *txt=g_u8RecData; 
						
    if(u32IntSts & UART_ISR_RDA_INT_Msk) //
    {
				u8InChar = UART_READ(UART0);
				if(u8InChar==0X0D){
						g_u8RecData[g_u32comRtail]='\0'; //end of string
						printf("%s\n",(char*)txt);
								
						START_End_SG90(); //??�X?�ӡ�???B?��??�X??F ?N???}?l??
					
						for(i=0;i<g_u32comRtail;i++)
						{						
								BITS_Transfer(g_u8RecData[i]);
						}
						
						START_End_SG90(); //??�X?�ӡ�???B?��??�X??F ?N????��?
						
						//reset
						g_u8RecData[0]=0;
						g_u32comRbytes=0;
						g_u32comRtail=0;
						g_bWait = FALSE;
				}					
				else if(g_u32comRbytes < RXBUFSIZE)
        {
            g_u8RecData[g_u32comRtail] = u8InChar; //put the char in g_u8RecData
            g_u32comRtail = (g_u32comRtail == (RXBUFSIZE - 1)) ? 0 : (g_u32comRtail + 1); //check oversize
            g_u32comRbytes++;
        }
		}
}

void UART02_IRQHandler(void) 
{
    UART_Transfer_HANDLE();
}


/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/

int32_t main(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();
    /* Init System, IP clock and multi-function I/O */
    SYS_Init();
    /* Lock protected registers */
    SYS_LockReg();
    /* Init UART0 for printf */
    UART0_Init();
/* =========================================================*/
	   /* set PWMA channel 0 output configuration */
    PWM_ConfigOutputChannel(PWMA, PWM_CH0, 50, 12);//7.5% -> 1.5ms    2/7/12
		PWM_ConfigOutputChannel(PWMB, PWM_CH1, 50, 12);
    /* Enable PWM Output path for PWMA channel 0 */
    PWM_EnableOutput(PWMA, 0x1);  // 0001
		PWM_EnableOutput(PWMB, 0x2);
		
    // Enable PWM channel 0 period interrupt
    PWMA->PIER = PWM_PIER_PWMIE0_Msk;
		PWMB->PIER = PWM_PIER_PWMIE1_Msk ;
		
    NVIC_EnableIRQ(PWMA_IRQn);
		NVIC_EnableIRQ(PWMB_IRQn);

    printf("+------------------------------------------------------------------------+\n");
    printf("|                          Backscatter                                   |\n");
    printf("|                                                                        |\n");
    printf("+------------------------------------------------------------------------+\n");
		
		while(1){
			int j;
			printf("Input the word:");
			UART_EnableInt(UART0, ( UART_IER_RDA_IEN_Msk | UART_IER_THRE_IEN_Msk | UART_IER_TOUT_IEN_Msk));
			while(g_bWait);

			
			g_bWait=TRUE;
		}

    // =========== Start ================
		angle[0]=0;
		angle[1]=180;
    PWM_Start(PWMA, 0x1); 	//0001
		PWM_Start(PWMB, 0x2); 	//0010

    while(1){
				CLK_SysTickDelay(700000);
				CLK_SysTickDelay(180000);
				toggle=0;
				toggle_2=0;
				CLK_SysTickDelay(700000);
				CLK_SysTickDelay(180000);
				toggle=1;
				toggle_2=1;
		};

}
