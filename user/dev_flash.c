/**
 ******************************************************************************
 * @file    dev_Flash.c
 * @author  lizdDong
 * @version V1.0
 * @date    2021-4-19
 * @brief   None
 * @attention
 *
 ******************************************************************************
 */

#include "stm32f10x_flash.h"
#include "dev_flash.h"


static uint16_t FlashTemp[PAGE_SIZE / 2]; //Up to 2K bytes

/**
 ****************************************************************************
 * @brief  There is no check writing.
 * @note   This function can be used for all STM32F10x devices.
 * @param  writeAddr: The starting address to be written.
 * @param  pBuffer: The pointer to the data.
 * @param  numToWrite:  The number of half words written
 * @retval None
 ****************************************************************************
*/
static FLASH_Status dev_flashWriteNoCheck(uint32_t writeAddr, uint16_t *pBuffer, uint16_t numToWrite)
{
    uint16_t i;
    FLASH_Status status = FLASH_COMPLETE;
    for(i = 0; i < numToWrite; i++)
    {
        status = FLASH_ProgramHalfWord(writeAddr, pBuffer[i]);
        if(status != FLASH_COMPLETE)
        {
            break;
        }
        writeAddr += 2;   //add addr 2.
    }
    return status;
}

/**
 ****************************************************************************
 * @brief  Write data from the specified address to the specified length.
 * @note   This function can be used for all STM32F10x devices.
 * @param  addr: The starting address to be written.(The address must be a multiple of two)
 * @param  pBuff: The pointer to the data.
 * @param  size: The number of byte written(8bit), The number should beat to a multiple of two.
 * @retval None
 ****************************************************************************
*/
uint32_t dev_flashWrite(uint32_t addr, const uint8_t *pBuff, uint32_t size)
{
    uint32_t secpos;     //扇区地址
    uint16_t secoff;     //扇区内偏移地址(16位字计算)
    uint16_t secremain;  //扇区内剩余地址(16位字计算)
    uint16_t i;
    uint8_t f_need_erase = 0;
    uint8_t f_need_write = 0;
    uint32_t offaddr;    //去掉0X08000000后的地址

    uint32_t writeAddr = addr;
    uint16_t *pBuffer = (uint16_t *)(pBuff);    //转换为半字操作
    uint32_t numToWrite = size / 2;             //转换为半字操作
    uint32_t numOfWrited = 0;     //实际写入的数据长度

    if((writeAddr < FLASH_BASE) || (writeAddr >= FLASH_BASE + FLASH_SIZE))
    {
        //非法地址
        return 0;
    }

    if((writeAddr & 0x00000001) == 0x00000001)
    {
        //地址必须为偶数
        return 0;
    }

    if(writeAddr + size >= FLASH_BASE + FLASH_SIZE)
    {
        //实际可写入的空间大小
        numToWrite = (FLASH_BASE + FLASH_SIZE - writeAddr) / 2;
    }

    offaddr = writeAddr - FLASH_BASE;     //实际偏移地址.
    secpos = offaddr / PAGE_SIZE;         //扇区地址  0~127 for STM32F103RBT6
    secoff = (offaddr % PAGE_SIZE) / 2;   //在扇区内的偏移(2个字节为基本单位.)
    secremain = PAGE_SIZE / 2 - secoff;   //扇区剩余空间大小(16位字计算)

    if(numToWrite <= secremain)
    {
        //数据写入无需跨扇区
        secremain = numToWrite;
    }

    //__set_PRIMASK(1);     /* 关中断 */
    FLASH_Unlock();       //解锁
    /* Clear pending flags (if any) */
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    while(1)
    {
        //读出整个扇区的内容
        dev_flashRead(secpos * PAGE_SIZE + FLASH_BASE, (uint8_t *)FlashTemp, PAGE_SIZE);

        //判断是否需要执行擦除操作
        for(i = 0; i < secremain; i++)
        {
            //校验目标地址是否与将要写入的数据相等
            if(FlashTemp[secoff + i] != pBuffer[i])
            {
                f_need_write = 1;

                //校验目标地址是否全为0xFFFF，
                if(FlashTemp[secoff + i] != 0XFFFF)
                {
                    //需要擦除
                    f_need_erase = 1;
                    break;
                }
            }
        }

        if(f_need_erase)
        {
            //需要擦除
            if(FLASH_ErasePage(secpos * PAGE_SIZE + FLASH_BASE) != FLASH_COMPLETE)   //擦除这个扇区
            {
                return numToWrite;
            }
            for(i = 0; i < secremain; i++)
            {
                //先将数据写入缓存
                FlashTemp[secoff + i] = pBuffer[i];
            }
            //然后再写整个扇区
            if(dev_flashWriteNoCheck(secpos * PAGE_SIZE + FLASH_BASE, FlashTemp, PAGE_SIZE / 2) != FLASH_COMPLETE)
            {
                return numToWrite;
            }
            numOfWrited += (secremain * 2);
        }
        else
        {
            //无需擦除,直接写入扇区剩余区间
            if(f_need_write)
            {
                if(dev_flashWriteNoCheck(writeAddr, pBuffer, secremain) != FLASH_COMPLETE)
                {
                    return numToWrite;
                }
            }
            numOfWrited += (secremain * 2);
        }

        if(numToWrite == secremain)
        {
            break;           //写入结束
        }
        else
        {
            //准备写下一个扇区
            secpos++;     //扇区地址增1
            secoff = 0;   //偏移位置为0
            pBuffer += secremain;            //指针偏移
            writeAddr += (secremain * 2);    //写地址偏移
            numToWrite -= secremain;         //字节(16位)数递减
            if(numToWrite > (PAGE_SIZE / 2))
            {
                //剩余数据仍需跨扇区
                secremain = PAGE_SIZE / 2;
            }
            else
            {
                //剩余数据无需跨扇区
                secremain = numToWrite;
            }
        }
    }
    FLASH_Lock();//上锁
    __set_PRIMASK(0);     /* 开中断 */

    return numOfWrited;
}

/**
 ****************************************************************************
 * @brief  Start reading the specified data from the specified address.
 * @note   This function can be used for all STM32F10x devices.
 * @param  readAddr: Start addr
 * @param  pBuff: The pointer to the data.
 * @param  size: The number of byte written(8bit)
 * @retval The real size to read
 ****************************************************************************
*/
uint32_t dev_flashRead(uint32_t addr, uint8_t *pBuff, uint32_t size)
{
    uint32_t i;
    uint32_t readAddr = addr;

    if(readAddr + size >= FLASH_BASE + FLASH_SIZE)
    {
        //实际可读取的数据长度
        size = FLASH_BASE + FLASH_SIZE - readAddr;
    }

    for(i = 0; i < size; i++)
    {
        *pBuff++ = *(uint8_t *)readAddr++;
    }

    return size;
}


/****************************** End of file ***********************************/



