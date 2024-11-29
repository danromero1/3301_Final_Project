/*
 * File:   final_project.c
 * Author: Daniel
 *
 * Created on November 26, 2024, 3:00 PM
 * WBGT = (0.1 * Dry bulb temp) + (0.7 * Natural Wet Bulb Temp) + (0.2 * Globe Temp)
 * Dry Bulb Temp = air temperature 
 * Natural Wet Bulb Temp = Humudity + Wind + Radiation
 * Globe Temperature = Radiation + Wind
 */


#include <xc.h>
#include <stdio.h>
#include "LCD.h"
#include "pic18f4321-Config.h"
#define _XTAL_FREQ 4000000
#define SENSOR_PORT PORTCbits.RC1  //DHT11
#define SENSOR_DIR TRISCbits.TRISC1

void initiate_sensor();
char fetch_byte();
char validate_data(char whole_Major, char decimal_Minor, char whole_Major, char decimal_Minor, char checkSum);
void display_readings(char whole_Humidity, char whole_Temperature);

// Configuration for ADC
void setupADC() {
    ADCON1bits.VCFG1 = 0; // VREF- = VSS (Ground)
    ADCON1bits.VCFG0 = 1; // VREF+ = External (Sensor Output)

    ADCON0bits.CHS = 0;   // Select AN0 as input channel
    ADCON0bits.ADON = 1;  // Turn on ADC module

    ADCON2bits.ADCS = 2;  // ADC clock = FOSC/32
    ADCON2bits.ACQT = 2;  // Acquisition time = 4 TAD
    ADCON2bits.ADFM = 1;  // Right-justified result
}

// Read ADC value
unsigned int readADC() {
    ADCON0bits.GO = 1;          // Start conversion
    while (ADCON0bits.GO);      // Wait for conversion to complete
    return (ADRESH << 8) | ADRESL; // Return 10-bit result
}


void main(void) {
    
    OSCCON = 0x60;  
    ADCON1 = 0x0F;  
    LCD_init();     
    LCD_clear();    
    
    char whole_Humidity, decimal_Humidity, whole_Temperature, decimal_Temperature, checksum;

    setupADC();
    
    unsigned int adcValue;
    float windSpeedVoltage;

    
    const int wbgt; 
    enum { NO = 1, LOW, MODERATE, HIGH, EXTREME }; 
    
    while(1){
        
        adcValue = readADC();
        windSpeedVoltage = (adcValue * 1.2) / 1024.0; // Calculate voltage (assuming VREF+ = 1.2V)
        // Use windSpeedVoltage for calculations or display
        
        initiate_sensor();
        
        while (SENSOR_PORT);    // Wait for signal to go low
        while (!SENSOR_PORT);   // Wait for signal to go high
        while (SENSOR_PORT);    // Wait for signal to go low again
        
        whole_Humidity = fetch_byte();
        decimal_Humidity = fetch_byte();
        whole_Temperature = fetch_byte();
        decimal_Temperature = fetch_byte();
        checksum = fetch_byte();

        if (validate_data(whole_Humidity, decimal_Humidity, whole_Temperature, decimal_Temperature, checksum)) {
            display_readings(whole_Humidity, whole_Temperature);
        } else {
            LCD_clear();
            LCD_cursor_set(1, 1);
            LCD_write_string("Checksum Error");
        }

        __delay_ms(2000);    
        
    }
    
    switch(wbgt) {
        
        //Case where wbgt <= 80 and no flag
        case 1:
            
            LCD_init();
            LCD_cursor_set(1,1);
            LCD_write_string("Heat stress measurement: ");
            
            LCD_cursor_set(2,1);
            LCD_write_string("You in the clear champ. ");
            
            break;
                
        //Case where wbgt = 80 -> 84.9 Low heat category
        case 2:
                     
            LCD_init();
            LCD_cursor_set(1,1);
            LCD_write_string("Heat stress measurement: ");
            
            LCD_cursor_set(2,1);
            LCD_write_string("5 minute water/rest break every 25 min.");
            
            break;
            
        //Case where wbgt = 85 -> 87.9 Moderate category
        case 3:
                        
            LCD_init();
            LCD_cursor_set(1,1);
            LCD_write_string("Heat stress measurement: ");
            
            LCD_cursor_set(2,1);
            LCD_write_string("Moderate: Breaks and Ice Pool.");
            
            break;
            
        //Case where wbgt = 88 - 88.9 High category
        case 4:
                        
            LCD_init();
            LCD_cursor_set(1,1);
            LCD_write_string("Heat stress measurement: ");
            
            LCD_cursor_set(2,1);
            LCD_write_string("High: constant observation.");
            
            break;
            
        //Case where wbgt = Over 90 Extreme category
        case 5:
                        
            LCD_init();
            LCD_cursor_set(1,1);
            LCD_write_string("Heat stress measurement: ");
            
            LCD_cursor_set(2,1);
            LCD_write_string("Not it, champ. Cancel Practice.");
            break;
    }  
    
    return;
}

// Function to initiate communication with the DHT11
void initiate_sensor() {
    SENSOR_DIR = 0;       
    LATCbits.LATC1 = 0;   
    __delay_ms(18);       
    LATCbits.LATC1 = 1;   
    __delay_us(25);       
    SENSOR_DIR = 1;      
}

void display_readings(char whole_Humidity, char whole_Temperature) {
    LCD_cursor_set(1, 6);  
    LCD_write_variable(whole_Humidity, 0);
    LCD_write_string(" %");

    LCD_cursor_set(2, 6);  
    LCD_write_variable(whole_Temperature, 0);
    LCD_write_string(" C");
}

// Function to validate the checksum from the DHT11
char validate_data(char whole_Humidity, char decimal_Humidity, char whole_Temperature, char decimal_Temperature, char checkSum) {
    return (checkSum == (whole_Humidity + decimal_Humidity + whole_Temperature + decimal_Temperature));
}

