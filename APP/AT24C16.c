/**
 ******************************************************************************
 * @file    AT24C16.c
 * @author  kokirika
 * @brief   AT24C16 EEPROM Driver
 ******************************************************************************
 */

#include "AT24C16.h"
#include <string.h>


/* ==================== 全局公式参数 ==================== */

FormulaParam_t FormulaParam =
{
    .magic = FORMULA_MAGIC,
    .a = FORMULA_DEFAULT_A,
    .b = FORMULA_DEFAULT_B,
    .c = FORMULA_DEFAULT_C
};


/* ============================================================
 * AT24C16内部函数
 * ============================================================ */


/**
 * @brief 根据EEPROM地址计算AT24C16设备地址
 *
 * AT24C16比较特殊：
 *
 * 11位EEPROM地址：
 *
 * A10 A9 A8 A7 ... A0
 *
 * 其中：
 *
 * A10 A9 A8
 *
 * 被放到了I2C设备地址最低3位。
 *
 * 因此：
 *
 * EEPROM 0x000~0x0FF -> I2C地址0x50
 * EEPROM 0x100~0x1FF -> I2C地址0x51
 * EEPROM 0x200~0x2FF -> I2C地址0x52
 * ...
 * EEPROM 0x700~0x7FF -> I2C地址0x57
 *
 */
static uint16_t AT24C16_GetDeviceAddress(uint16_t mem_addr)
{
    uint16_t block;

    block = (mem_addr >> 8) & 0x07U;

    /*
     * STM32 HAL要求设备地址左移1位
     */
    return (uint16_t)((AT24C16_BASE_ADDR | block) << 1);
}


/**
 * @brief 获取块内8-bit地址
 */
static uint8_t AT24C16_GetWordAddress(uint16_t mem_addr)
{
    return (uint8_t)(mem_addr & 0xFFU);
}


/**
 * @brief 等待EEPROM内部写入完成
 */
static HAL_StatusTypeDef AT24C16_WaitReady(uint16_t dev_addr)
{
    return HAL_I2C_IsDeviceReady(&hi2c1,
                                 dev_addr,
                                 AT24C16_READY_TRIALS,
                                 AT24C16_TIMEOUT);
}



/* ============================================================
 * AT24C16检测
 * ============================================================ */

HAL_StatusTypeDef AT24C16_IsReady(void)
{
    /*
     * 检测第一个Block：
     * 0x50 << 1
     */
    return HAL_I2C_IsDeviceReady(&hi2c1,
                                 (AT24C16_BASE_ADDR << 1),
                                 5,
                                 AT24C16_TIMEOUT);
}



/* ============================================================
 * AT24C16写
 * ============================================================ */

HAL_StatusTypeDef AT24C16_Write(uint16_t mem_addr,
                               const uint8_t *data,
                               uint16_t len)
{
    HAL_StatusTypeDef status;

    uint16_t dev_addr;
    uint8_t word_addr;

    uint16_t page_remaining;
    uint16_t block_remaining;
    uint16_t write_len;


    /* -------------------- 参数检查 -------------------- */

    if ((data == NULL) || (len == 0U))
    {
        return HAL_ERROR;
    }

    if (mem_addr >= AT24C16_SIZE)
    {
        return HAL_ERROR;
    }

    if (((uint32_t)mem_addr + len) > AT24C16_SIZE)
    {
        return HAL_ERROR;
    }


    /* -------------------- 分段写入 -------------------- */

    while (len > 0U)
    {
        dev_addr = AT24C16_GetDeviceAddress(mem_addr);

        word_addr = AT24C16_GetWordAddress(mem_addr);


        /*
         * 当前16 Byte页面还剩多少空间
         *
         * 例如：
         *
         * 地址0x05：
         * 当前页剩余 = 16 - 5 = 11 Byte
         */
        page_remaining =
            AT24C16_PAGE_SIZE -
            (mem_addr % AT24C16_PAGE_SIZE);


        /*
         * 当前256 Byte Block还剩多少空间
         *
         * AT24C16每256 Byte需要切换一次设备地址
         */
        block_remaining =
            0x100U -
            (mem_addr & 0xFFU);


        /*
         * 本次写入长度先取总剩余长度
         */
        write_len = len;


        /*
         * 不允许跨16 Byte页
         */
        if (write_len > page_remaining)
        {
            write_len = page_remaining;
        }


        /*
         * 不允许跨256 Byte Block
         */
        if (write_len > block_remaining)
        {
            write_len = block_remaining;
        }


        /* -------------------- 执行写入 -------------------- */

        status = HAL_I2C_Mem_Write(&hi2c1,
                                   dev_addr,
                                   word_addr,
                                   I2C_MEMADD_SIZE_8BIT,
                                   (uint8_t *)data,
                                   write_len,
                                   AT24C16_TIMEOUT);

        if (status != HAL_OK)
        {
            return status;
        }


        /*
         * EEPROM写完数据以后，
         * 内部还需要几毫秒真正写入Flash Cell。
         *
         * 这里不用固定HAL_Delay(5)，
         * 直接ACK Polling等待完成。
         */
        status = AT24C16_WaitReady(dev_addr);

        if (status != HAL_OK)
        {
            return status;
        }


        /* -------------------- 更新地址 -------------------- */

        mem_addr += write_len;

        data += write_len;

        len -= write_len;
    }


    return HAL_OK;
}



