/* Functions to handle all input elements */

// Activate general trace output

//#define TRACE_INPUT 1
//#define TRACE_INPUT_ENCODER 1
//#define TRACE_INPUT_HIGHRES 1

/* Port constants */

volatile const byte switch_pin_list[]={6,    // ENCODER A
                              7,    // ENCODER B
                              8,    // BUTTON A SELECT ( ENCODER PUSH)
                              9     // BUTTON B Snooze
                         //     8     // BUTTON C Alarm Main Switch
                              };  

  const unsigned int debounce_mask[]={  /* every bit is 2 ms */
                              0x0007,    // ENCODER A
                              0x0007,    // ENCODER B
                              0x0007,    // BUTTON A SELECT ( ENCODER PUSH)
                              0xffff     // BUTTON B Snooze
                         //     8     // BUTTON C Alarm Main Switch
                              };

volatile bool setupComplete=false;

#define INPUT_PORT_COUNT sizeof(switch_pin_list)

volatile unsigned int raw_state_register[sizeof(switch_pin_list)];
volatile unsigned int raw_state;
volatile unsigned int debounced_state=0;  /* Debounced state with history to last cycle managed by the ISR */
unsigned int tick_state=0;                /* State provided in the actual tick, with change indication to last tick */

                         
#define INPUT_BITIDX_ENCODER_A 0
#define INPUT_BITIDX_ENCODER_B 2
#define INPUT_BITIDX_BUTTON_A 4
#define INPUT_BITIDX_BUTTON_B 6
/*                                         76543210 */
#define INPUT_ENCODER_A_BITS              0x0003
#define INPUT_ENCODER_A_CLOSED_PATTERN    0x0001
#define INPUT_ENCODER_A_OPENED_PATTERN    0x0002

#define INPUT_ENCODER_B_BITS             0x000c
#define INPUT_ENCODER_B_CLOSED_PATTERN   0x0004
#define INPUT_ENCODER_B_OPENED_PATTERN   0x0008

#define INPUT_ENCODER_AB_BITS            0x000f

#define INPUT_BUTTON_A_BITS              0x0030
#define INPUT_BUTTON_A_PRESSED_PATTERN   0x0010
#define INPUT_BUTTON_A_RELEASED_PATTERN  0x0020

#define INPUT_BUTTON_B_BITS              0x00c0
#define INPUT_BUTTON_B_PRESSED_PATTERN   0x0040
#define INPUT_BUTTON_B_RELEASED_PATTERN  0x0080

/* Variable for reducing cpu usage */
volatile unsigned long input_last_change_time=0;

/* Variables for debounce handling */

#define INPUT_CURRENT_BITS 0x5555
#define INPUT_PREVIOUS_BITS 0xaaaa


//unsigned long stateChangeTs[sizeof(switch_pin_list)];


/* Variables for encoder tracking */

#define ENCODER_IDLE_STATE 0
#define ENCODER_A_FIRST_STATE INPUT_ENCODER_A_CLOSED_PATTERN
#define ENCODER_B_FIRST_STATE INPUT_ENCODER_B_CLOSED_PATTERN

volatile byte encoder_transition_state=0;
volatile int encoder_movement=0;

int input_encoder_value=0;
int input_encoder_rangeMin=0;
int input_encoder_rangeMax=719;
int input_encoder_stepSize=1;
bool input_encoder_change_event=false;
unsigned long last_interaction_time=0;




/* ********************************************************************************************************** */
/*               Interface functions                                                                          */

byte input_selectGotPressed() {
 return (tick_state&INPUT_BUTTON_A_BITS)==INPUT_BUTTON_A_PRESSED_PATTERN; ; /* We switched from unpressed to pressed */;
}

byte input_snoozeGotPressed() {
 return (tick_state&INPUT_BUTTON_B_BITS)==INPUT_BUTTON_B_PRESSED_PATTERN; ; /* We switched from unpressed to pressed */;
}


byte input_getEncoderValue(){
  return input_encoder_value;
}

void input_setEncoderValue(int newValue) {
  input_encoder_value=newValue;
  if(input_encoder_value<input_encoder_rangeMin) input_encoder_value=input_encoder_rangeMin;
  if(input_encoder_value>input_encoder_rangeMax) input_encoder_value=input_encoder_rangeMax;
}

/*  Debug Interface */

byte input_getRawState() {return raw_state;}

byte input_getDebouncedState() {return debounced_state;}

byte input_getTickState() {return tick_state;}

byte input_getRawRegisterLow(byte index) {return lowByte(raw_state_register[index]) ;}

byte input_getRawRegisterHigh(byte index) {return highByte(raw_state_register[index]);}

/* ********************************************************************************************************** */
/*               S E T U P                                                                                    */


