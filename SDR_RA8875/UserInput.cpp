/*
*   UserInput.cpp
*/

#include "SDR_RA8875.h"
#include "RadioConfig.h"

#define BUTTON_TOUCH    10  // distance in pixels that defines a button vs a gesture. A drag and gesture will be > this value.
//#define MAXTOUCHLIMIT    2  //1...5

Metro gesture_timer=Metro(300);  // Change this to tune the button press timing. A drag will be > than this time.
extern Metro popup_timer; // used to check for popup screen request

// Our  extern declarations. Mostly needed for button activities.
#ifdef USE_RA8875
	extern RA8875 tft;
#else 
	extern RA8876_t3    tft;
    extern FT5206       cts;
    uint8_t registers[FT5206_REGISTERS];
#endif
//extern int16_t spectrum_preset;   // Specify the default layout option for spectrum window placement and size.
extern void RampVolume(float vol, int16_t rampType);
//extern void Spectrum_Parm_Generator(int);
//extern struct Spectrum_Parms Sp_Parms_Def[];
extern uint8_t curr_band;   // global tracks our current band setting.  
extern uint32_t VFOA;  // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
extern uint32_t VFOB;
extern struct Band_Memory bandmem[];
extern struct User_Settings user_settings[];
extern const struct Filter_Settings filter[];
extern struct Standard_Button std_btn[];
extern struct Label labels[];
extern uint8_t user_Profile;
extern AudioControlSGTL5000 codec1;
extern uint8_t popup;
extern void set_MF_Service(uint8_t client_name);
extern struct Frequency_Display disp_Freq[];
extern AudioSynthWaveformSine_F32 sinewave1; // for audible alerts like touch beep confirmations
extern void touchBeep(bool enable);
extern bool MeterInUse;  // S-meter flag to block updates while the MF knob has control
extern bool MF_default_is_active;
extern void MF_Service(int8_t counts, uint8_t knob);
extern uint8_t MF_client;  // Flag for current owner of MF knob services

// Function declarations
void Button_Handler(int16_t x, uint16_t y); 
uint8_t Gesture_Handler(uint8_t gesture);
void setPanel(void);
void Touch( void);
void zero_coordinates(void);

// structure to record the touch event info used to determine if there is a button press or a gesture.
struct Touch_Control{            
    //uint32_t  touch_start;  // using metro timer instead
    //uint32_t  elapsed_time;  // recalc this each time it is read. Make=O for start of events
    uint16_t    start_coordinates[MAXTOUCHLIMIT][2]; // start of event location
    uint16_t    temp_coordinates[MAXTOUCHLIMIT][2]; // start of event location
    uint16_t    last_coordinates[MAXTOUCHLIMIT][2];   // updated to curent or end of event location
int16_t         distance[MAXTOUCHLIMIT][2];  // signed value used for direction.  5 touch points with X and Y values each.
} static touch_evt;   // create a static instance of the structure to remember between events

// computation variables
int16_t T1_X = 0;  
int16_t T1_Y = 0;   

int16_t T1_X_last = 0;
int16_t T1_Y_last = 0;

int16_t t1_x_s = 0;
int16_t t1_y_s = 0;
int16_t t2_x_s = 0;
int16_t t2_y_s = 0;

int16_t t1_x_e = 0;
int16_t t1_y_e = 0;
int16_t t2_x_e = 0;
int16_t t2_y_e = 0;

uint8_t holdtime = 0;
uint8_t dragEvent = 0;

COLD uint8_t Touched()
{
    #ifdef USE_RA8875
        return tft.touched();
    #else
        return cts.touched();
    #endif 
}

COLD void touch_update()
{
    #ifdef USE_RA8875
        tft.updateTS();
    #else  // FT5206/5213
        cts.getTSregisters(registers);
    #endif
}

