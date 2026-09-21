/**
 ******************************************************************************
 * @file    AT24C16.h
 * @author  kokirika
 * @brief   AT24C16 EEPROM Driver
 ******************************************************************************
 */

#ifndef __AT24C16_H
#define __AT24C16_H

#include "main.h"
#include "i2c.h"
#include <stdint.h>

/* ==================== AT24C16 参数 ==================== */

/*
 * AT24C16容量：
 * 16 Kbit = 2048 Byte
 *
 * 地址范围：
 * 0x000 ~ 0x7FF
 */
#define AT24C16_SIZE             2048U

/*
 * AT24C16页大小：
 * 16 Byte
 */
#define AT24C16_PAGE_SIZE        16U

/*
 * AT24C16基础7-bit I2C地址
 *
 * 实际地址范围：
 * 0x50 ~ 0x57
 *
 * HAL调用时内部需要左移1位，
 * 驱动函数会自动处理。
 */
#define AT24C16_BASE_ADDR        0x50U

/*
 * I2C超时时间
 */
#define AT24C16_TIMEOUT          100U

/*
 * EEPROM内部写周期ACK轮询次数
 */
#define AT24C16_READY_TRIALS     100U


/* ==================== 公式参数 ==================== */

/*
 * 参数存储起始地址
 *
 * 0x000开始存储：
 *
 * 0x000 ~ 0x003     magic
 * 0x004 ~ 0x007     a
 * 0x008 ~ 0x00B     b
 * 0x00C ~ 0x00F     c
 *
 * 共16 Byte，刚好一页。
 */
#define FORMULA_EEPROM_ADDR      0x000U

/*
 * 参数有效标志
 */
#define FORMULA_MAGIC            0x4B4F4B49UL


/*
 * 默认公式：
 *
 * y = a*x*x + b*x + c
 */
#define FORMULA_DEFAULT_A        0.2f
#define FORMULA_DEFAULT_B        1.2f
#define FORMULA_DEFAULT_C        2.1f


typedef struct
{
    uint32_t magic;

    float a;
    float b;
    float c;

} FormulaParam_t;


/* ==================== 全局参数 ==================== */

extern FormulaParam_t FormulaParam;


/* ==================== AT24C16底层函数 ==================== */

/**
 * @brief  写AT24C16
 *
 * @param  mem_addr EEPROM地址 0~2047
 * @param  data     数据指针
 * @param  len      数据长度
 *
 * @retval HAL_OK / HAL_ERROR / HAL_TIMEOUT / HAL_BUSY
 */
HAL_StatusTypeDef AT24C16_Write(uint16_t mem_addr,
                               const uint8_t *data,
                               uint16_t len);


/**
 * @brief  读AT24C16
 *
 * @param  mem_addr EEPROM地址 0~2047
 * @param  data     数据缓存
 * @param  len      数据长度
 *
 * @retval HAL_OK / HAL_ERROR / HAL_TIMEOUT / HAL_BUSY
 */
HAL_StatusTypeDef AT24C16_Read(uint16_t mem_addr,
                              uint8_t *data,
                              uint16_t len);


/**
 * @brief 检测AT24C16是否存在
 *
 * @retval HAL_OK      存在
 * @retval 其它        未检测到
 */
HAL_StatusTypeDef AT24C16_IsReady(void);


/* ==================== 公式参数函数 ==================== */

/**
 * @brief 保存当前a、b、c到EEPROM
 */
HAL_StatusTypeDef Formula_Save(void);


/**
 * @brief 保存指定a、b、c到EEPROM
 */
HAL_StatusTypeDef Formula_SetAndSave(float a,
                                    float b,
                                    float c);


/**
 * @brief 从EEPROM读取参数
 *
 * @retval 1 参数有效
 * @retval 0 参数无效/读取失败
 */
uint8_t Formula_Load(void);


/**
 * @brief 恢复默认参数并保存
 */
HAL_StatusTypeDef Formula_ResetDefault(void);

float get_Formula_a(void);
float get_Formula_b(void);	
float get_Formula_c(void);																			

#endif