void input_setup(int encoderRangeMin, int encoderRangeMax) {
  
  /* Initialize switch pins and debounce timer array */
  for(byte switchIndex=0;switchIndex<INPUT_PORT_COUNT ;switchIndex++) {
       pinMode(switch_pin_list[switchIndex], INPUT_PULLUP);
       //stateChangeTs[switchIndex]=0;  // and initialize timer array
      raw_state_register[switchIndex]=0; 
  }
  pinMode(LED_BUILTIN,OUTPUT);

    // Timer 1
  noInterrupts();           // Alle Interrupts temporär abschalten
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;                // Register mit 0 initialisieren
  OCR1A = 62  ;             // Output Compare für call alle 1 ms https://timer-interrupt-calculator.simsso.de/
//  OCR1A = 50000  ;             // Output Compare für call alle 1 ms https://timer-interrupt-calculator.simsso.de/
  TCCR1B |= (1 << CS12);    // 256 als Prescale-Wert spezifizieren
  TIMSK1 |= (1 << OCIE1A);  // Timer Compare Interrupt aktivieren
  interrupts();             // alle Interrupts scharf schalten  


  /* Initalize the encoder storage */
  input_encoder_rangeMin=encoderRangeMin;
  input_encoder_rangeMax=encoderRangeMax;
  input_encoder_value=encoderRangeMin;
  setupComplete=true;    
}


// Timer Interrupt  will be called every 3 milliseconds
ISR(TIMER1_COMPA_vect)       
 
{
  
  TCNT1 = 0;             // Zähler reset

  
  if(!setupComplete) return;
  
  /* copy debounce of state of last cycle  to history bits */
  debounced_state=(debounced_state&INPUT_CURRENT_BITS)<<1
                 |(debounced_state&INPUT_CURRENT_BITS);
                 
  
  for(byte i=0;i<INPUT_PORT_COUNT;i++) { // for all input ports configured
  
    // Push  actual reading into the raw state registers
    raw_state_register[i]<<=1;
    raw_state_register[i]|=!digitalRead(switch_pin_list[i]);
    bitWrite(raw_state,i*2,raw_state_register[i]&0x0001); /* and the current status bits */
  
    // if raw state is stable  copy it to debounced state
    if((raw_state_register[i]&0x001f)==0x0000) bitClear(debounced_state,i<<1);
    else if((raw_state_register[i]&debounce_mask[i])==debounce_mask[i]) bitSet(debounced_state,i<<1); 
  }

   /* now track the encoder */
  switch(encoder_transition_state) {
      case ENCODER_IDLE_STATE:
              if((debounced_state&INPUT_ENCODER_AB_BITS) 
               == INPUT_ENCODER_A_CLOSED_PATTERN ||
             ((debounced_state&INPUT_ENCODER_AB_BITS)
              ==INPUT_ENCODER_B_CLOSED_PATTERN)) {
              encoder_transition_state=debounced_state&INPUT_ENCODER_AB_BITS;
          };
          break;
           
      case ENCODER_A_FIRST_STATE:
            if(bitRead(debounced_state,INPUT_BITIDX_ENCODER_A)==0  // A is back open 
               && ((debounced_state&INPUT_ENCODER_B_BITS) == INPUT_ENCODER_B_OPENED_PATTERN)){ // B Pin just got opened
               encoder_movement++;
            }           
            break;
            
      case ENCODER_B_FIRST_STATE:
            if(bitRead(debounced_state,INPUT_BITIDX_ENCODER_B)==0  // B is back open 
               && ((debounced_state&INPUT_ENCODER_A_BITS) == INPUT_ENCODER_A_OPENED_PATTERN)){ // A Pin just got opened
                encoder_movement--;
                 
            } 
            break;
    };
            
    /* Reset encoder  transition state, when all debounced 
       states of the encoder contacts are low */
     if((debounced_state&
       INPUT_ENCODER_AB_BITS&
       INPUT_CURRENT_BITS)==0) {
        encoder_transition_state=ENCODER_IDLE_STATE;
    }

    /* Reset last change timer if anything has changed */
    if(debounced_state & INPUT_CURRENT_BITS != (debounced_state&INPUT_PREVIOUS_BITS)>>1) input_last_change_time=millis();

  digitalWrite(LED_BUILTIN,LOW);
}

/* ********************************************************************************************************** */
/* the central scanning function to track the state changes of the buttons and switches                       */

void input_switches_scan_tick() {  

   int tick_encoder_movement=encoder_movement;
          
   if(tick_encoder_movement) {
       input_encoder_change_event=true;
       input_encoder_value+=tick_encoder_movement*input_encoder_stepSize;
       encoder_movement-=tick_encoder_movement; // remove the caputure value from the buffer
       if(input_encoder_value>input_encoder_rangeMax) input_encoder_value=input_encoder_rangeMin;
       else if(input_encoder_value<input_encoder_rangeMin) input_encoder_value=input_encoder_rangeMax;
   }

  /* copy processed debounce state to history bits */
  tick_state=(tick_state&INPUT_CURRENT_BITS)<<1
                 |(debounced_state&INPUT_CURRENT_BITS);
  
} // void input_switches_tick()


