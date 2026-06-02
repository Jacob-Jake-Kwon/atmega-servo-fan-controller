#include <avr/io.h>
#include <util/delay.h>

#define LED_DDR          DDRD
#define LED_PORT         PORTD
#define BUTTON_DDR       DDRG
#define BUTTON_PIN       PING
#define BUTTON_ON        0
#define BUTTON_OFF       1
#define BUTTON_TOGGLE    2
#define BUTTON_4         3

enum { PUSHED, RELEASED };
enum { NO_ACT, ACT_PUSH, ACT_RELEASE };

typedef struct
{
     volatile uint8_t *ddr;       
     volatile uint8_t *pin;       
     uint8_t btnPin;              
     uint8_t prevState;           
} BUTTON;

void Button_Init(BUTTON *button, volatile uint8_t *ddr, volatile uint8_t *pin, uint8_t pinNum)
{
     button->ddr = ddr;
     button->pin = pin;
     button->btnPin = pinNum;
     button->prevState = RELEASED; 
     *button->ddr &= ~(1 << button->btnPin); 
     *(button->ddr + 1) |= (1 << button->btnPin); 
}

uint8_t Button_GetState(BUTTON *button)
{
     uint8_t currState = (*button->pin & (1 << button->btnPin)) ? RELEASED : PUSHED;
     if (currState != button->prevState)
     {
          _delay_ms(15); 
          uint8_t confirmState = (*button->pin & (1 << button->btnPin)) ? RELEASED : PUSHED;
          if (confirmState != button->prevState)
          {
               button->prevState = confirmState; 
               if (confirmState == PUSHED)   return ACT_PUSH;
               if (confirmState == RELEASED) return ACT_RELEASE;
          }
     }
     return NO_ACT; 
}

int main()
{
     DDRE |= (1 << PE5) | (1 << PE3);
     TCCR3A |= (1 << COM3A1) | (1 << COM3C1) | (1 << WGM31);
     TCCR3B |= (1 << WGM33) | (1 << WGM32) | (1 << CS31) | (1 << CS30);
     TCCR3C = 0;
     ICR3 = 4999;    

     LED_DDR = 0xff;

     BUTTON btnOn, btnOff, btnTogg, btn4;
     Button_Init(&btnOn, &BUTTON_DDR, &BUTTON_PIN, BUTTON_ON);
     Button_Init(&btnOff, &BUTTON_DDR, &BUTTON_PIN, BUTTON_OFF);
     Button_Init(&btnTogg, &BUTTON_DDR, &BUTTON_PIN, BUTTON_TOGGLE);
     Button_Init(&btn4, &BUTTON_DDR, &BUTTON_PIN, BUTTON_4);

     // --- State variables for the non-blocking sweep ---
     uint8_t isSweeping = 0;      // 0 = Stopped, 1 = Auto-Sweeping
     int servoPos = 280;          // Tracks current position
     int servoDirection = 1;      // 1 = Moving up, -1 = Moving down

     // Initialize baselines
     OCR3A = 0;     // Fan OFF
     OCR3C = servoPos;

     while (1)
     {
          // --- 1. CHECK BUTTON INPUTS (Instantly responsive!) ---
          
          if (Button_GetState(&btnOn) == ACT_RELEASE)
          {
               LED_PORT = 0xff;
               OCR3A = 0;     // Turn Fan ON full speed
               isSweeping = 0;   // Cancel the automatic sweep mode
               OCR3C = servoPos;      // Return servo to home position

          }

          if (Button_GetState(&btnOff) == ACT_RELEASE) 
          {
               LED_PORT = 0x00;
               OCR3A = 4998;        // Turn Fan completely OFF
          }

          if (Button_GetState(&btnTogg) == ACT_RELEASE) 
          {
               LED_PORT ^= 0xff;
               OCR3A = (OCR3A == 1500) ? 4998 : 1500; 
          }

          // Button 4 toggles the sweep mode state engine on/off
          if (Button_GetState(&btn4) == ACT_RELEASE)
          {
               isSweeping = !isSweeping; 
          }

          // --- 2. EXECUTE CONTINUOUS SWEEP STATE MACHINE ---
          if (isSweeping)
          {
               servoPos += servoDirection; // Step forward or backward by 1 unit
               OCR3C = servoPos;

               // Check boundaries and reverse direction if needed
               if (servoPos >= 475)
               {
                    servoDirection = -1; // Change direction to down
                    _delay_ms(200);      // Tiny pause at the peak edge
               }
               else if (servoPos <= 280)
               {
                    servoDirection = 1;  // Change direction to up
                    _delay_ms(200);      // Tiny pause at the bottom edge
               }

               _delay_ms(15); // Controls the actual speed of the movement sweep
          }
     }
}