/* ============================================================
 * AT24C16读
 * ============================================================ */

HAL_StatusTypeDef AT24C16_Read(uint16_t mem_addr,
                              uint8_t *data,
                              uint16_t len)
{
    HAL_StatusTypeDef status;

    uint16_t dev_addr;
    uint8_t word_addr;

    uint16_t block_remaining;
    uint16_t read_len;


    /* -------------------- 参数检查 -------------------- */

    if ((data == NULL) || (len == 0U))
    {
        return HAL_ERROR;
    }

    if (mem_addr >= AT24C16_SIZE)
    {
        return HAL_ERROR;
    }

    if (((uint32_t)mem_addr + len) > AT24C16_SIZE)
    {
        return HAL_ERROR;
    }


    /* -------------------- 分Block读取 -------------------- */

    while (len > 0U)
    {
        dev_addr = AT24C16_GetDeviceAddress(mem_addr);

        word_addr = AT24C16_GetWordAddress(mem_addr);


        /*
         * 当前256 Byte Block剩余空间
         */
        block_remaining =
            0x100U -
            (mem_addr & 0xFFU);


        read_len = len;


        /*
         * 防止读操作跨越Block，
         * 因为下一个Block设备地址会发生变化。
         */
        if (read_len > block_remaining)
        {
            read_len = block_remaining;
        }


        status = HAL_I2C_Mem_Read(&hi2c1,
                                  dev_addr,
                                  word_addr,
                                  I2C_MEMADD_SIZE_8BIT,
                                  data,
                                  read_len,
                                  AT24C16_TIMEOUT);

        if (status != HAL_OK)
        {
            return status;
        }


        mem_addr += read_len;

        data += read_len;

        len -= read_len;
    }


    return HAL_OK;
}



/* ============================================================
 * 公式参数保存
 * ============================================================ */

HAL_StatusTypeDef Formula_Save(void)
{
    /*
     * 每次保存前重新写Magic
     */
    FormulaParam.magic = FORMULA_MAGIC;


    return AT24C16_Write(
        FORMULA_EEPROM_ADDR,
        (const uint8_t *)&FormulaParam,
        sizeof(FormulaParam_t)
    );
}



/* ============================================================
 * 设置参数并保存
 * ============================================================ */

HAL_StatusTypeDef Formula_SetAndSave(float a,
                                    float b,
                                    float c)
{
    FormulaParam.magic = FORMULA_MAGIC;

    FormulaParam.a = a;
    FormulaParam.b = b;
    FormulaParam.c = c;


    return Formula_Save();
}



/* ============================================================
 * 从EEPROM加载参数
 * ============================================================ */

uint8_t Formula_Load(void)
{
    FormulaParam_t temp;


    /*
     * 先读到临时变量，
     * 防止EEPROM读取失败破坏当前RAM参数。
     */
    if (AT24C16_Read(
            FORMULA_EEPROM_ADDR,
            (uint8_t *)&temp,
            sizeof(FormulaParam_t)
        ) != HAL_OK)
    {
        return 0;
    }


    /*
     * 检查Magic
     */
    if (temp.magic != FORMULA_MAGIC)
    {
        return 0;
    }


    /*
     * 参数有效，复制到当前参数
     */
    FormulaParam = temp;


    return 1;
}



/* ============================================================
 * 恢复默认参数
 * ============================================================ */

HAL_StatusTypeDef Formula_ResetDefault(void)
{
    FormulaParam.magic = FORMULA_MAGIC;

    FormulaParam.a = FORMULA_DEFAULT_A;

    FormulaParam.b = FORMULA_DEFAULT_B;

    FormulaParam.c = FORMULA_DEFAULT_C;


    return Formula_Save();
}

float get_Formula_a()
{
	return FormulaParam.a;
}

float get_Formula_b()
{
	return FormulaParam.b;
}	

float get_Formula_c()
{
	return FormulaParam.c;
}



