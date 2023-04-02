/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.7
        Device            :  PIC16F1459
        Driver Version    :  2.00
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#include "mcc_generated_files/mcc.h"

#define BUF_SIZE (0xFF - 1)

typedef struct {
    uint8_t* buffer;
    uint8_t head;
    uint8_t tail;
    uint8_t maxlen;
} circ_bbuf_t;

uint8_t circ_bbuf_push(circ_bbuf_t *c, uint8_t data){
    uint8_t next;

    next = c->head + 1;  // next is where head will point to after this write.
    if (next >= c->maxlen){
        next = 0;
    }

    if (next == c->tail){  // if the head + 1 == tail, circular buffer is full
        return 0xFF;
    }

    c->buffer[c->head] = data;  // Load data and then move
    c->head = next;             // head to next data offset.
    return 0;  // return success to indicate successful push.
}

uint8_t circ_bbuf_pop(circ_bbuf_t *c, uint8_t *data){
    uint8_t next;

    if (c->head == c->tail){  // if the head == tail, we don't have any data
        return 0xFF;
    }

    next = c->tail + 1;  // next is where tail will point to after this read.
    if(next >= c->maxlen){
        next = 0;
    }

    *data = c->buffer[c->tail];  // Read data and then move
    c->tail = next;              // tail to next offset.
    return 0;  // return success to indicate successful push.
}

static uint8_t readBuffer[32];
static uint8_t writeBuffer[32];

uint8_t recvCircBufPhysical[BUF_SIZE];
circ_bbuf_t recvCircBuf;

uint8_t sendCircBufPhysical[BUF_SIZE];
circ_bbuf_t sendCircBuf;

/*void MCC_USB_CDC_DemoTasks(void)
{
    if(USBGetDeviceState() < CONFIGURED_STATE){
        return;
    }

    if(USBIsDeviceSuspended()== true){
        return;
    }

    if(USBUSARTIsTxTrfReady() == true){
        uint8_t i;
        uint8_t numBytesRead;

        numBytesRead = getsUSBUSART(readBuffer, sizeof(readBuffer));
        
        for(i=0; i<numBytesRead; i++){
            switch(readBuffer[i]){
                // echo line feeds and returns without modification. 
                case 0x0A:
                case 0x0D:
                    writeBuffer[i] = readBuffer[i];
                    break;

                // all other characters get +1 (e.g. 'a' -> 'b')
                default:
                    writeBuffer[i] = readBuffer[i] + 1;
                    break;
            }
        }

        if(numBytesRead > 0){
            putUSBUSART(writeBuffer,numBytesRead);
        }
    }

    CDCTxService();
}*/

void writeAddress(uint16_t address){
    TRISC = 0; // set data pins as output
    LATC = (address & 0xFF); // set output to address lower byte
    LAT1_LAT = 1;
    LAT1_LAT = 0; // toggle data into lower byte latch
    LATC = (address >> 8);
    LAT2_LAT = 1;
    LAT2_LAT = 0;
}
uint8_t readByte(uint16_t address){
    writeAddress(address);
    OE_373_LAT = 0; // set output enable for buffers
    TRISC = 0xFF; // set data pins as input
    CE_LAT = 0; // enable EEPROM on
    OE_LAT = 0; // enable EEPROM read on
    uint8_t eepromRead = PORTC;
    OE_LAT = 1;
    CE_LAT = 1;
    OE_373_LAT = 1; // turn off the buffers
    return eepromRead;
}
void writeByte(uint8_t byte, uint16_t address){
    writeAddress(address);
    OE_373_LAT = 0;
    TRISC = 0;
    LATC = byte;
    CE_LAT = 0;
    WE_LAT = 0;
    WE_LAT = 1;
    CE_LAT = 1;
    OE_373_LAT = 0;
}