COLD uint8_t get_Touches()
{
    #ifdef USE_RA8875
        return tft.getTouches(); 
    #else  // FT5206/5213
        return cts.getTScoordinates(touch_evt.temp_coordinates, registers);
    #endif
}
//
// _______________________________________ Touch() ____________________________
// 
// Broker for touch events.  Determines if there is a valid button press or gesture and calls the appropriate function
//
//      Input:  None.  Assumes the FT5206 touch controller was started in setup()
//     Output:  Calls Button_Handler() or Gesture_Handler()  
// 
COLD void Touch( void)
{
    uint8_t current_touches = 0;
    static uint8_t previous_touch = 0;

//#define DBG_GESTURE

#ifdef DBG_GESTURE
    int16_t x1, y1;
#endif
    int16_t  x, y, i;

    struct Standard_Button *ptr = std_btn; // pointer to standard button layout table

    if (Touched())
    {      
        touch_update();
        current_touches = get_Touches();
        //Serial.print("Gesture Register=");  // used to test if controller gesture detection really works. It does not do very well.
        //Serial.println(tft.getGesture());

        // Start a state engine.  There are 4 states to track here. 
        // 1. Invalid touch event (not pressed hard enough or long enough).  
        //          current_touches = 0 &&  previous_touch = 0.
        // 2. Valid touch event started, finger(s) in contact. 
        //          current_touches = 1 &&  previous_touch = 0
        //   2a. Store event time start and coordinates into a structure
        //   2b. Set previous_touch = 1.
        // 3. Valid Touch pending, finger(s) still in contact
        //          current_touches = 1 &&  previous_touch = 1
        //   3a. If a timeout occurs, toss the results and start over. Set previous_touch = 0.
        //   3b. If not timed out yet, return with value = distance from last change and time elapsed.
        // Exception: If a slider is active, then report movement to the calling functions so they can do real time adjustments.  
        //      Examples include tuning, volume up and down, brightness adjust, attenuation adjust and so on.
        // 4. Valid touch completed, finger(s) lifted
        //   4a. If only 1 touch, coordinates and have not moved far, it must be a button press, not a swipe.
        //   4b. If only 1 touch, coordinates have moved far enough, must be a swipe.
        //   4c. If 2 touches, coordinates have not moved far enough, then set previous_touches to 0 and return, false alarm.
        //   4d. If 2 touches, coordinates have moved far enough, now direction can be determined. It must be a pinch gesture.
        
        // STATE 1
        if (!current_touches && !previous_touch)
            return;  // nothing to do, nothing pending, invalid touch event. Try pressing longer and/or harder.

        // STATE 2
        if (current_touches && !previous_touch)
        {
            //   2.  A valid touch has started.
            //   2a. Store event time start (or reset timer) and coordinates into a structure
            //   2b. Set previous_touch = 1.                        
            previous_touch = current_touches;  // will be 1 for buttons, 2 for gestures
            touch_update();    // get current facts  
            #ifdef USE_RA8875     
                tft.getTScoordinates(touch_evt.start_coordinates);  // Store the starting coordinates into the structure
                tft.getTScoordinates(touch_evt.last_coordinates);  // Easy way to effectively zero out the last coordinates
            #else
                cts.getTScoordinates(touch_evt.start_coordinates, registers);  // Store the starting coordinates into the structure
                cts.getTScoordinates(touch_evt.last_coordinates, registers);  // Easy way to effectively zero out the last coordinates
            #endif
            
            #ifdef DBG_GESTURE
            //touch_evt.distanceX = touch_evt.distanceY = 0; // reset distance to 0            
            for (i = 0; i< current_touches; i++)   /// Debug info
            {
                #ifndef TOUCH_ROTATION 
                    x = touch_evt.start_coordinates[i][0];
                    y = touch_evt.start_coordinates[i][1];
                    x1 = touch_evt.last_coordinates[i][0];
                    y1 = touch_evt.last_coordinates[i][1];
                    #else
                    x = tft.width() -  touch_evt.start_coordinates[i][0];
                    y = tft.height() - touch_evt.start_coordinates[i][1];
                    x1 = tft.width() -  touch_evt.last_coordinates[i][0];
                    y1 = tft.height() - touch_evt.last_coordinates[i][1];
                #endif
            
                //Serial.print("Touch START time="); Serial.print(touch_evt.touch_start); 
                Serial.print(F("\nState 2 START #=")); Serial.print(i);
                Serial.print(F(" x=")); Serial.print(x);
                Serial.print(F(" x1=")); Serial.print(x1);
                Serial.print(F("  y=")); Serial.print(y);                 
                Serial.print(F(" y1=")); Serial.println(y1);        
                 
            }
            #endif  //  DBG_GESTURE 
            //touch_evt.touch_start = millis();
            //touch_evt.elapsed_time = 0;  // may not need this, use metro timer instead

            // check for 0,0 empty touch event and restart if so.
            if (touch_evt.start_coordinates[0][0] == 0 && touch_evt.start_coordinates[0][1] == 0)
                return;
            dragEvent = 0;   // Assume this is a drag until it is not.
            gesture_timer.reset();   // reset timer
            return;
        }

        // STATE 3
        if (current_touches && previous_touch)
        {
        //   3. Valid Touch pending, finger(s) still in contact
        //          current_touches = 1 &&  previous_touch = 1
        //   3a. If a timeout occurs, toss the results and start over. Set previous_touch = 0.
        //   3b. If not timed out yet, return with value = distance set from last change and time elapsed.
        // Exception: If a slider is active, then report movement to the calling functions so thaty may do ral time adjsutments.  
        //      Examples include tuning, volume up and down, brightness adjust, attenuation adjust and so on.
            touch_update(); 
            #ifdef USE_RA8875
                tft.getTScoordinates(touch_evt.last_coordinates);  // Update current coordinates
            #else  // USE_RA8875
                cts.getTScoordinates(touch_evt.last_coordinates, registers);  // Update current coordinates
            #endif  // USE_RA8875

            #ifdef DBG_GESTURE    
            for (i = 0; i< current_touches; i++)   /// Debug info
            {
                #ifndef TOUCH_ROTATION
                    x = touch_evt.start_coordinates[i][0];
                    y = touch_evt.start_coordinates[i][1];
                    x1 =touch_evt.last_coordinates[i][0];
                    y1 =touch_evt.last_coordinates[i][1];
                #else
                    x = tft.width() -  touch_evt.start_coordinates[i][0];
                    y = tft.height() - touch_evt.start_coordinates[i][1];
                    x1 = tft.width() -  touch_evt.last_coordinates[i][0];
                    y1 = tft.height() - touch_evt.last_coordinates[i][1];
                #endif

                //Serial.print("Touch START time="); Serial.print(touch_evt.touch_start); 
                Serial.print(F("State 3 START #=")); Serial.print(i);
                Serial.print(F(" x=")); Serial.print(x);
                Serial.print(F(" x1=")); Serial.print(x1);
                Serial.print(F("  y=")); Serial.print(y);                 
                Serial.print(F(" y1=")); Serial.println(y1);              
            }
            #endif // DBG_GESTURE   
            
            /// If the coordinates have moved far enough, it is a gesture not a button press.  
            //  Store the disances for touch even 0 now.
            x = touch_evt.distance[0][0] = (touch_evt.last_coordinates[0][0] - touch_evt.start_coordinates[0][0]);            
            y = touch_evt.distance[0][1] = (touch_evt.last_coordinates[0][1] - touch_evt.start_coordinates[0][1]);
            #ifdef DBG_GESTURE 
                Serial.print(F("Drag Event =")); Serial.print(dragEvent);
                Serial.print(F("  Distance 1 x=")); Serial.print(touch_evt.distance[0][0]); 
                Serial.print(F(", y=")); Serial.println(touch_evt.distance[0][1]); 
            #endif
        
            if (gesture_timer.check() == 1)
            {
                //Serial.print(F("Touch Timer expired"));
                holdtime += 1;  // count the number of timer periods the button was presses and held for drag and press and hold feature
                //Serial.print(F("  New holdtime is ")); Serial.println(holdtime);
            }

            if (!dragEvent && abs(x) < BUTTON_TOUCH && abs(y) < BUTTON_TOUCH) // filter out long press touch events
            {
                dragEvent = 0;
            }
            else if (holdtime)   //  This is now a drag event
            {                               
                //i = SPECTUNE_BTN;  // limit drag gestures to certain areas
                //if((x > (ptr+i)->bx && x < (ptr+i)->bx + (ptr+i)->bw) && ( y > (ptr+i)->by && y < (ptr+i)->by + (ptr+i)->bh))
               // {
                    //if ((ptr+i)->show)
                   // { 
                        dragEvent = Gesture_Handler(previous_touch);
                        dragEvent = 1; 
                   // }
                //}
            }
            //previous_touch = 0;// Our timer has expired
            return;  // can calc the distance once we hav a valid event;                                           
        }

        // STATE 4
        if (!current_touches && previous_touch)   // Finger lifted, previous_touch knows if one or 2 touch points
        {
        // 4. Valid touch completed, finger(s) lifted
        //   4a. If only 1 touch, coordinates and have not moved far, it must be a button press, not a swipe.
        //   4b. If only 1 touch, coordinates have moved far enough, must be a swipe.
        //   4c. If 2 touches, coordinates have not moved far enough, then set previous_touches to 0 and return, false alarm.
        //   4d. If 2 touches, coordinates have moved far enough, now direction can be determined. It must be a pinch gesture.           
            touch_update(); 
            #ifdef USE_RA8875
                tft.getTScoordinates(touch_evt.last_coordinates);  // Update current coordinates
            #else
                cts.getTScoordinates(touch_evt.last_coordinates, registers);  // Update current coordinates
            #endif

            #ifdef DBG_GESTURE
            for (i = 0; i< previous_touch; i++)   /// Debug info
            {
                #ifndef TOUCH_ROTATION
                    x = touch_evt.start_coordinates[i][0];
                    y = touch_evt.start_coordinates[i][1];
                    x1 = touch_evt.last_coordinates[i][0];
                    y1 = touch_evt.last_coordinates[i][1];
                    #else
                    x = tft.width() -  touch_evt.start_coordinates[i][0];
                    y = tft.height() - touch_evt.start_coordinates[i][1];
                    x1 = tft.width() -  touch_evt.last_coordinates[i][0];
                    y1 = tft.height() - touch_evt.last_coordinates[i][1];
                #endif
                
                //Serial.print("Touch START time="); Serial.print(touch_evt.touch_start); 
                Serial.print(F("State 4 START #=")); Serial.print(i);
                Serial.print(F(" x=")); Serial.print(x);
                Serial.print(F(" x1=")); Serial.print(x1);
                Serial.print(F("  y=")); Serial.print(y);                 
                Serial.print(F(" y1=")); Serial.println(y1);                       
            }
            #endif  //  DBG_GESTURE
         
            #ifdef TOUCH_ROTATION  
                touch_evt.start_coordinates[0][0] = tft.width() -  touch_evt.start_coordinates[0][0];   // 1st touch point
                //Serial.print("START x="); Serial.print(touch_evt.start_coordinates[0][0]);
                touch_evt.start_coordinates[0][1] = tft.height() - touch_evt.start_coordinates[0][1];       
                //Serial.print(", y="); Serial.println(touch_evt.start_coordinates[0][1]);

                touch_evt.start_coordinates[1][0] = tft.width() -  touch_evt.start_coordinates[1][0];   // 2nd touch point
                touch_evt.start_coordinates[1][1] = tft.height() - touch_evt.start_coordinates[1][1];

                if(MAXTOUCHLIMIT==3)
                {
                    touch_evt.start_coordinates[2][0] = tft.width() -  touch_evt.start_coordinates[2][0];   // 3rd touch point
                    touch_evt.start_coordinates[2][1] = tft.height() - touch_evt.start_coordinates[2][1];
                }
                
                touch_evt.last_coordinates[0][0] = tft.width() -  touch_evt.last_coordinates[0][0];   // 1st touch point
                //Serial.print("END x="); Serial.print(touch_evt.last_coordinates[0][0]);
                
                touch_evt.last_coordinates[0][1] = tft.height() - touch_evt.last_coordinates[0][1];       
                //Serial.print(", y="); Serial.println(touch_evt.last_coordinates[0][1]);

                touch_evt.last_coordinates[1][0] = tft.width() -  touch_evt.last_coordinates[1][0];   // 2nd touch point
                touch_evt.last_coordinates[1][1] = tft.height() - touch_evt.last_coordinates[1][1];

                if(MAXTOUCHLIMIT==3)
                {
                    touch_evt.last_coordinates[2][0] = tft.width() -  touch_evt.last_coordinates[2][0];   // 3rd touch point
                    touch_evt.last_coordinates[2][1] = tft.height() - touch_evt.last_coordinates[2][1];
                }
            #endif  //  TOUCH_ROTATION

            /// If the coordinates have moved far enough, it is a gesture not a button press.  
            //  Store the disances for touch even 0 now.
            touch_evt.distance[0][0] = (touch_evt.last_coordinates[0][0] - touch_evt.start_coordinates[0][0]);
            touch_evt.distance[0][1] = (touch_evt.last_coordinates[0][1] - touch_evt.start_coordinates[0][1]);
            #ifdef DBG_GESTURE 
                Serial.print(F("Distance 1 x=")); Serial.print(touch_evt.distance[0][0]); 
                Serial.print(F(", y=")); Serial.println(touch_evt.distance[0][1]); 
            #endif

            if (previous_touch == 1)   // A value of 2 is 2 touch points. For button & slide/drag, only 1 touch point is present
            {
                touch_evt.distance[1][0] = 0;   // zero out 2nd touch point
                touch_evt.distance[1][1] = 0;             
            }
            else   // populate the distances for touch point 2
            {
                touch_evt.distance[1][0] = (touch_evt.last_coordinates[1][0] - touch_evt.start_coordinates[1][0]);
                touch_evt.distance[1][1] = (touch_evt.last_coordinates[1][1] - touch_evt.start_coordinates[1][1]);
                #ifdef DBG_GESTURE                      
                    Serial.print(F("Distance 2 x=")); Serial.print(touch_evt.distance[1][0]);
                    Serial.print(F(", y=")); Serial.println(touch_evt.distance[1][1]);
                #endif
            }

            //Serial.print(F(" dragEvent is ")); Serial.println(dragEvent);
            //Serial.print(F(" holdtime is ")); Serial.println(holdtime);
            if (!dragEvent)
            {
                // if only 1 touch and X or Y distance is OK for a button call the button event handler with coordinates
                if (previous_touch == 1 && (abs(touch_evt.distance[0][0]) < BUTTON_TOUCH && abs(touch_evt.distance[0][1]) < BUTTON_TOUCH))
                {
                    Button_Handler(touch_evt.start_coordinates[0][0], touch_evt.start_coordinates[0][1]);  // pass X and Y, and duration
                }
                else // Had 2 touches or 1 swipe touch - Distance was longer than a button touch so must be a swipe
                {
                    //Serial.println("Non drag type gesture - check if allowed");
                    x = touch_evt.start_coordinates[0][0];
                    y = touch_evt.start_coordinates[0][1];
                    i = SPECTUNE_BTN;  // limit gesture to certain areas
                    //Serial.print("Before search x="); Serial.print(x); Serial.print(", y="); Serial.println(y);
                    if((x > (ptr+i)->bx && x < (ptr+i)->bx + (ptr+i)->bw) && ( y > (ptr+i)->by && y < (ptr+i)->by + (ptr+i)->bh))
                    {    
                        //Serial.print("After search x="); Serial.print(x); Serial.print(", y="); Serial.println(y);                    
                        if ((ptr+i)->enabled)
                        {  
                            //Serial.println("Non drag type gesture allowed");
                            Gesture_Handler(previous_touch);   // moved enough to be a gesture
                        }
                    }
                }
            }
            previous_touch = 0;   // Done, reset this for a new event            
            holdtime = 0;
            dragEvent = 0; 
            zero_coordinates();
        }   // End State 4
    }
}

