#include <avr/io.h>
#include <util/delay.h>

// Hardware configuration macros
#define LED_DDR          DDRD
#define LED_PORT         PORTD
#define BUTTON_DDR       DDRG
#define BUTTON_PIN       PING
#define BUTTON_ON        0
#define BUTTON_OFF       1
#define BUTTON_TOGGLE    2
#define BUTTON_4         3

// Enum for button physical states
enum { PUSHED, RELEASED };

// Enum for button action events
enum
{
    NO_ACT,
    ACT_PUSH,
    ACT_RELEASE
};

// Object-oriented structure to represent a button
typedef struct
{
    volatile uint8_t *ddr;       // Pointer to Data Direction Register
    volatile uint8_t *pin;       // Pointer to Input Pins Address Register
    uint8_t btnPin;              // Specific pin number for the button
    uint8_t prevState;           // Stores the previous stable state
} BUTTON;

/**
 * @brief Initializes the button structure and configures the AVR pin as input.
 */
void Button_Init(BUTTON *button, volatile uint8_t *ddr, volatile uint8_t *pin, uint8_t pinNum)
{
    button->ddr = ddr;
    button->pin = pin;
    button->btnPin = pinNum;
    button->prevState = RELEASED;           // Default state is unpressed (released)
    
    *button->ddr &= ~(1 << button->btnPin); // Configure the pin as INPUT
    
    // Enable Internal Pull-up Resistor to fix floating pins
    *(button->ddr + 1) |= (1 << button->btnPin);
}

/**
 * @brief Debounces the button and detects transition events (Press / Release).
 */
uint8_t Button_GetState(BUTTON *button)
{
    uint8_t currState = (*button->pin & (1 << button->btnPin)) ? RELEASED : PUSHED;
    
    if (currState != button->prevState)
    {
        _delay_ms(15); // Debounce delay
        
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
    // Configure PE3 (OC3A) and PE5 (OC3C) as outputs
    DDRE |= (1 << PE5) | (1 << PE3);
    
    // Configure Timer3 for Fast PWM (Mode 14), Top = ICR3, Prescaler = 64
    TCCR3A |= (1 << COM3A1) | (1 << COM3C1) | (1 << WGM31);
    TCCR3B |= (1 << WGM33) | (1 << WGM32) | (1 << CS31) | (1 << CS30);
    TCCR3C = 0;
    ICR3 = 4999;
    
    // Configure all PORTD pins as outputs for LEDs
    LED_DDR = 0xFF;
    
    // Create button instances
    BUTTON btnOn, btnOff, btnTogg, btn4;
    
    // Initialize buttons
    Button_Init(&btnOn, &BUTTON_DDR, &BUTTON_PIN, BUTTON_ON);
    Button_Init(&btnOff, &BUTTON_DDR, &BUTTON_PIN, BUTTON_OFF);
    Button_Init(&btnTogg, &BUTTON_DDR, &BUTTON_PIN, BUTTON_TOGGLE);
    Button_Init(&btn4, &BUTTON_DDR, &BUTTON_PIN, BUTTON_4);
    
    while (1)
    {
        // Turn all LEDs ON when the ON button is released
        if (Button_GetState(&btnOn) == ACT_RELEASE)
        {
            LED_PORT = 0xFF;
            OCR3A = 0;
            OCR3C = 280;
        }
        
        // Turn all LEDs OFF when the OFF button is released
        if (Button_GetState(&btnOff) == ACT_RELEASE)
        {
            LED_PORT = 0x00;
            OCR3A = 1500;
        }
        
        // Toggle all LEDs when the TOGGLE button is released
        if (Button_GetState(&btnTogg) == ACT_RELEASE)
        {
            LED_PORT ^= 0xFF;
            OCR3A = (OCR3A == 1500) ? 3000 : 1500;
        }
        
        // Button 4: Handle Servo Sweeping Loop
        if (Button_GetState(&btn4) == ACT_RELEASE)
        {
            // Forward sweep
            for (int i = 280; i <= 475; i++)
            {
                OCR3C = i;
                _delay_ms(15);
            }
            
            _delay_ms(500);
            
            // Reverse sweep
            for (int i = 475; i >= 280; i--)
            {
                OCR3C = i;
                _delay_ms(15);
            }
        }
    }
}