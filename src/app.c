

#include "app.h"

APP_DATA appData;



void APP_Initialize ( void )
{
    appData.state = INICIALIZA_APLICATIVO;
}



int flagLedToggle = 0;

DRV_HANDLE handleSPI_DRV;
DRV_HANDLE handleUSART_DRV;
DRV_SPI_BUFFER_HANDLE bufHandle; /*Rastreia o status da transação de dados*/
//uint8_t buffer_TX_SPI[] = {0x06,0x02,0x00,0x00,0xAA,0x55};
uint8_t buffer_TX_SPI[10];
uint8_t buffer_RX_SPI[10];
uint8_t buffer_TX_USART[10] = {1,2,3,4,5,6,7,8,9,10};
uint8_t buffer_RX_USART[10] = {0,0,0,0,0,0,0,0,0,0};

//DRV_SST25_BLOCK_COMMAND_HANDLE  blockHandle1, blockHandle2, blockHandle3;

void APP_Tasks ( void )
{
    switch ( appData.state )
    {        
        case INICIALIZA_APLICATIVO:
        {
            bool appInitialized = true;
            appData.adcConvertedValue0 = 0;
            appData.adcConvertedValue4 = 0;
            appData.esperaAN0 = 0;
            appData.esperaAN4 = 0;
            
            if (appInitialized)
            {            
                
                LED_D10Off(); //Inicializa os LEDs apagados
                LED_D09Off();          
                CSOn();      //ChipSelect nao seleciona a memoria (ativa em nivel baixo)           
                DRV_ADC_Open(); //Abre o drive do ADC
                
                uint32_t minhaFreq = 2; //pisca Led em uma frequencia de 1Hz (500ms ligado e 500ms desligado)
                uint32_t FreqClock = DRV_TMR0_CounterFrequencyGet();
                uint32_t contagem = FreqClock/minhaFreq;
                if(!DRV_TMR0_AlarmRegister(contagem,true,0,HeartBeat))
                {                    
                    return;
                }
                
                DRV_TMR0_AlarmEnable(true);
                DRV_TMR0_Start();//Abre o drive da instancia TIMER 0 (usa o timer 1)
                
                appData.spiHandle = DRV_SPI_Open(DRV_SPI_INDEX_0,DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_NONBLOCKING);
                if(appData.spiHandle == DRV_HANDLE_INVALID)
                {
                    while(1);                                
                }
                handleUSART_DRV = DRV_USART_Open(DRV_USART_INDEX_0, DRV_IO_INTENT_READ);
                if (DRV_HANDLE_INVALID == handleUSART_DRV)
                {
                    while(1);
                }
                DRV_USART_ByteReceiveCallbackSet(DRV_USART_INDEX_0, APP_USARTReceiveEventHandler);
                                
                appData.state = LOOP_PRINCIPAL;
            }
            break;
        }

        case LOOP_PRINCIPAL:
        {
            volatile unsigned int tmp = 0 ;
            
            //while(DRV_USART_ReceiverBufferIsEmpty(handleUSART_DRV));
//            if((DRV_USART_TRANSFER_STATUS_RECEIVER_DATA_PRESENT & DRV_USART_TransferStatus(handleUSART_DRV)))
//            {
//                uint8_t b;
//                Nop();
//                b = DRV_USART_ReadByte(handleUSART_DRV);
//                Nop();
//            }
            
            if (flagLedToggle)
            {                
                flagLedToggle = 0;                
                tmp = 0;
                appData.esperaAN0 = 1;
                DRV_ADC_PositiveInputSelect(ADC_MUX_A, ADC_INPUT_POSITIVE_AN0);                
                DRV_ADC_Start();
                while(appData.esperaAN0);
                appData.esperaAN4 = 1;
                DRV_ADC_PositiveInputSelect(ADC_MUX_A, ADC_INPUT_POSITIVE_AN4);                
                DRV_ADC_Start();
                while(appData.esperaAN4 != 0);                                
            }
            static int cont = 0;
            if(!ChaveS3StateGet())
            {
                if(cont < 255)
                {
                    cont++;
                }
                else if (cont == 255)
                {
                    LED_D10Toggle();
                    //DRV_SST25_BlockErase(appData.flashDriver, &blockHandle1, 0x0, 1);
                    //CSOn();
//                    uint8_t b;
//                    //while(DRV_USART_ReceiverBufferIsEmpty(handleUSART_DRV));
//                    while(!(DRV_USART_TRANSFER_STATUS_RECEIVER_DATA_PRESENT & DRV_USART_TransferStatus(handleUSART_DRV)));
//                    b = DRV_USART_ReadByte(handleUSART_DRV);

//                    DRV_USART0_Read( buffer_RX_USART,5);        
//                    DRV_USART0_Write(buffer_TX_USART, 10);                    
                                
                    //EscreveSPI();
                    //LeSPI();                    
                    //bufHandle = DRV_SPI_BufferAddWrite ( handleSPI_DRV, buffer_TX_SPI ,1 , NULL, NULL );
                    //DRV_SST25_BlockErase(handleSST25_DRV,&ApagaBloco,0,1); //Apaga 1 bloco começando pelo endereço 0
                    //while(ChaveS3StateGet());
                    cont++;
                }                
            }
            else
            {
                cont = 0;
            }
            if(appData.adcConvertedValue0 > 1000)
            {
                LED_D8On();
            }
            else
            {
                LED_D8Off();
            }
            if(appData.adcConvertedValue4 > 256)
            {
                LED_D7On();
            }
            else
            {
                LED_D7Off();
            }
            break;
        }
        default:
        {            
            break;
        }
    }
}

