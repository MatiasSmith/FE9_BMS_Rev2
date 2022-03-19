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
#include "project.h"
#include "cell_interface.h"
#include "can_manager.h"
#include <LTC6811.h>

#include "math.h"
#include <time.h>

//global variables
//uint8_t cfga_on_init[6];
//uint8_t auxa[6];
volatile uint8_t CAN_UPDATE_FLAG=0;
extern volatile BAT_PACK_t bat_pack;
extern BAT_SUBPACK_t bat_subpack[N_OF_SUBPACK];
extern volatile float32 sortedTemps[N_OF_TEMP_CELL]; 
extern float32 high_temp_overall;
extern volatile BAT_ERR_t* bat_err_array;
extern volatile uint8_t bat_err_index;
extern volatile uint8_t bat_err_index_loop;
volatile uint8_t CAN_DEBUG=0;
//volatile uint8_t RACING_FLAG=0;    // this flag should be set in CAN handler
BAT_SOC_t bat_soc;
//Dash_State dash_state = 0;
uint8_t rx_cfg[IC_PER_BUS][8];
void DEBUG_send_cell_voltage();
void DEBUG_send_temp();
//void DEBUG_send_current();

//The old code had many more BMS modes, are we ever going to need that?
//Need BMS_CHARGING at least. We only want to balance cells during charging. 
//BMS_CHARGING will share a lot with BMS_NORMAL, so maybe charging shouldn't be its own mode
typedef enum {
    BMS_NORMAL, 
    BMS_FAULT
}BMS_MODE;

void init(void){   //initialize modules
    SPI_Start();  
    FTDI_UART_Start();
    PIC18_UART_Start();
    //USB_Start(0, USB_5V_OPERATION);
    //USB_CDC_Init();
    //PCAN_Start();
    can_init();
    bms_init(MD_NORMAL); 
    mypack_init();
}

void process_event(){
    CyGlobalIntDisable
    //Old code: small delay(idk why)
    CyDelay(10);
    can_send_status(0xFE,
    	bat_pack.SOC_percent,
    	bat_pack.status,
    	0,0,0);
    CyDelay(10);
    // send voltage   
    can_send_volt(bat_pack.LO_voltage,
				bat_pack.HI_voltage,
				bat_pack.voltage);
    CyDelay(10);
    
    // TEST_DAY_1
    //send temp only if within reasonable range from last temperature

    //TODO: rewrite this for dynamic number of subpacks? 
    can_send_temp(bat_pack.subpacks,
			bat_pack.HI_temp_node,
			bat_pack.HI_temp_c);
    
    can_send_volt(bat_pack.LO_voltage, bat_pack.HI_voltage, bat_pack.voltage);
    //TODO: current will be sent by PEI board
    CyDelay(10);

    CyGlobalIntEnable;
}


int main(void)
{  //SEE ADOW ON DATASHEET PAGE 33

    //In old code, the loop had a BMS_BOOTUP case, but we're doin it all before it
    //goes into the while loop. I am assuming that nothing in the initialization process
    //will throw a BMS_FAULT. If that is the case, we'll need to change it back to how
    //it was.
    //We can probably get away with no bootup. I did have some odd issues with code outside of the while loop, but we'll see. 
    //Even if something throws a fault outside of the while loop, cant we still set the mode to BMS_FAULT to the same effect? 
    

    CyGlobalIntEnable; /* Enable global interrupts. */

    init();   //initialize modules

    //Initialize state machine
    BMS_MODE bms_status = BMS_NORMAL;
    uint32_t system_interval = 0;

    volatile double prev_time_interval;

    while(1) {
        switch(bms_status) {
            case BMS_NORMAL:
                //Start timer to time normal state
                Timer_1_Start();

                //Flowchart: "Tells board to keep HV path closed (CAN)"
                //Not quite sure wha that means.
                //If OK signal goes low, the car shutsdown and cannot be turned back on without power cycling. 
                //The OK signal must be high within a couple seconds of startup to prevent unwanted shutdown. 
                OK_SIG_Write(1);

                //Apparantly the filtered version gives more precision for measurements
                //Yes. Accuracy is specified in the LTC6811 datasheet. 
                bms_init(MD_FILTERED);
                get_voltages();
                get_current(); //get_current used to be under bms_init(MD_NORMAL) but it seemed that
                               //the precision was necessary for SOC estimation
                               //current used to come from a analog input on the PSoC. It will now be coming from PCAN. 

                //Not quite sure why it set it as normal, does it waste time when we leave it as MD_FILTERED?
                //higher accuracy requires more time to acquire (datasheet). Don't need as much accuracy on temps.
                bms_init(MD_NORMAL); 
                get_all_temps();
                //SOC_estimation(double prev_time_interval);
                bms_status = bat_health_check();

                //Calculating time spent in state
                //Old code said "do these time tests ONE FILE AT A TIME due to the hardcoded variable"
                //Not sure what that quite means. Also it seems like we didn't really use the time at all, so 
                //we can maybe send it over to the dashboard through UART?
                //I think time was used to estimate an integral for coulomb counting. Unless the Kalmann filter needs time, we can get rid of it. 
                Timer_1_Stop();
                uint32 time_spent_cycle = Timer_1_ReadCounter();
                prev_time_interval = (double)(time_spent_cycle) / (double)(24000000); //gives time in seconds
                
                break;
            case BMS_FAULT:
                OK_SIG_Write(0u);
                bms_status = BMS_FAULT;
                system_interval = 500;
                process_failure();
                break;
            default:
                bms_status = BMS_FAULT;
                break;
        }
        process_event();
        CyDelay(system_interval);
    }
}

/* [] END OF FILE */
