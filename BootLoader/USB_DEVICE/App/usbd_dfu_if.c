/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_dfu_if.c
  * @brief          : Usb device for Download Firmware Update.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_dfu_if.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
static char str[160];
/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device.
  * @{
  */

/** @defgroup USBD_DFU
  * @brief Usb DFU device module.
  * @{
  */

/** @defgroup USBD_DFU_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */
extern uint32_t AddrAppBegin;
extern uint32_t AddrAppEnd;
extern UART_HandleTypeDef huart2;
/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_DFU_Private_Defines
  * @brief Private defines.
  * @{
  */

#define FLASH_DESC_STR      "@Internal Flash   /0x08000000/03*016Ka,01*016Kg,01*064Kg,07*128Kg,04*016Kg,01*064Kg,07*128Kg"

/* USER CODE BEGIN PRIVATE_DEFINES */
#define FLASH_ERASE_TIME    (uint16_t)50
#define FLASH_PROGRAM_TIME  (uint16_t)50
/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_DFU_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_DFU_Private_Variables
  * @brief Private variables.
  * @{
  */

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_DFU_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_DFU_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static uint16_t MEM_If_Init_FS(void);
static uint16_t MEM_If_Erase_FS(uint32_t Add);
static uint16_t MEM_If_Write_FS(uint8_t *src, uint8_t *dest, uint32_t Len);
static uint8_t *MEM_If_Read_FS(uint8_t *src, uint8_t *dest, uint32_t Len);
static uint16_t MEM_If_DeInit_FS(void);
static uint16_t MEM_If_GetStatus_FS(uint32_t Add, uint8_t Cmd, uint8_t *buffer);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

#if defined ( __ICCARM__ ) /* IAR Compiler */
  #pragma data_alignment=4
#endif
__ALIGN_BEGIN USBD_DFU_MediaTypeDef USBD_DFU_fops_FS __ALIGN_END =
{
   (uint8_t*)FLASH_DESC_STR,
    MEM_If_Init_FS,
    MEM_If_DeInit_FS,
    MEM_If_Erase_FS,
    MEM_If_Write_FS,
    MEM_If_Read_FS,
    MEM_If_GetStatus_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Memory initialization routine.
  * @retval USBD_OK if operation is successful, MAL_FAIL else.
  */
uint16_t MEM_If_Init_FS(void)
{
  /* USER CODE BEGIN 0 */
    HAL_StatusTypeDef flash_ok = HAL_ERROR;

    sprintf( str, "Flash unlock ... " );
    HAL_UART_Transmit( &huart2, (uint8_t*)str, strlen( str ), HAL_MAX_DELAY );
    while ( flash_ok != HAL_OK )
        flash_ok = HAL_FLASH_Unlock();
    sprintf( str, "OK\r" );
    HAL_UART_Transmit( &huart2, (uint8_t*)str, strlen( str ), HAL_MAX_DELAY );
    return USBD_OK;
  /* USER CODE END 0 */
}

/**
  * @brief  De-Initializes Memory
  * @retval USBD_OK if operation is successful, MAL_FAIL else
  */
uint16_t MEM_If_DeInit_FS(void)
{
  /* USER CODE BEGIN 1 */
    HAL_StatusTypeDef flash_ok = HAL_ERROR;

    sprintf( str, "Flash lock ... " );
    HAL_UART_Transmit( &huart2, (uint8_t*)str, strlen( str ), HAL_MAX_DELAY );
    flash_ok = HAL_ERROR;
    while ( flash_ok != HAL_OK )
        flash_ok = HAL_FLASH_Lock();
    sprintf( str, "OK\r" );
    HAL_UART_Transmit( &huart2, (uint8_t*)str, strlen( str ), HAL_MAX_DELAY );
    return USBD_OK;
  /* USER CODE END 1 */
}

/**
  * @brief  Erase sector.
  * @param  Add: Address of sector to be erased.
  * @retval 0 if operation is successful, MAL_FAIL else.
  */
uint16_t MEM_If_Erase_FS(uint32_t Add)
{
  /* USER CODE BEGIN 2 */
    uint32_t NbOfPages = 0;
    uint32_t PageError = 0;
    /* Variable contains Flash operation status */
    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef eraseinitstruct;

    /* Get the number of sector to erase from 1st sector*/
    NbOfPages = ( ( AddrAppEnd - AddrAppBegin ) / FLASH_PAGE_SIZE ) + 1;
    eraseinitstruct.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseinitstruct.PageAddress = AddrAppBegin;
    eraseinitstruct.NbPages = NbOfPages;
    eraseinitstruct.Banks = FLASH_BANK_1;
    status = HAL_FLASHEx_Erase( &eraseinitstruct, &PageError );
    sprintf( str, "Erase flash, address: 0x%08X, pages: 0x%04X (%u), page size: %u ... %s\r", (unsigned)AddrAppBegin, (unsigned)NbOfPages, (unsigned)NbOfPages, FLASH_PAGE_SIZE, ( status == HAL_OK ) ? "OK" : "NO" );
    HAL_UART_Transmit( &huart2, (uint8_t*)str, strlen( str ), HAL_MAX_DELAY );
    if ( status != HAL_OK )
        return USBD_FAIL;
    else return USBD_OK;
  /* USER CODE END 2 */
}

/**
  * @brief  Memory write routine.
  * @param  src: Pointer to the source buffer. Address to be written to.
  * @param  dest: Pointer to the destination buffer.
  * @param  Len: Number of data to be written (in bytes).
  * @retval USBD_OK if operation is successful, MAL_FAIL else.
  */
uint16_t MEM_If_Write_FS(uint8_t *src, uint8_t *dest, uint32_t Len)
{
  /* USER CODE BEGIN 3 */
    uint32_t i = 0;

    sprintf( str, "Write flash: Src: 0x%08X Dest: 0x%08X: Len: 0x%04X\r", (unsigned)src, (unsigned)dest, (unsigned)Len );
    HAL_UART_Transmit( &huart2, (uint8_t*)str, strlen( str ), HAL_MAX_DELAY );
    for ( i = 0; i < Len; i += 4 ) {
        //Device voltage range supposed to be [2.7V to 3.6V], the operation will be done by byte
        if ( HAL_FLASH_Program( FLASH_TYPEPROGRAM_WORD, (uint32_t) (dest + i), *(uint32_t*) (src + i)) == HAL_OK ) {
            //Check the written value
            if ( *(uint32_t *) (src + i) != *(uint32_t*) (dest + i) ) {
                sprintf( str, "Flash content doesn't match SRAM content, address: 0x%08X Src: 0x%08X Dest: 0x%08X\r", (unsigned)src, (unsigned)(*(uint32_t *)(src + i)), (unsigned)(*(uint32_t*)(dest + i)) );
                HAL_UART_Transmit( &huart2, (uint8_t*)str, strlen( str ), HAL_MAX_DELAY );
                return 2; //Flash content doesn't match SRAM content
            }
        }
        else {
            sprintf( str, "Error occurred while writing data in Flash memory\r" );
            HAL_UART_Transmit( &huart2, (uint8_t*)str, strlen( str ), HAL_MAX_DELAY );
            return 1; //Error occurred while writing data in Flash memory
        }
      }
    return USBD_OK;
  /* USER CODE END 3 */
}

/**
  * @brief  Memory read routine.
  * @param  src: Pointer to the source buffer. Address to be written to.
  * @param  dest: Pointer to the destination buffer.
  * @param  Len: Number of data to be read (in bytes).
  * @retval Pointer to the physical address where data should be read.
  */
uint8_t *MEM_If_Read_FS(uint8_t *src, uint8_t *dest, uint32_t Len)
{
  /* Return a valid address to avoid HardFault */
  /* USER CODE BEGIN 4 */
    uint32_t i = 0;
    uint8_t *psrc = src;

    for ( i = 0; i < Len; i++ )
        dest[i] = *psrc++;
    sprintf( str, "Read flash: 0x%08X Dest: 0x%08X: Len: 0x%04X (%u)\r", (unsigned)src, (unsigned)dest, (unsigned)Len, (unsigned)Len );
    HAL_UART_Transmit( &huart2, (uint8_t*)str, strlen( str ), HAL_MAX_DELAY );
    return (uint8_t*) dest;
  /* USER CODE END 4 */
}

/**
  * @brief  Get status routine
  * @param  Add: Address to be read from
  * @param  Cmd: Number of data to be read (in bytes)
  * @param  buffer: used for returning the time necessary for a program or an erase operation
  * @retval USBD_OK if operation is successful
  */
uint16_t MEM_If_GetStatus_FS(uint32_t Add, uint8_t Cmd, uint8_t *buffer)
{
  /* USER CODE BEGIN 5 */

    switch ( Cmd ) {
    case DFU_MEDIA_PROGRAM:
        buffer[1] = (uint8_t) FLASH_PROGRAM_TIME;
        buffer[2] = (uint8_t) (FLASH_PROGRAM_TIME << 8);
        buffer[3] = 0;
        break;
    case DFU_MEDIA_ERASE:
    default:
        buffer[1] = (uint8_t) FLASH_ERASE_TIME;
        buffer[2] = (uint8_t) (FLASH_ERASE_TIME << 8);
        buffer[3] = 0;
        break;
    }
    sprintf( str, "Get status flash.\r" );
    HAL_UART_Transmit( &huart2, (uint8_t*) str, strlen( str ), HAL_MAX_DELAY );
    return USBD_OK;
  /* USER CODE END 5 */
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