void zero_coordinates(void)
{
    touch_evt.distance[0][0] = 0;   // zero out 1st touch point
    touch_evt.distance[0][1] = 0;       
    touch_evt.distance[1][0] = 0;   // zero out 2nd touch point
    touch_evt.distance[1][1] = 0;
    
    T1_X_last = 0;
    T1_Y_last = 0;
 
    T1_X = touch_evt.distance[0][0] = 0;  
    T1_Y = touch_evt.distance[0][1] = 0;  

    t1_x_s = touch_evt.start_coordinates[0][0] = 0;
    t1_y_s = touch_evt.start_coordinates[0][1] = 0;
    t2_x_s = touch_evt.start_coordinates[1][0] = 0;
    t2_y_s = touch_evt.start_coordinates[1][1] = 0;

    t1_x_e = touch_evt.last_coordinates[0][0] = 0;
    t1_y_e = touch_evt.last_coordinates[0][1] = 0;
    t2_x_e = touch_evt.last_coordinates[1][0] = 0;
    t2_y_e = touch_evt.last_coordinates[1][1] = 0;
}
//
// _______________________________________ Gesture_Handler ____________________________________
//
/*
*   The built in gesture detection rarely works.
*   Only pinch and swipe up are ever reported on my test display and swipe up is a rare event.
*   So we will track the touch point time and coordinates and figure it out on our own.
*
*/
COLD uint8_t Gesture_Handler(uint8_t gesture)
{
    // Get our various coordinates
    T1_X = touch_evt.distance[0][0];  
    T1_Y = touch_evt.distance[0][1];  

    t1_x_s = touch_evt.start_coordinates[0][0];
    t1_y_s = touch_evt.start_coordinates[0][1];
    t2_x_s = touch_evt.start_coordinates[1][0];
    t2_y_s = touch_evt.start_coordinates[1][1];

    t1_x_e = touch_evt.last_coordinates[0][0];
    t1_y_e = touch_evt.last_coordinates[0][1];
    t2_x_e = touch_evt.last_coordinates[1][0];
    t2_y_e = touch_evt.last_coordinates[1][1];

    // If a long event then must be a drag.  
    if (gesture == 1 && dragEvent)   // must be a 1 finger drag       
    {    
        int x = T1_X;
        if (T1_X > 0 && abs(T1_X) > abs(T1_Y))  // x is smaller so must be dragb in the right direction
        {               
            Serial.println(F("Drag RIGHT")); 
            switch (MF_client) {
                case NB_BTN:        x /= 100; break;
                case ATTEN_BTN:     x /= 30;  break;
                case AFGAIN_BTN:
                case RFGAIN_BTN:    x /= 10;  break;                         
                case MFTUNE:        x /= -5;   break;
                case REFLVL_BTN:    x /= 8;  break;
                default: break; 
            }
            MF_Service(x, MF_client);
        }
        else if (T1_X < 0 && abs(T1_X) > abs(T1_Y))  // no, must be the wrong direction !
        {
            Serial.println(F("Drag LEFT")); 
            switch (MF_client) {
                case NB_BTN:        x /= 100; break;
                case ATTEN_BTN:     x /= 30;  break;
                case AFGAIN_BTN:
                case RFGAIN_BTN:    x /= 10;  break;                         
                case MFTUNE:        x /= -5;   break;
                case REFLVL_BTN:    x /= 8;  break;
                default: break; 
            }
            MF_Service(x, MF_client);
        }  
        else if (T1_Y > 0)  // y is smaller so must be drag in UP direction
        {
            Serial.println(F("Drag DOWN"));
            MF_Service(-T1_Y/5, user_settings[user_Profile].encoder2_client);
            //AFgain(T1_Y/10);
            //RefLevel(T1_Y/5);
        }
        else if (T1_Y < 0)  // no, maybe down !
        {
            Serial.println(F("Drag UP")); 
            MF_Service(-T1_Y/5, user_settings[user_Profile].encoder2_client);
            //AFgain(T1_Y/10);
            //RefLevel(T1_Y/5);            
        }

        Serial.print(F("Start Drag X="));Serial.print(T1_X);Serial.print(F("  Drag Y="));Serial.println(T1_Y);
        
        touch_evt.start_coordinates[0][0] = touch_evt.last_coordinates[0][0];
        touch_evt.start_coordinates[0][1] = touch_evt.last_coordinates[0][1];
        
        return 1;
    }
    else if (holdtime)
    {
        dragEvent = 0;
        return 0;
    }
    if (dragEvent)
        return 1;
    
    switch (gesture) 
    {
        ////------------------ SWIPE -------------------------------------------
        case 1:  // only 1 touch so must be a swipe or drag.  Get direction vertical or horizontal
        { 
            //#define DBG_GESTURE

            #ifdef DBG_GESTURE
            Serial.print(" T1_X="); Serial.print(T1_X);
            Serial.print(" T1_Y="); Serial.print(T1_Y);                
            #endif

            ////------------------ SWIPE ----------------------------------------------------////
            if ( abs(T1_Y) > abs(T1_X)) // Y moved, not X, vertical swipe    
            {               
                Serial.println(F("\nSwipe Vertical"));
                ////------------------ SWIPE DOWN  -------------------------------------------
                if (T1_Y > 0)  // y is negative so must be vertical swipe down direction                    
                {                    
                    Serial.println(F(" Swipe DOWN")); 
                    //Serial.println("Band -");
                    changeBands(-1);                                     
                } 
                ////------------------ SWIPE UP  -------------------------------------------
                else
                {
                    Serial.println(F(" Swipe UP"));
                    //Set_Spectrum_RefLvl(1);   // Swipe up    
                    //Serial.println("Band +");
                    changeBands(1);                                     
                }
            } 
            ////------------------ SWIPE LEFT & RIGHT -------------------------------------------////
            else  // X moved, not Y, horizontal swipe
            {
                ////------------------ SWIPE LEFT  -------------------------------------------
                Serial.println(F("\nSwipe Horizontal"));
                if (T1_X < 0)  // x is smaller so must be swipe left direction
                {     
                    selectFrequency(-1);           
                    //Rate(-1);
                    Serial.println(F("Swiped Left"));                    
                }
                ////------------------ SWIPE RIGHT  -------------------------------------------
                else  // or larger so a Swipe Right
                {
                    selectFrequency(1);
                    //Rate(1);
                    Serial.println(F("Swiped Right"));
                }
            }                                 
            break;
        }
        ////------------------ PINCH -------------------------------------------
        case 2: // look for T1 going in opposite directiom compared to T2. If T1-T2 closer to 0 it is a Pinch IN            
        {       
            // Calculate the distance between T1 and T2 at the start
            int16_t dist_start  = sqrt(pow(t2_x_s - t1_x_s, 2) + pow(t2_y_s - t1_y_s, 2));
            int16_t dist_end    = sqrt(pow(t2_x_e - t1_x_e, 2) + pow(t2_y_e - t1_y_e, 2));
            #ifdef DBG_GESTURE
            Serial.print(F("Dist Start =")); Serial.println(dist_start);
            Serial.print(F("Dist End   =")); Serial.println(dist_end);
            #endif
            // Calculate the distance between T1 and T2 at the end   
            if (dist_start - dist_end > 200 )                        
                Set_Spectrum_Scale(-1); // Was a pinch in.  Just pass on direction, the end function can look for distance if needed
            if (dist_end - dist_start > 200)                        
                Set_Spectrum_Scale(1); // was a pinch out   
            if (dist_end - dist_start <= 200  && abs(t1_x_s - t1_x_e) < 200  && abs(t1_y_s - t1_y_e) > 200)
            {
                Serial.println(F("Volume UP"));
                //codec1.volume(bandmem[0].spkr_Vol_last+0.2);  // was 2 finger swipe down
            }
            else if (dist_start - dist_end <= 200)
            {
                Serial.println(F("Volume DOWN"));
                //codec1.volume(bandmem[0].spkr_Vol_last-0.2);  // was 2 finger swipe up
            }
            break;
        }
        case 3: if (abs(T1_Y) > abs(T1_X)) // Y moved, not X, vertical swipe    
        {               
            //Serial.println("\n3 point swipe Vertical");
            ////------------------ 3 FINGER SWIPE DOWN  -------------------------------------------
            if (T1_Y > 0)  // y is negative so must be vertical swipe do
            {                
                AFgain(-1);   //  Increment by +1 for 0-100
                Serial.print(F("3-point Volume DOWN  ")); Serial.println(user_settings[user_Profile].afGain);
            }
            else //---------------- 3 FINGER SWIPE UP  -----------------------------------
            {
                AFgain(1);   //  Range 0 to 100, this is a request to change level by 1 step
                Serial.print(F("3-point Volume UP  ")); Serial.println(user_settings[user_Profile].afGain);
            }                
            break;
        }        
        case 0: // nothing applicable, leave
       default: Serial.println(F(" Gesture = 0 : Should not be here!"));
                return 0;  // leave, should not be here
    }
    return 0;
}

