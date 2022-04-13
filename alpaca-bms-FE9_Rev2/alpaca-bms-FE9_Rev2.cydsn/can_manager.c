/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include "can_manager.h"

#include "cell_interface.h"

volatile uint8_t can_buffer[8];
volatile uint8_t rx_can_buffer[8];
volatile int16_t current = 0;
extern BAT_PACK_t bat_pack;
extern volatile uint8_t CAN_DEBUG;

/* Data Frame format for Voltage and Temperature
The datatype consists of three bytes:
1. Identifying Number (eg cell #1)
2. upper byte of data
3. lower byte of data
*/

/* PCAN_SendMsgx() function associations
0. PCAN_SendMsg0() => Sends Temps
1. PCAN_SendMsg1() => Sends Voltage
2. PCAN_SendMsg2() => Sends Current
3. PCAN_SendMsg3() => Sends Status
4. PCAN_SendMsg4() => Sends SOC message(may not be used)
*/

void can_send_temp(volatile BAT_SUBPACK_t *subpacks[N_OF_SUBPACK],
    volatile uint8_t high_tempNode,
    volatile uint8_t high_temp)
{
    for (unsigned int i = 0; i < N_OF_SUBPACK; i++) {
        can_buffer[i] = subpacks[i]->high_temp; //I'm not quite sure I sent in the correct parameter to access this. Could you check this?
    }    
    can_buffer[6] = 0xff & high_tempNode;
    can_buffer[7] = high_temp; //(high_temp/10)<<4 | (high_temp%10);

    //This works in the case that the number of subpacks is

	PCAN_SendMsg0(); // Sends Temps
    CyDelay(5);
} // can_send_temp() 


void can_send_volt(
    volatile uint16_t min_voltage,
    volatile uint16_t max_voltage,
    volatile uint32_t pack_voltage)
{
    //max and min voltage means the voltage of single cell
        can_buffer[0] = HI8(min_voltage);
        can_buffer[1] = LO8(min_voltage);

        can_buffer[2] = HI8(max_voltage);
        can_buffer[3] = LO8(max_voltage);

        can_buffer[4] = 0xFF & (pack_voltage >> 24);
        can_buffer[5] = 0xFF & (pack_voltage >> 16);
        can_buffer[6] = 0xFF & (pack_voltage >> 8);
        can_buffer[7] = 0xFF & (pack_voltage);


        PCAN_SendMsg1();  // Sends Voltage
        CyDelay(1);

} // can_send_volt()


void can_send_status(volatile uint8_t name,
                    volatile uint8_t SOC_P,
                    volatile uint16_t status,
                    volatile uint8_t stack,
                    volatile uint8_t cell,
                    volatile uint16_t value16){
//8 SOC Percent
//8 AH used since full charge
//16 BMS Status bits (error flags)
//16 Number of charge cycles
//16 Pack balance (delta) mV
    can_buffer[0] = name;
    can_buffer[1] = (uint8_t)(SOC_P/10)<<4 | (uint8_t)(SOC_P%10);
    can_buffer[2] = HI8(status);
    can_buffer[3] = LO8(status);
    can_buffer[4] = stack & 0xFF;
    can_buffer[5] = (cell) & 0xFF;
    can_buffer[6] = HI8(value16);
    can_buffer[7] = LO8(value16);

    PCAN_SendMsg3(); // Sends Status
}

void get_current(volatile BAT_PACK_t *bat_pack)
{
    bat_pack->current = current;
}

void RX_get_current(uint8_t *msg, int CAN_ID)
{
    uint8 InterruptState = CyEnterCriticalSection();
    
    if (CAN_ID == 0x069)
    {
        for (int i = 0; i < 8; i++) 
        {
            rx_can_buffer[i] = msg[i];
        }
        current += rx_can_buffer[0] << 8;
        current += rx_can_buffer[1];
    }
    
    
    CyExitCriticalSection(InterruptState);
    //return current;
}
                    
void can_init()
{
	PCAN_GlobalIntEnable(); // CAN Initialization
	PCAN_Init();
	PCAN_Start();
} // can_init(
/* [] END OF FILE */
