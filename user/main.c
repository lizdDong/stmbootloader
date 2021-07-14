/**
  ******************************************************************************
  * @file    main.c
  * @author  lizdDong
  * @version V1.0
  * @date    2021-4-13
  * @brief   V1.0 2021-4-13 使用Y-modem协议通过串口进行升级。
  *          V1.1 2021-6-18 增加从flash升级功能，读取标志位，有效则从flash指定
  *                         区域拷贝升级文件到运行区。
  * @attention
  *
  ******************************************************************************
  */

#include "stdio.h"
#include "stm32f10x.h"
#include "ymodem.h"
#include "iap_cfg.h"
#include "dev_flash.h"


uint8_t gaRecvData[1024] = {0};
uint8_t gaFlashTemp[2048];
__IO uint32_t gMsCounter = 0;

static void uart_init(void);
static void io_init(void);
static void systick_init(void);
static int32_t app_run(void);
static void flash_copy(uint32_t destination, uint32_t source, uint32_t size);
#if (USE_RS485_PORT)
static void RS485_InitTXE(void);
#endif

static void uart_deinit(void);
static void systick_deinit(void);
static void io_deinit(void);



/**
 ****************************************************************************
 * @brief  None
 * @author lizdDong
 * @note   None
 * @param  None
 * @retval None
 ****************************************************************************
*/
void init_all(void)
{
    io_init();  // 主要目的是将JTAG的2个端口PA15和PB4输出为低（默认配置下为高）
    uart_init();
    systick_init();
}

/**
 ****************************************************************************
 * @brief  None
 * @author lizdDong
 * @note   None
 * @param  None
 * @retval None
 ****************************************************************************
*/
void deinit_all(void)
{
    systick_deinit();
    uart_deinit();
    io_deinit();
}

/**
 ****************************************************************************
 * @brief  None
 * @author lizdDong
 * @note   None
 * @param  None
 * @retval None
 ****************************************************************************
*/
void print_msg(void)
{
    printf("\r\n");
    printf("============ STM Bootloader =============\r\n");
    printf(" Author:    lizdDong                     \r\n");
    printf(" Revision:  V1.1 @2021-7-1               \r\n");
    printf(" Application address: 0x%08X             \r\n", ApplicationAddress);
    printf(" Image       address: 0x%08X             \r\n", IAP_IMAGE_ADDR);
    printf(" Auto run application after %d(s).       \r\n", RUN_APP_DELAY_S);
    printf(" Key <F1>  run application.              \r\n");
    printf(" Key <F2>  upgrede via Ymodem.           \r\n");
    printf(" Key <F3>  forced to upgrede from image! \r\n");
    printf("=========================================\r\n");
    gMsCounter = 0;
}

/**
 ****************************************************************************
 * @brief  None
 * @author lizdDong
 * @note   None
 * @param  None
 * @retval None
 ****************************************************************************
*/
void run_app_failed(void)
{
    printf("Run application failed.\r\n");
}

/**
 ****************************************************************************
 * @brief  None
 * @author lizdDong
 * @note   None
 * @param  None
 * @retval None
 ****************************************************************************
*/
int main(void)
{
    uint8_t c, step = 0, get_key_f1 = 0, get_key_f2 = 0, get_key_f3 = 0;
    uint16_t iap_flag;
    
    init_all();
    print_msg();
    while(1)
    {
        if(Receive_Byte(&c, 10) == 0)
        {
            switch(step)
            {
                case 0:
                    if(c == 0x1B)
                        step++;
                    break;
                case 1:
                    if(c == 0x4F)
                        step++;
                    else
                        step = 0;
                    break;
                case 2:
                    if(c == 0x50)   // key <F1>
                        get_key_f1 = 1;
                    if(c == 0x51)   // key <F2>
                        get_key_f2 = 1;
                    if(c == 0x52)   // key <F3>
                        get_key_f3 = 1;
                    step = 0;
                    break;
                default:
                    break;
            }
            if(get_key_f1)
            {
                get_key_f1 = 0;
                if(app_run() < 0)
                {
                    run_app_failed();
                    print_msg();
                }
            }

            if(get_key_f2)
            {
                get_key_f2 = 0;
                printf(" Waiting upgrade via Ymodem, key <a> to abort.\r\n");
                if(Ymodem_Receive(gaRecvData) > 0)
                {
                    if(app_run() < 0)
                    {
                        run_app_failed();
                        print_msg();
                    }
                }
                else
                {
                    printf("\r\n Ymodem receive failed.\r\n");
                    print_msg();
                }
            }

            if(get_key_f3)
            {
#if (UPGRADE_FROM_IMAGE)

                get_key_f3 = 0;
                printf("Upgrade from image ...\r\n");
                printf("Image address: 0x%08X\r\n", IAP_IMAGE_ADDR);
                flash_copy(IAP_APP_ADDR, IAP_IMAGE_ADDR, IAP_APP_SIZE);
                iap_flag = 0xFFFF;
                dev_flashWrite(IAP_FLAG_ADDR, (uint8_t *)&iap_flag, 2);

#endif
            }
        }

        if(gMsCounter > 1000 * RUN_APP_DELAY_S)
        {
#if (UPGRADE_FROM_IMAGE)

            dev_flashRead(IAP_FLAG_ADDR, (uint8_t *)&iap_flag, 2);
            if(iap_flag == IAP_FLAG)
            {
                printf("Upgrade from image ...\r\n");
                printf("Image address: 0x%08X\r\n", IAP_IMAGE_ADDR);
                flash_copy(IAP_APP_ADDR, IAP_IMAGE_ADDR, IAP_APP_SIZE);
                iap_flag = 0xFFFF;
                dev_flashWrite(IAP_FLAG_ADDR, (uint8_t *)&iap_flag, 2);
            }

#endif

            if(app_run() < 0)
            {
                run_app_failed();
                print_msg();
            }
        }
    }
}