//
// _______________________________________ Button_Handler ____________________________
//
//   Input:     X and Y coordinates for normal button push events.
//  Output:     None.  Acts on specified button.  
//   Usage:     Called from the touch sensor broker Touch().
//
//      Enabled:
//      The enabled.  It was intended to track the state of a button.  In reality most buttons and labels 
//      read direct from another table to determine the feature state and then update to match or most often 
//      is ignored/not used.  There are some cases where a button is all that exists, usually as a placeholder
//      for a future feature so the button may store its own on/off state until the real feature arrives
//      with a place to query that feature status.
//
//      Show:
//      Show is for button or label visiblity.  Must be on for a button or label to be visible on screen
//      In the case of panels of buttons that alternately show and hide, a touch event can be for any of 
//      several buttons occupying the same space. Use the show property to control when to show a button
//      and only pass through touch event to the control function when the button or label is showing.
//      Show applies only to the touch system processing. 
//      Other controllers can call the control function bypassing the touch system.
//      Display functions only report the existing state and do not alter states.  They can update a button
//      text info or label color to match the current state when the object is showing.
//    
//      Use:
//      So how does a touch event get processed here?
//      1. Touch determines it is a 1 point non drag event and passes the X,Y coordinates to a list
//      of XY rectangle definitions here.
//      2. Only 1 object is visible in the touch space at any moment
//      3. Out of sight buttons must be ignored by a touch event.
//      4. A non touch event can call the same control fucntion the touch event does.
//      5. Normally the control event launches a display update calling any linked objects.  It knows best when
//      something has changed.
//      5. The Display functions get called in all cases of change but must check the show status and exit if the object is
//      currently hidden.  It may update an active label but not update a hidden button.
//      6. When a label or button is first drawn (rotate from hidden to show state) it will refresh its status live.
//      7.  A multi-function knob or panel switch or remote command may call a control function and no touch involved.
//      The control and display functions must proceed.
//  
COLD void Button_Handler(int16_t x, uint16_t y)
{
    //Serial.print(F("Button:"));Serial.print(x);Serial.print(" ");Serial.println(y);
    
    if (popup)
        popup_timer.reset();

    struct Standard_Button *ptr = std_btn; // pointer to standard button layout table
    for ( uint16_t i=0; i < STD_BTN_NUM; i++ )
    {
        //Serial.println((ptr+i)->label);
        if((x > (ptr+i)->bx && x < (ptr+i)->bx + (ptr+i)->bw) && ( y > (ptr+i)->by && y < (ptr+i)->by + (ptr+i)->bh))
        {
            if ((ptr+i)->show && holdtime == 0)  // if the show property ius active, call the button function to act on it.
            {   // used the index to the table to match up a function to call
                // feedback beep
                touchBeep(true);  // a timer will shut it off.  
                switch (i)
                {
                    case MODE_BTN:      setMode(1);     break; //Increment the mode from current value
                    case FILTER_BTN:    Filter(0);      break;
                    case RATE_BTN:      Rate(0);        break; //Increment from current value 
                    case AGC_BTN:       AGC();          break;
                    case ANT_BTN:       Ant();          break;                    
                    case MUTE_BTN:      Mute();         break;
                    case MENU_BTN:      Menu();         break;
                    case VFO_AB_BTN:    VFO_AB();       break; // VFO A and B Switching button - Can touch the A/B button or the Frequency Label itself to toggle VFOs
                    case ATTEN_BTN:     Atten(2);       break; // 2 = toggle state, 1 is set, 1 is off, -1 use current
                    case PREAMP_BTN:    Preamp(2);      break; // 2 = toggle state, 1 is set, 1 is off, -1 use current
                    case RIT_BTN:       RIT();          break;
                    case XIT_BTN:       XIT();          break;
                    case SPLIT_BTN:     Split();        break;
                    case XVTR_BTN:      Xvtr();         break;
                    case ATU_BTN:       ATU();          break;
                    case FINE_BTN:      Fine();         break;
                    case XMIT_BTN:      Xmit();         break;
                    case NB_BTN:        NB(2);          break;
                    case NR_BTN:        NR();           break;
                    case ENET_BTN:      Enet();         break;
                    case AFGAIN_BTN:    setAFgain(2);   break;
                    case RFGAIN_BTN:    setRFgain(2);   break;
                    case SPOT_BTN:      Spot();         break;
                    case REFLVL_BTN:    setRefLevel(2); break;
                    case NOTCH_BTN:     Notch();        break;
                    case BANDUP_BTN:    BandUp();       break;
                    case BANDDN_BTN:    BandDn();       break;
                    case BAND_BTN:      Band();         break;
                    case DISPLAY_BTN:   Display();      break;
                    case FN_BTN:        setPanel();     break;
                    case UTCTIME_BTN:   break;        //nothing to do
                    case SMETER_BTN:    setAFgain(2);    break; // TODO toggle through RF and AF
                    default: Serial.print(F("Found a button with SHOW on but has no function to call.  Index = "));
                        Serial.println(i); break;
                }
            } // LONG PRESS
            if ((ptr+i)->show && holdtime > 0)  // if the show property is active, call the button function to act on it.
            {   // used the index to the table to match up a function to call
                // feedback beep
                touchBeep(true);  // a timer will shut it off.
                switch (i)
                {
                    case NB_BTN:        NB(1);          break; //Increment the mode from current value           
                    case AGC_BTN:       AGC();          break;   
                    case ATTEN_BTN:     Atten(1);       break; // 2 = toggle state, 1 is set, 1 is off, -1 use current      
                    case SMETER_BTN:    setRFgain(1);   break;
                    //case AFGAIN_BTN:    setAFgain(1);   break;
                    //case RFGAIN_BTN:    setRFgain(1);   break;
                    default: Serial.print(F("Found a button with SHOW on WITH LONG PRESS but has no function to call.  Index = "));
                       Serial.println(i); break;
                }
            }
            if ((ptr+i)->enabled)    // TOUCHTUNE button - This uses the enabled field so treated on its own
            {
                // feedback beep
                touchBeep(true);  // a timer will shut it off.
                switch (i)
                {
                    case SPECTUNE_BTN: TouchTune(x);    break;
                    default: //Serial.print("Found a button ENABLED but has no function to call: "); 
                        //Serial.println(i); 
                        break;
                }     
            }
        }
    }

    struct Label *pLabel = labels;
    for ( uint16_t i=0; i < LABEL_NUM; i++ )
    {
        //Serial.println((ptr+i)->label);
        if((x > (pLabel+i)->x && x < (pLabel+i)->x + (pLabel+i)->w) && ( y > (pLabel+i)->y && y < (pLabel+i)->y + (pLabel+i)->h))
        {
            if ((pLabel+i)->show)  // if the show property ius active, call the button function to act on it.
            {   
                // feedback beep
                touchBeep(true);  // a timer will shut it off.
                // used the index to the table to match up a function to call
                switch (i)
                {
                    case MODE_LBL:      setMode(1);     break; //Increment the mode from current value
                    case FILTER_LBL:    Filter(0);      break;
                    case RATE_LBL:      Rate(0);        break;
                    case AGC_LBL:       AGC();          break;
                    case ANT_LBL:       Ant();          break;
                    default: break;
                }
            }
        }
    }
    
    // These are special cases
    struct Frequency_Display *ptrAct  = &disp_Freq[0];   // The frequency display areas itself  
    if ((x > ptrAct->bx && x < ptrAct->bx + ptrAct->bw) && ( y > ptrAct->by && y < ptrAct->by + ptrAct->bh))
        VFO_AB();
    
    ptrAct  +=1;   // The frequency display areas itself  
    if ((x > ptrAct->bx && x < ptrAct->bx + ptrAct->bw) && ( y > ptrAct->by && y < ptrAct->by + ptrAct->bh))
        VFO_AB();

    struct Frequency_Display *ptrStby = &disp_Freq[2];   // The frequency display areas itself  
    if ((x > ptrStby->bx && x < ptrStby->bx + ptrStby->bw) && ( y > ptrStby->by && y < ptrStby->by + ptrStby->bh))
        VFO_AB();
        
    ptrStby += 1;   // The frequency display areas itself  
    if ((x > ptrStby->bx && x < ptrStby->bx + ptrStby->bw) && ( y > ptrStby->by && y < ptrStby->by + ptrStby->bh))
        VFO_AB();
}