/*Callback TIMER1*/
void HeartBeat()
{
    flagLedToggle = 1;
    LED_D09Toggle();
}

void EscreveSPI()
{
    appData.spiWRState = SPI_WR_STATE_WREN;
    appData.spiTXBuffer = &buffer_TX_SPI[0];
    while(appData.spiWRState <= SPI_WR_STATE_WAIT_DATA)
    {
        switch(appData.spiWRState)
        {
            case SPI_WR_STATE_WREN:
            {
                buffer_TX_SPI[0] = 0x06;
                CSOff();
                appData.spiTXBufferHandle = DRV_SPI_BufferAddWrite ( appData.spiHandle, appData.spiTXBuffer ,1 , NULL, NULL );
                appData.spiWRState = SPI_WR_STATE_WAIT_WREN;
                break;
            }
            case SPI_WR_STATE_WAIT_WREN:
            {
                if( DRV_SPI_BUFFER_EVENT_COMPLETE & DRV_SPI_BufferStatus( appData.spiTXBufferHandle ) )
                {
                    appData.spiWRState = SPI_WR_STATE_WR_CMD;
                    CSOn();
                }
                break;
            }
            case SPI_WR_STATE_WR_CMD:
            {
                buffer_TX_SPI[0] = 0x02;
                CSOff();
                appData.spiTXBufferHandle = DRV_SPI_BufferAddWrite ( appData.spiHandle, appData.spiTXBuffer ,1 , NULL, NULL );
                appData.spiWRState = SPI_WR_STATE_WAIT_WR_CMD;
                break;
            }
            case SPI_WR_STATE_WAIT_WR_CMD:
            {
                if( DRV_SPI_BUFFER_EVENT_COMPLETE & DRV_SPI_BufferStatus( appData.spiTXBufferHandle ) )
                {
                    appData.spiWRState = SPI_WR_STATE_ADD_MSB;                    
                }
                break;
            }
            case SPI_WR_STATE_ADD_MSB:
            {
                buffer_TX_SPI[0] = 0x00;                
                appData.spiTXBufferHandle = DRV_SPI_BufferAddWrite ( appData.spiHandle, appData.spiTXBuffer ,1 , NULL, NULL );
                appData.spiWRState = SPI_WR_STATE_WAIT_ADD_MSB;
                break;                
            }
            case SPI_WR_STATE_WAIT_ADD_MSB:
            {
                if( DRV_SPI_BUFFER_EVENT_COMPLETE & DRV_SPI_BufferStatus( appData.spiTXBufferHandle ) )
                {
                    appData.spiWRState = SPI_WR_STATE_ADD_LSB;                    
                }
                break;
            }
            case SPI_WR_STATE_ADD_LSB:
            {
                buffer_TX_SPI[0] = 0x00;                
                appData.spiTXBufferHandle = DRV_SPI_BufferAddWrite ( appData.spiHandle, appData.spiTXBuffer ,1 , NULL, NULL );
                appData.spiWRState = SPI_WR_STATE_WAIT_ADD_LSB;
                break;
                break;
            }
            case SPI_WR_STATE_WAIT_ADD_LSB:
            {
                if( DRV_SPI_BUFFER_EVENT_COMPLETE & DRV_SPI_BufferStatus( appData.spiTXBufferHandle ) )
                {
                    appData.spiWRState = SPI_WR_STATE_DATA;                    
                }
                break;
            }
            case SPI_WR_STATE_DATA:
            {
                buffer_TX_SPI[0] = 0x00;
                buffer_TX_SPI[1] = 0x01;  
                buffer_TX_SPI[2] = 0x02;  
                buffer_TX_SPI[3] = 0x03;  
                buffer_TX_SPI[4] = 0x04;  
                appData.spiTXBufferHandle = DRV_SPI_BufferAddWrite ( appData.spiHandle, appData.spiTXBuffer ,5 , NULL, NULL );
                appData.spiWRState = SPI_WR_STATE_WAIT_DATA;
                break;
            }
            case SPI_WR_STATE_WAIT_DATA:
            {
                if( DRV_SPI_BUFFER_EVENT_COMPLETE & DRV_SPI_BufferStatus( appData.spiTXBufferHandle ) )
                {
                    CSOn();
                    appData.spiWRState++;                    
                }
                break;
            }
        }
    }
}

