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
#include <LTC6811.h>

void init(void){   //initialize modules
    SPI_Start();  
    //USB_Start(0, USB_5V_OPERATION);
    //USB_CDC_Init();
    //PCAN_Start();
}

int main(void)
{  //SEE ADOW ON DATASHEET PAGE 33
    CyGlobalIntEnable; /* Enable global interrupts. */

    init();   //initialize modules
    
    LTC6811_initialize(MD_FAST);
    LTC6811_init_cfg();

    uint8_t cfga[6];
    uint16_t aux;
    volatile uint16_t volts[16];
    volatile uint16_t i = 0; 
    
    const char8 str[] = "Bitch"; 
    
    uint16_t cell_voltages[2][12];
    
<<<<<<< Updated upstream
    BMS_OK_Write(1);
    
=======
    LTC6811_wrcfga_mux(0, 5);
    LTC6811_wrcfga_discharge_cell(0xFF, 10);
    LTC6811_wrcfga_reset_discharge(0xFF);
    /*
>>>>>>> Stashed changes
    PCAN_TX_MSG message; 
    
    message.dlc = 0; 
    message.id = 1; 
    message.ide = 0; 
    message.irq = 0;

    while(1){
<<<<<<< Updated upstream
        /*
        LTC6811_wakeup();
        CyDelay(1);
        
        LTC6811_adcv();
        CyDelay(1);
        //LTC6811_rdcv(0, 2, cell_voltages);
        LTC6811_rdcv(0, 2, cell_voltages);
        
        LTC6811_wrcfga_balance(1); 
        
        */
        /*
        LTC6811_rdcfga(1, cfga);
        BMS_OK_Write(1);
        for (i=0; i<16; i++){
            LTC6811_wrcfga(1, i, cfga);
            LTC6811_adax();
            CyDelayUs(200);
            LTC6811_rdaux_pin(1, GPIO5, &aux);
            volts[i] = aux;
        }*/

        PCAN_SendMsg(&message); 
        CyDelay(2000);
=======
        //USB_PutString("Bitch\n"); 
        CyDelay(1000);
>>>>>>> Stashed changes
    }
}

/* [] END OF FILE */