//
//  -------------------------------------------- SetPanel() ------------------------------------------------------
//
// If the Fn key is touched then we need to sort through the panel field and turn on buttons
//      only for that row or panel. We set the correct set of buttons to (show) and turn off (!show) the rest.
COLD  void setPanel()
{
    struct Standard_Button *ptr = std_btn; // pointer to standard button layout table
    ptr = std_btn + FN_BTN;     // pointer to button object passed by calling function    
    std_btn[FN_BTN].enabled += 1; // increment to the next panel - storing the current panel number in the enabled field.
    if (std_btn[FN_BTN].enabled > PANEL_ROWS-1)     // Normally always have this button showing but may not some day later.                                                        
        std_btn[FN_BTN].enabled = 2;                // Start at 2. 0 and 1 are used for on/off visible status. So 2 == panel 1.

    uint16_t panel = std_btn[FN_BTN].enabled - 1;    // The panel field uses 1  to X so adjust our index down
    Serial.print(F("Fn Pressed, Show Panel "));  Serial.println(panel);
    sprintf(std_btn[FN_BTN].label, "Fn %d", panel);   // Update our button label

    // Turn ON or OFF Panel X buttons
    for ( uint16_t i=1; i < STD_BTN_NUM; i++ ) //skip record 0 which is the the Fn key 
    {
        if ((ptr+i)->Panelnum == panel || (ptr+i)->Panelnum == panel*50 || (ptr+i)->Panelnum == panel*40 )  // if the field has a matching panel number then turn it in for "show" state else turn it off
        {                                                               // panelnum*X is a one off cheat to allow a button to appear in more than 1 row.
            (ptr+i)->show = ON;
            //Serial.print(F("Turning ON "));
            //Serial.println((ptr+i)->label);
        }
        else 
        {
            if ((ptr+i)->Panelnum !=0)   // Anything with panel == 0 is not in a panel so is ignored here.
            {
                (ptr+i)->show = OFF;
                //Serial.print(F("Turning OFF "));
                //Serial.println((ptr+i)->label);
            }
        }
    }
    displayRefresh();   // redraw button to show new ones and hide old ones.  Set arg =1 to skip calling ourself
    //Serial.print("Fn Pressed ");  Serial.println(std_btn[FN_BTN].enabled);
    return;
}

