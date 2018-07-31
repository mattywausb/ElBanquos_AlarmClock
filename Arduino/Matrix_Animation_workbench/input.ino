/* Functions to handle all input elements */

// Activate general trace output

//#define TRACE_INPUT 1

/* Port constants */

const byte plug_bus_clock_pin=9;
const byte plug_bus_storage_pin=8;
const byte plug_bus_data_pin=7;


const byte switch_pin_list[]={6,    // ENCODER A
                              7,    // ENCODER B
                              8,    // SELECT ( ENCODER PUSH)
                              9     // Snooze
                         //     8     // Alarm Main Switch
                              };  
                         
#define INPUT_BITIDX_ENCODER_A 0
#define INPUT_BITIDX_ENCODER_B 2
#define INPUT_BITIDX_SELECT 4
#define INPUT_BITIDX_SNOOZe 6
/*                                         76543210 */
#define INPUT_ENCODER_A_MASK              B00000011
#define INPUT_ENCODER_A_PRESSED_PATTERN   B00000001
#define INPUT_ENCODER_A_RELEASED_PATTERN  B00000010

#define INPUT_ENCODER_B_MASK              B00001100
#define INPUT_ENCODER_B_PRESSED_PATTERN   B00000100
#define INPUT_ENCODER_B_RELEASED_PATTERN  B00001000

#define INPUT_ENCODER_AB_MASK             B00001111

#define INPUT_SELECT_MASK              B00110000
#define INPUT_SELECT_PRESSED_PATTERN   B00010000
#define INPUT_SELECT_RELEASED_PATTERN  B00100000

#define INPUT_RESULT_MASK              B11000000
#define INPUT_RESULT_PRESSED_PATTERN   B01000000
#define INPUT_RESULT_RELEASED_PATTERN  B10000000

const unsigned long input_debounce_cooldown_interval = 5000; //in microseconds, never check individual state again bevor this time is over
const unsigned long input_scan_interval = 200; //in microseconds, never scan anything state bevore this time is over, 

/* Variable for reducing cpu usage */
unsigned long lastScanTs=0;

/* Variables for debounce handling */

#define INPUT_DEBOUNCED_CURRENT_STATE_MASK B01010101
#define INPUT_DEBOUNCED_PREVIOUS_STATE_MASK B10101010

byte raw_state_previous=0;
byte debounced_state=0;  // can track up to 4 buttons (current at 6420 and previous at 7531) 
unsigned long stateChangeTs[sizeof(switch_pin_list)];


/* Variables for encoder tracking */
#define ENCODER_IDLE_POSITION 0
#define ENCODER_START_WITH_A_PATTERN B00000001
#define ENCODER_START_WITH_B_PATTERN B00000100

byte encoder_transition_state=0;

int input_encoder_value=0;
int input_encoder_rangeMin=0;
int input_encoder_rangeMax=45;




/* ********************************************************************************************************** */
/*               Interface functions                                                                          */

byte input_selectGotPressed() {
 return (debounced_state&INPUT_SELECT_MASK)==INPUT_SELECT_PRESSED_PATTERN; ; /* We switched from unpressed to pressed */;
}

byte input_snoozeGotPressed() {
 return (debounced_state&INPUT_RESULT_MASK)==INPUT_RESULT_PRESSED_PATTERN; ; /* We switched from unpressed to pressed */;
}


byte input_getEncoderValue(){
  return input_encoder_value;
}

void input_setEncoderValue(int newValue) {
  input_encoder_value=newValue;
  if(input_encoder_value<input_encoder_rangeMin) input_encoder_value=input_encoder_rangeMin;
  if(input_encoder_value>input_encoder_rangeMax) input_encoder_value=input_encoder_rangeMax;
}

/* ********************************************************************************************************** */
/*               S E T U P                                                                                    */


