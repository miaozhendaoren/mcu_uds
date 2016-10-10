/***************************************************************************//**
    \file          uds-support.c
    \author        huanghai
    \mail          huanghai@auto-link.com
    \version       0.01
    \date          2016-10-8
    \description   uds service spport code, include read/write data and routine control
*******************************************************************************/

/*******************************************************************************
    Include Files
*******************************************************************************/
#include <absacc.h>
#include <stdint.h>
#include "uds_type.h"
#include "uds_support.h"

/*******************************************************************************
    Function  declaration
*******************************************************************************/
static uint8_t
uds_rtctrl_nothing (void);

/*******************************************************************************
    Global Varaibles
*******************************************************************************/
uint8_t ASC_boot_ver[10]=
{'v', 'e', 'r', '-', '0', '.', '0', '1', 0, 0};
extern uint8_t uds_session;
uint8_t ASC_sys_supplier_id[5]=
{'1', '2', '3', '4', '5'};
uint8_t ASC_soft_ver[10]=
{'v', 'e', 'r', '-', '0', '.', '0', '1', 0, 0};
uint8_t ASC_sys_name[10]=
{'I', 'C', 'U', '-', 'm', '1', '2', 'e', 0, 0};


uint8_t ASC_ecu_part_num[15]  __at(0x20000400);
uint8_t BCD_manufacture_date[3] __at(0x2000040F);
uint8_t HEX_ecu_sn[10] __at(0x20000412);
uint8_t ASC_VIN[17] __at(0x2000041C);
uint8_t HEX_tester_sn[10] __at(0x2000042D);
uint8_t BCD_program_date[3] __at(0x20000437);


const uds_rwdata_t rwdata_list[RWDATA_CNT] =
{
    {0xF183, ASC_boot_ver,         10, UDS_RWDATA_RDONLY,      UDS_RWDATA_DFLASH},
    {0xF186, &uds_session,         1,  UDS_RWDATA_RDONLY,      UDS_RWDATA_RAM},
    {0xF187, ASC_ecu_part_num,     15, UDS_RWDATA_RDWR_INBOOT, UDS_RWDATA_EEPROM},
    {0xF18A, ASC_sys_supplier_id,  5,  UDS_RWDATA_RDONLY,      UDS_RWDATA_DFLASH},
    {0xF18B, BCD_manufacture_date, 3,  UDS_RWDATA_RDONLY,      UDS_RWDATA_EEPROM}, /* be writen after manufacture */
    {0xF18C, HEX_ecu_sn,           10, UDS_RWDATA_RDONLY,      UDS_RWDATA_EEPROM}, /* be writen after manufacture */
    {0xF190, ASC_VIN,              17, UDS_RWDATA_RDWR_WRONCE, UDS_RWDATA_EEPROM}, /* be writen after installment */
    {0xF195, ASC_soft_ver,         10, UDS_RWDATA_RDONLY,      UDS_RWDATA_DFLASH},
    {0xF197, ASC_sys_name,         10, UDS_RWDATA_RDONLY,      UDS_RWDATA_DFLASH},
    {0xF198, HEX_tester_sn,        10, UDS_RWDATA_RDWR_INBOOT, UDS_RWDATA_EEPROM}, /* update by tester after program */
    {0xF199, BCD_program_date,     3,  UDS_RWDATA_RDWR_INBOOT, UDS_RWDATA_EEPROM} /* update by tester after program */
};

const uds_ioctrl_t ioctrl_list[IOCTRL_CNT] = 
{
    {0, NULL, 0, 0, 0, 0, 0}
};

const uds_rtctrl_t rtctrl_list[RTCTRL_CNT] =
{
    {0, UDS_RT_ST_IDLE, &uds_rtctrl_nothing, &uds_rtctrl_nothing, &uds_rtctrl_nothing}
};

/**
 * uds_rtctrl_nothing - a temp Function
 *
 * @void : first frame dlc
 *
 * returns:
 *     0 - ok
 */
static uint8_t
uds_rtctrl_nothing (void)
{
    return 0;
}

/****************EOF****************/