/**
 ****************************************************************************
 * @brief  None
 * @author lizdDong
 * @note   None
 * @param  None
 * @retval None
 ****************************************************************************
*/
static int32_t app_run(void)
{
    uint32_t appAddress;
    pFunction application;

    printf("Run application >>>>>>>> \r\n");
    deinit_all();
    __disable_irq();
    if(((*(__IO uint32_t *)ApplicationAddress) & 0x2FFE0000) == 0x20000000) //判断用户是否已经下载程序，防止跑飞
    {
        //跳转至用户代码
        appAddress = *(__IO uint32_t *)(ApplicationAddress + 4);
        application = (pFunction)appAddress;
        //初始化用户程序的堆栈指针
        __set_MSP(*(__IO uint32_t *)ApplicationAddress);
        __enable_irq();
        application();
    }
    init_all();
    __enable_irq();
    return (-1);
}

/**
 ****************************************************************************
 * @brief  None
 * @author lizdDong
 * @note   None
 * @param  None
 * @retval None
 ****************************************************************************
*/
static void flash_copy(uint32_t destination, uint32_t source, uint32_t size)
{
    uint32_t addr_d, addr_s, addr_inc, count;

    addr_inc = sizeof(gaFlashTemp);
    addr_d = IAP_APP_ADDR;
    addr_s = IAP_IMAGE_ADDR;
    count = 0;

    while(count < size)
    {
        if(size - count > addr_inc)
        {
            addr_inc = sizeof(gaFlashTemp);
        }
        else
        {
            addr_inc = size - count;
        }

        dev_flashRead(addr_s, gaFlashTemp, addr_inc);
        dev_flashWrite(addr_d, gaFlashTemp, addr_inc);
        addr_s += addr_inc;
        addr_d += addr_inc;
        count += addr_inc;

        printf("Progress: %d%%   \r", count * 100 / size);
    }
    printf("\n");
}

/**
 ****************************************************************************
 * @brief  None
 * @author lizdDong
 * @note   None
 * @param  None
 * @retval None
 ****************************************************************************
*/
static void io_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    GPIO_ResetBits(GPIOA, GPIO_Pin_15);   // JTDI
    GPIO_ResetBits(GPIOB, GPIO_Pin_4);    // NJTRST
}

/**
 ****************************************************************************
 * @brief  None
 * @author lizdDong
 * @note   None
 * @param  None
 * @retval None
 ****************************************************************************
*/
static void io_deinit(void)
{
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
    GPIO_DeInit(GPIOA);
    GPIO_DeInit(GPIOB);
}

/**
 ****************************************************************************
 * @brief  None
 * @author lizdDong
 * @note   None
 * @param  None
 * @retval None
 ****************************************************************************
*/
static void systick_init(void)
{
    SysTick_Config(SystemCoreClock / 1000);
}

/**
 ****************************************************************************
 * @brief  None
 * @author lizdDong
 * @note   None
 * @param  None
 * @retval None
 ****************************************************************************
*/
static void systick_deinit(void)
{
    SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;
}

/**
 ****************************************************************************
 * @brief  None
 * @author lizdDong
 * @note   None
 * @param  None
 * @retval None
 ****************************************************************************
*/
static void uart_init(void)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

#if (USART_PORT_USE == 1)

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    //配置 USART1 Tx (PA.09) 作为功能引脚并推挽复用模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //配置 USART1 Rx (PA.10) 作为功能引脚并是浮空输入模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

#elif (USART_PORT_USE == 3)

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    //配置 USART3 Tx (PB.10) 作为功能引脚并推挽复用模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //配置 USART3 Tx (PB.11) 作为功能引脚并是浮空输入模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

#endif

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(COM_PORT, &USART_InitStructure);
    USART_ClearFlag(COM_PORT, USART_FLAG_TC);
    USART_Cmd(COM_PORT, ENABLE);

#if (USE_RS485_PORT)
    RS485_InitTXE();
#endif
}

/**
 ****************************************************************************
 * @brief  None
 * @author lizdDong
 * @note   None
 * @param  None
 * @retval None
 ****************************************************************************
*/
static void uart_deinit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    USART_DeInit(COM_PORT);
}

#if (USE_RS485_PORT)
/**
 ****************************************************************************
 * @brief  None
 * @author lizdDong
 * @note   None
 * @param  None
 * @retval None
 ****************************************************************************
*/
static void RS485_InitTXE(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_RS485_TXEN, ENABLE); /* 打开GPIO时钟 */

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    /* 推挽输出模式 */
    GPIO_InitStructure.GPIO_Pin = PIN_RS485_TXEN;
    GPIO_Init(PORT_RS485_TXEN, &GPIO_InitStructure);
}
#endif

/**
 ****************************************************************************
 * @brief  None
 * @author lizdDong
 * @note   None
 * @param  None
 * @retval None
 ****************************************************************************
*/
int fputc(int ch, FILE *f)
{
#if(PRINT_MSG_EN)
    Send_Byte((uint8_t)ch);
#endif

    return ch;
}

/**
 ****************************************************************************
 * @brief  None
 * @author lizdDong
 * @note   None
 * @param  None
 * @retval None
 ****************************************************************************
*/
int fgetc(FILE *f)
{
    while(USART_GetFlagStatus(COM_PORT, USART_FLAG_RXNE) == RESET);
    return (int)USART_ReceiveData(COM_PORT);
}

/**
 ****************************************************************************
 * @brief  None
 * @author lizdDong
 * @note   None
 * @param  None
 * @retval None
 ****************************************************************************
*/
void SysTick_Handler(void)
{
    gMsCounter++;
}