void input_setup(int encoderRangeMin, int encoderRangeMax) {
  
  /* Initialize switch pins and debounce timer array */
  for(byte switchIndex=0;switchIndex<sizeof(switch_pin_list);switchIndex++) {
       pinMode(switch_pin_list[switchIndex], INPUT_PULLUP);
       stateChangeTs[switchIndex]=0;  // and initialize timer array
  }
  

  /* Initalize the encoder storage */
  input_encoder_rangeMin=encoderRangeMin;
  input_encoder_rangeMax=encoderRangeMax;
  input_encoder_value=encoderRangeMin;
      
}




/* ********************************************************************************************************** */
/* the central scanning function to track the state changes of the buttons and switches                       */

void input_switches_scan_tick() {  /* After every tick, especially the flank events must be checked, because they will be lost in the next tick */
  byte switchIndex;
  byte bitIndex;
  byte rawRead;
  
  /* copy remembered debounced state to previous debounced state*/
  debounced_state=(debounced_state&INPUT_DEBOUNCED_CURRENT_STATE_MASK)<<1
                 |(debounced_state&INPUT_DEBOUNCED_CURRENT_STATE_MASK);
  
  if (micros() - lastScanTs < input_scan_interval) return;  /* return if it is to early to scan again */ 
  lastScanTs = micros();

  /* Collect raw state an transform it to debounced state */
  for(switchIndex=0;switchIndex<sizeof(switch_pin_list);switchIndex++) {
    bitIndex=switchIndex<<1;
    rawRead=!digitalRead(switch_pin_list[switchIndex]); // Read and reverse bit due to INPUT_PULLUP configuration

    
    if(bitRead(raw_state_previous,bitIndex)!= rawRead) { // we have a flank
      bitWrite(raw_state_previous,bitIndex,rawRead); // remember the new raw state
      stateChangeTs[switchIndex]=micros(); // remember  our time
    } else {  /* no change in raw state */
      if(bitRead(debounced_state,bitIndex)!= rawRead && // but a change against debounced state
         (micros()-stateChangeTs[switchIndex]>input_debounce_cooldown_interval))  // and raw is holding it long enough
          bitWrite(debounced_state,bitIndex,rawRead); // Change our debounce state
    }
  }// For switch index


  /* Track encoder transitions transaction */
 
    switch(encoder_transition_state) {
      case ENCODER_IDLE_POSITION:
          if((debounced_state&INPUT_ENCODER_AB_MASK) 
               == ENCODER_START_WITH_A_PATTERN ||
             ((debounced_state&INPUT_ENCODER_AB_MASK)
              ==ENCODER_START_WITH_B_PATTERN)) {
              encoder_transition_state=debounced_state&INPUT_ENCODER_AB_MASK;
              #ifdef INPUT_TRACE
                Serial.print("T");
              #endif 
          };
          break;
    
      case ENCODER_START_WITH_A_PATTERN:
            if(bitRead(debounced_state,INPUT_BITIDX_ENCODER_A)==0  // A is back open 
               && ((debounced_state&INPUT_ENCODER_B_MASK) == INPUT_ENCODER_B_RELEASED_PATTERN)){ // B Pin just got opened
               if(--input_encoder_value<input_encoder_rangeMin) input_encoder_value=input_encoder_rangeMax; 
            };
            break;
      case ENCODER_START_WITH_B_PATTERN:
            if(bitRead(debounced_state,INPUT_BITIDX_ENCODER_B)==0  // B is back open 
               && ((debounced_state&INPUT_ENCODER_A_MASK) == INPUT_ENCODER_A_RELEASED_PATTERN)){ // A Pin just got opened
                 if(++input_encoder_value>input_encoder_rangeMax) input_encoder_value=input_encoder_rangeMin;
            };
            break;
    };
            
    /* Reset encoder  transition state, when all debounced 
       states of the encoder contacts are low */
    if((debounced_state&
       INPUT_ENCODER_AB_MASK&
       INPUT_DEBOUNCED_CURRENT_STATE_MASK)==0) {
       #ifdef INPUT_TRACE
                if(encoder_transition_state) {Serial.print(input_encoder_value); Serial.println("<--Encoder idle");}
              #endif 
              encoder_transition_state=ENCODER_IDLE_POSITION;

       }

  
} // void input_switches_tick()


