#ifndef _APP_H
#define _APP_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "system_config.h"
#include "system_definitions.h"


typedef enum
{
	/* Application's state machine's initial state. */
	INICIALIZA_APLICATIVO=0,
	LOOP_PRINCIPAL

} APP_STATES;

typedef enum
{
	/* Application's state machine's initial state. */
	SPI_WR_STATE_WREN=0,
    SPI_WR_STATE_WAIT_WREN,        
    SPI_WR_STATE_WR_CMD,
    SPI_WR_STATE_WAIT_WR_CMD,
    SPI_WR_STATE_ADD_MSB,
    SPI_WR_STATE_WAIT_ADD_MSB,
    SPI_WR_STATE_ADD_LSB,
    SPI_WR_STATE_WAIT_ADD_LSB,
    SPI_WR_STATE_DATA,
    SPI_WR_STATE_WAIT_DATA,
} SPI_WR_STATE;

typedef enum
{
	/* Application's state machine's initial state. */       
    SPI_RD_STATE_RD_CMD,
    SPI_RD_STATE_WAIT_RD_CMD,
    SPI_RD_STATE_ADD_MSB,
    SPI_RD_STATE_WAIT_ADD_MSB,
    SPI_RD_STATE_ADD_LSB,
    SPI_RD_STATE_WAIT_ADD_LSB,
    SPI_RD_STATE_DATA,
    SPI_RD_STATE_WAIT_DATA,
} SPI_RD_STATE;




typedef struct
{
    APP_STATES state; //Estados do Aplicativo
    SPI_WR_STATE spiWRState;
    SPI_RD_STATE spiRDState;
    uint8_t esperaAN0; //Diz qdo o valor da conversão do ADC0 está pronto
    uint8_t esperaAN4; //Diz qdo o valor da conversão do ADC4 está pronto
    uint32_t adcConvertedValue0; //Valor convertido do ADC0
    uint32_t adcConvertedValue4; //Valor convertido do ADC4
    DRV_HANDLE spiHandle; //Handle do SPI
    uint8_t *spiTXBuffer; //Buffer de Transmissão da SPI
    uint8_t *spiRXBuffer; //Buffer de Recepção da SPI
    DRV_SPI_BUFFER_HANDLE spiTXBufferHandle; //Handle do buffer TX da SPI
    DRV_SPI_BUFFER_HANDLE spiRXBufferHandle; //Handle do buffer RX da SPI
} APP_DATA;

extern APP_DATA appData;

void APP_Initialize ( void );
void ControleLedADC(); //Controle do LED relacionado ao valor da conversão do ADC.
void HeartBeat();
void EscreveSPI();
void LeSPI();
void APP_Tasks( void );
void APP_USARTReceiveEventHandler(const SYS_MODULE_INDEX index);




#endif /* _APP_H */


//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