const char hexArr[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void USBTransfer(){
    if(USBGetDeviceState() < CONFIGURED_STATE){
        return;
    }

    if(USBIsDeviceSuspended() == true){
        return;
    }

    if(USBUSARTIsTxTrfReady() == true){
        uint8_t numBytesRead;

        numBytesRead = getsUSBUSART(readBuffer, sizeof(readBuffer));
        
        for(uint8_t i = 0; i < numBytesRead; i++){
            if(circ_bbuf_push(&recvCircBuf, readBuffer[i])){
                writeBuffer[0] = 'R'; // retry later
                putUSBUSART(writeBuffer, 1);
                return;
            }
        }
        uint8_t i;
        uint8_t temp;
        for(i = 0; i < 32; i++){
            if(circ_bbuf_pop(&sendCircBuf, &temp)){
                break;
            }
            writeBuffer[i] = temp;
        }
        putUSBUSART(writeBuffer, i);
    }
    CDCTxService();
}

void main(void)
{
    // initialize the device
    SYSTEM_Initialize();
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();
    
    recvCircBuf.buffer = recvCircBufPhysical;
    recvCircBuf.head = 0;
    recvCircBuf.tail = 0;
    recvCircBuf.maxlen = BUF_SIZE;
    
    sendCircBuf.buffer = sendCircBufPhysical;
    sendCircBuf.head = 0;
    sendCircBuf.tail = 0;
    sendCircBuf.maxlen = BUF_SIZE;

    uint8_t temp;
    uint8_t temp2;
    uint16_t address = 0;
    bool argsMode = false;
    uint8_t arg = 0;
    uint8_t numBytesRead = 0;
    while (1)
    {
        while(!circ_bbuf_pop(&recvCircBuf, &temp)){
            if(argsMode){
                if(arg == 'm'){
                    address++;
                    writeByte(temp, address);
                    argsMode = false;
                }
                if(arg == 'r'){
                    switch(numBytesRead){
                    case 0:
                        address = temp;
                        address <<= 8;
                        break;
                    case 1:
                        address += temp;
                        temp = readByte(address);
                        circ_bbuf_push(&sendCircBuf, hexArr[temp >> 4]);
                        circ_bbuf_push(&sendCircBuf, hexArr[temp & 0xF]);
                        /*writeBuffer[0] = hexArr[temp >> 4];
                        writeBuffer[1] = hexArr[temp & 0xF];
                        writeBuffer[2] = ' ';
                        writeBuffer[3] = hexArr[(address >> 12) & 0xF];
                        writeBuffer[4] = hexArr[(address >> 8) & 0xF];
                        writeBuffer[5] = hexArr[(address >> 4) & 0xF];
                        writeBuffer[6] = hexArr[address & 0xF];
                        writeBuffer[7] = '\n';
                        writeBuffer[8] = '\r';
                        putsUSBUSART(writeBuffer, 9);*/
                        argsMode = false;
                        break;
                    }
                    numBytesRead++;
                    if(numBytesRead == 2){
                        numBytesRead = 0;
                    }
                }
                if(arg == 'w'){
                    switch(numBytesRead){
                        case 0:
                        address = temp;
                        address <<= 8;
                        break;
                    case 1:
                        address += temp;
                        break;
                    case 2:
                        writeByte(temp, address);
                        argsMode = false;
                        break;
                    }
                    numBytesRead++;
                    if(numBytesRead == 3){
                        numBytesRead = 0;
                    }
                }
                
            }else{
                switch(temp){ // from host's perspective:
                    case 'r': // send 2 byte address, receive 2 byte hex
                    case 'w': // send 2 byte address, send 1 byte data
                    case 'm': // send 1 byte hex
                        arg = temp;
                        argsMode = true;
                        break;
                    case 'n': // receive 1 byte hex
                        address++;
                        temp = readByte(address);
                        circ_bbuf_push(&sendCircBuf, hexArr[temp >> 4]);
                        circ_bbuf_push(&sendCircBuf, hexArr[temp & 0xF]);
                        break;
                }
            }
        }
        USBTransfer();
    }
}