void LeSPI()
{
    buffer_RX_SPI[0] = 0xFF;
    buffer_RX_SPI[1] = 0xFF;
    buffer_RX_SPI[2] = 0xFF;
    buffer_RX_SPI[3] = 0xFF;
    buffer_RX_SPI[4] = 0xFF;
    
    appData.spiRDState = SPI_RD_STATE_RD_CMD;
    appData.spiTXBuffer = &buffer_TX_SPI[0];
    appData.spiRXBuffer = &buffer_RX_SPI[0];
    
    while(appData.spiRDState <= SPI_RD_STATE_WAIT_DATA)
    {
        switch(appData.spiRDState)
        {
            case SPI_RD_STATE_RD_CMD:
            {
                buffer_TX_SPI[0] = 0x03;
                CSOff();
                appData.spiTXBufferHandle = DRV_SPI_BufferAddWrite ( appData.spiHandle, appData.spiTXBuffer ,1 , NULL, NULL );
                appData.spiRDState = SPI_RD_STATE_WAIT_RD_CMD;
                break;
            }
            case SPI_RD_STATE_WAIT_RD_CMD:
            {
                if( DRV_SPI_BUFFER_EVENT_COMPLETE & DRV_SPI_BufferStatus( appData.spiTXBufferHandle ) )
                {
                    appData.spiRDState = SPI_RD_STATE_ADD_MSB;                    
                }
                break;
            }
            case SPI_RD_STATE_ADD_MSB:
            {
                buffer_TX_SPI[0] = 0x00;                
                appData.spiTXBufferHandle = DRV_SPI_BufferAddWrite ( appData.spiHandle, appData.spiTXBuffer ,1 , NULL, NULL );
                appData.spiRDState = SPI_RD_STATE_WAIT_ADD_MSB;
                break;                
            }
            case SPI_RD_STATE_WAIT_ADD_MSB:
            {
                if( DRV_SPI_BUFFER_EVENT_COMPLETE & DRV_SPI_BufferStatus( appData.spiTXBufferHandle ) )
                {
                    appData.spiRDState = SPI_RD_STATE_ADD_LSB;                    
                }
                break;
            }
            case SPI_RD_STATE_ADD_LSB:
            {
                buffer_TX_SPI[0] = 0x00;                
                appData.spiTXBufferHandle = DRV_SPI_BufferAddWrite ( appData.spiHandle, appData.spiTXBuffer ,1 , NULL, NULL );
                appData.spiRDState = SPI_RD_STATE_WAIT_ADD_LSB;
                break;                
            }
            case SPI_RD_STATE_WAIT_ADD_LSB:
            {
                if( DRV_SPI_BUFFER_EVENT_COMPLETE & DRV_SPI_BufferStatus( appData.spiTXBufferHandle ) )
                {
                    appData.spiRDState = SPI_RD_STATE_DATA;                    
                }
                break;
            }
            case SPI_RD_STATE_DATA:
            {  
                appData.spiRXBufferHandle = DRV_SPI_BufferAddRead ( appData.spiHandle, buffer_RX_SPI ,5 , NULL, NULL );
                appData.spiRDState = SPI_RD_STATE_WAIT_DATA;
                break;
            }
            case SPI_RD_STATE_WAIT_DATA:
            {
                if( DRV_SPI_BUFFER_EVENT_COMPLETE & DRV_SPI_BufferStatus( appData.spiRXBufferHandle ) )
                {
                    CSOn();
                    appData.spiRDState++;                    
                }
                break;
            }
        }
    }
}
void APP_USARTReceiveEventHandler(const SYS_MODULE_INDEX index)
{
    uint8_t b = 0;
    b = DRV_USART_ReadByte(handleUSART_DRV);
    Nop();  
}