#ifdef IGNORE_ME
    // DISPLAY Test button (hidden area)
    if ((x>700 && x<800)&&(y>300 && y<400))
    {
        // DISPLAY BUTTON  - test usage today for spectum mostly
        //Draw a black box where the old window was
        
        uint8_t s = spectrum_preset;
        tft.fillRect(Sp_Parms_Def[s].spect_x, Sp_Parms_Def[s].spect_y, Sp_Parms_Def[s].spect_width, Sp_Parms_Def[s].spect_height, RA8875_BLACK);  // x start, y start, width, height, array of colors w x h
        spectrum_preset += 1;
        if (spectrum_preset > PRESETS-1)
            spectrum_preset = 0;         
        drawSpectrumFrame(spectrum_preset);
        spectrum_wf_style = Sp_Parms_Custom[spectrum_preset].spect_wf_style;
        displayRefresh();   // redraw the rest of the screen and buttons
        /*
        Sp_Parms_Def[spectrum_preset].spect_wf_colortemp += 10;
        if (Sp_Parms_Def[spectrum_preset].spect_wf_colortemp > 10000)
             Sp_Parms_Def[spectrum_preset].spect_wf_colortemp = 1;              
        Serial.print("spectrum_wf_colortemp = ");
        Serial.println(Sp_Parms_Def[spectrum_preset].spect_wf_colortemp);
        */
        /*
        spectrum_update_set += 1;
        if (spectrum_update_set > 6)
            spectrum_update_set = 0;         
            */
        Spectrum_Parm_Generator(spectrum_preset);  // Generate values for current display (on the fly) or filling in teh ddefauil table for Presets.  value of 0 to PRESETS.
        return;
    }   
#endif
