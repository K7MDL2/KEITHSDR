/*
*   UserInput.h     
*
*   Usage:  A touch event broker that tracks touch point events and uses time, distance, and number of touch points
*           to determine if an event is a button press, a swipe or a pinch activity.  Data is stored in Touch_Control structure.
*           
*           Button: 1 touch point < BUTTON_TOUCH max travel. It passes on the last X and Y coordinates.
*
*           Gesture: 1 or 2 touch points exceeding BUTTON_TOUCH minimum travel. It passes the distance to the gesture handler
*           Direction is positive or negatiove distance fopr X and Y.
*           
*           Non_Blocking: This is a non-blocking state engine with timer "gesture_timer" set for max press duration. 
*           The event is reset after timer expiration.  When a finger is eventually lifted it can become a valid press
*           or gesture again but with new start points. See Dragging description for exception.
*           
*           Dragging: The starting x,y coordinate are stored and a timer started.  While waiting for a press event to complete
*           the current x, y coordinates are updated to permit feedback for dragging type events such as operating a slider.  
*           The starting coordinates are not reset so the total drag distance can still be determined even though the timer has expired.
*
*           Number of touch points can be up to 5 but the code is only working with 2 here.
*/
#include <Metro.h>

#define BUTTON_TOUCH    40  // distance in pixels that defines a button vs a gesture. A drag and gesture will be > this value.
//#define MAXTOUCHLIMIT    2  //1...5

Metro gesture_timer=Metro(700);  // Change this to tune the button press timing. A drag will be > than this time.
extern Metro popup_timer; // used to check for popup screen request

// Our  extern declarations. Mostly needed for button activities.
//extern RA8875   tft;
//extern int16_t spectrum_preset;   // Specify the default layout option for spectrum window placement and size.
extern void RampVolume(float vol, int16_t rampType);
//extern void Spectrum_Parm_Generator(int);
//extern struct Spectrum_Parms Sp_Parms_Def[];
extern uint8_t curr_band;   // global tracks our current band setting.  
extern uint32_t VFOA;  // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
extern uint32_t VFOB;
extern struct Band_Memory bandmem[];
extern struct User_Settings user_settings[];
extern struct Bandwidth_Settings bw[];
extern uint8_t user_Profile;
extern AudioControlSGTL5000 codec1;
extern uint8_t popup;

// The below are fixed numbers based on screen size and other screen object edges
// These will also be be need to declared as extern variables in other files to leverage.
//extern int L_frame_left;
//extern int L_frame_right;
//extern int R_frame_left;
//extern int R_frame_right;
//extern int Bottom_frame;
//extern int Top_frame;
//extern int Center_window_divider_line = 120;   // A line that can be drawn between the two button stack frames if desired.
// These define the button height and stack position and number of buttons, all equal size
//extern int number_of_buttons;
//extern int B_height; // = (Bottom_frame - Top_frame) / number_of_buttons;    // scales the vertical column of buttons on each side, all equal size.
//extern int B_num;        

// Function declarations
void Button_Handler(int16_t x, uint16_t y); 
void Set_Spectrum_Scale(int8_t zoom_dir);
void Set_Spectrum_RefLvl(int8_t zoom_dir);
void Gesture_Handler(uint8_t gesture);
void changeBands(int8_t direction);
void pop_win(uint8_t init);

// structure to record the touch event info used to determine if there is a button press or a gesture.
struct Touch_Control{            
    //uint32_t  touch_start;  // using metro timer instead
    //uint32_t  elapsed_time;  // recalc this each time it is read. Make=O for start of events
    uint16_t    start_coordinates[MAXTOUCHLIMIT][2]; // start of event location
    uint16_t    last_coordinates[MAXTOUCHLIMIT][2];   // updated to curent or end of event location
int16_t         distance[MAXTOUCHLIMIT][2];  // signed value used for direction.  5 touch points with X and Y values each.
} static touch_evt;   // create a static instance of the structure to remember between events

//
// _______________________________________ Touch() ____________________________
// 
// Broker for touch events.  Determines if there is a valid button press or gesture and calls the appropriate function
//
//      Input:  None.  Assumes the FT5206 touch controller was started in setup()
//     Output:  Calls Button_Handler() or Gesture_Handler()  
// 
void Touch( void)
{
    uint8_t current_touches = 0;
    static uint8_t previous_touch = 0;
    uint16_t x,y,i;
  
    if (tft.touched())
    {      
        x = y = 0;
        tft.updateTS();                      
        current_touches = tft.getTouches(); 
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
        //   4d. If 2 touches, cordnates have moved far enough, now direction can be determined. It must be a pinch gesture.
        
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
            tft.updateTS();    // get current facts         
            tft.getTScoordinates(touch_evt.start_coordinates);  // Store the starting coordinates into the structure
            tft.getTScoordinates(touch_evt.last_coordinates);  // Easy way to effectively zero out the last coordinates
            //touch_evt.distanceX = touch_evt.distanceY = 0; // reset distance to 0

            for (i = 0; i< current_touches; i++)   /// Debug info
            {
                x = touch_evt.start_coordinates[i][0];
                y = touch_evt.start_coordinates[i][1];
                //Serial.print("Touch START time="); Serial.print(touch_evt.touch_start); 
                Serial.print(" touch point#="); Serial.print(i);
                Serial.print(" x="); Serial.print(x);
                Serial.print(" y="); Serial.println(y);        
            }
            //touch_evt.touch_start = millis();
            //touch_evt.elapsed_time = 0;  // may not need this, use metro timer instead
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
            
            // Update elapsed time
            if (gesture_timer.check() == 1)
            {
                previous_touch = 0;// Our timer has expired
                Serial.println("Touch Timer expired");
                return;  // can calc the distance once we hav a valid event;
            }
            tft.updateTS();             
            tft.getTScoordinates(touch_evt.last_coordinates);  // Update curent coordinates
            return;           
        }
        // STATE 4
        if (!current_touches && previous_touch)   // Finger lifted, previous_touch knows if one or 2 touch points
        {
        // 4. Valid touch completed, finger(s) lifted
        //   4a. If only 1 touch, coordinates and have not moved far, it must be a button press, not a swipe.
        //   4b. If only 1 touch, coordinates have moved far enough, must be a swipe.
        //   4c. If 2 touches, coordinates have not moved far enough, then set previous_touches to 0 and return, false alarm.
        //   4d. If 2 touches, cordnates have moved far enough, now direction can be determined. It must be a pinch gesture.           
            tft.updateTS();             
            tft.getTScoordinates(touch_evt.last_coordinates);  // Update curent coordinates
            /// If the coordinates have moved far enough, it is a gesture not a button press.  
            //  Store the disances for touch even 0 now.
            touch_evt.distance[0][0] = (touch_evt.last_coordinates[0][0] - touch_evt.start_coordinates[0][0]);
            #ifdef DBG_GESTUREA             
            Serial.print("Distance 1 x="); Serial.println(touch_evt.distance[0][0]); 
            #endif
            touch_evt.distance[0][1] = (touch_evt.last_coordinates[0][1] - touch_evt.start_coordinates[0][1]);
            #ifdef DBG_GESTUREA 
            Serial.print("Distance 1 y="); Serial.println(touch_evt.distance[0][1]); 
            #endif

            if (previous_touch == 1)   // A value of 2 is 2 touch points. For button & slide/drag, only 1 touch point is present
            {
                touch_evt.distance[1][0] = 0;   // zero out 2nd touch point
                touch_evt.distance[1][1] = 0;             
            }
            else   // populate the distances for touch point 2
            {
                touch_evt.distance[1][0] = (touch_evt.last_coordinates[1][0] - touch_evt.start_coordinates[1][0]);
                #ifdef DBG_GESTUREA 
                Serial.print("Distance 2 x="); Serial.println(touch_evt.distance[1][0]); 
                #endif
                touch_evt.distance[1][1] = (touch_evt.last_coordinates[1][1] - touch_evt.start_coordinates[1][1]);
                #ifdef DBG_GESTUREA 
                Serial.print("Distance 2 y="); Serial.println(touch_evt.distance[1][1]); 
                #endif
            }

            // if only 1 touch and X or Y distance is OK for a button call the button event handler with coordinates
            if (previous_touch == 1 && (abs(touch_evt.distance[0][0]) < BUTTON_TOUCH && abs(touch_evt.distance[0][1]) < BUTTON_TOUCH))
            {
                Button_Handler(touch_evt.start_coordinates[0][0],  touch_evt.start_coordinates[0][1]);  // pass X and Y
            }
            else  // Had 2 touches or 1 swipe touch - Distance was longer than a button touch so must be a swipe
            {   
                Gesture_Handler(previous_touch);   // moved enough to be a gesture
            }
            previous_touch = 0;   // Done, reset this for a new event
            touch_evt.distance[0][0] = 0;   // zero out 1st touch point
            touch_evt.distance[0][1] = 0;       
            touch_evt.distance[1][0] = 0;   // zero out 2nd touch point
            touch_evt.distance[1][1] = 0;       
        }   // End State 4
    }
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
void Gesture_Handler(uint8_t gesture)
{
    // Get our various coordinates
    int16_t T1_X = touch_evt.distance[0][0];  
    int16_t T1_Y = touch_evt.distance[0][1];   
    int16_t t1_x_s = touch_evt.start_coordinates[0][0];
    int16_t t1_y_s = touch_evt.start_coordinates[0][1];
    int16_t t2_x_s = touch_evt.start_coordinates[1][0];
    int16_t t2_y_s = touch_evt.start_coordinates[1][1];

    int16_t t1_x_e = touch_evt.last_coordinates[0][0];
    int16_t t1_y_e = touch_evt.last_coordinates[0][1];
    int16_t t2_x_e = touch_evt.last_coordinates[1][0];
    int16_t t2_y_e = touch_evt.last_coordinates[1][1];

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
                //Serial.println("\nSwipe Vertical");
                ////------------------ SWIPE DOWN  -------------------------------------------
                if (T1_Y > 0)  // y is negative so must be vertical swipe down direction                    
                {                    
                    //Serial.println(" Swipe DOWN"); 
                    //Serial.println("Band -");
                    changeBands(-1);                                     
                } 
                ////------------------ SWIPE UP  -------------------------------------------
                else
                {
                    //Serial.println(" Swipe UP");
                    //Set_Spectrum_RefLvl(1);   // Swipe up    
                    //Serial.println("Band +");
                    changeBands(1);                                     
                }
            } 
            ////------------------ SWIPE LEFT & RIGHT -------------------------------------------////
            else  // X moved, not Y, horizontal swipe
            {
                ////------------------ SWIPE LEFT  -------------------------------------------
                //Serial.println("\nSwipe Horizontal");
                if (T1_X < 0)  // x is smaller so must be swipe left direction
                {                
                    selectStep(-1);
                    Serial.println("Swiped Left");                                           
                    return; 
                }
                ////------------------ SWIPE RIGHT  -------------------------------------------
                else  // or larger so a Swipe Right
                {
                    selectStep(1);
                    Serial.println("Swiped Right");             
                    return;    
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
            Serial.print("Dist Start ="); Serial.println(dist_start);
            Serial.print("Dist End   ="); Serial.println(dist_end);
            #endif
            // Calculate the distance between T1 and T2 at the end   
            if (dist_start - dist_end > 200 )                        
                Set_Spectrum_Scale(-1); // Was a pinch in.  Just pass on direction, the end function can look for distance if needed
            if (dist_end - dist_start > 200)                        
                Set_Spectrum_Scale(1); // was a pinch out   
            if (dist_end - dist_start <= 200  && abs(t1_x_s - t1_x_e) < 200  && abs(t1_y_s - t1_y_e) > 200)
            {
                Serial.println("Volume UP");
                //codec1.volume(bandmem[0].spkr_Vol_last+0.2);  // was 2 finger swipe down
            }
            else if (dist_start - dist_end <= 200)
            {
                Serial.println("Volume DOWN");
                //codec1.volume(bandmem[0].spkr_Vol_last-0.2);  // was 2 finger swipe up
            }
            break;
        }
        case 3: if (abs(T1_Y) > abs(T1_X)) // Y moved, not X, vertical swipe    
        {               
            //Serial.println("\n3 point swipe Vertical");
            ////------------------ SWIPE DOWN  -------------------------------------------
            if (T1_Y > 0)  // y is negative so must be vertical swipe do
            {                
                codec1.volume(user_settings[user_Profile].spkr_Vol_last -= 0.2);  // was 3 finger swipe down
                
                Serial.print("3-point Volume DOWN  "); Serial.println(user_settings[user_Profile].spkr_Vol_last);
            }
            else
            {
                codec1.volume(user_settings[user_Profile].spkr_Vol_last += 0.1);  // was 3 finger swipe up
                Serial.print("3-point Volume UP  "); Serial.println(user_settings[user_Profile].spkr_Vol_last);
            }                
            break;
        }        
        case 0: // nothing applicable, leave
       default: Serial.println(" Gesture = 0 : Should not be here!");
                return;  // leave, should not be here
    }
}
//
// _______________________________________ Button_Handler ____________________________
//
//   Input:     X and Y coordinates for normal button push events.
//  Output:     None.  Acts on specified button.  
//   Usage:     Called from the touch sensor broker Touch().
//
void Button_Handler(int16_t x, uint16_t y)
{
    Serial.print("Button:");Serial.print(x);Serial.print(" ");Serial.println(y);
    
    struct Standard_Button *ptr = std_btn; // pointer to standard button layout table

    if (popup)
        popup_timer.reset();

    // MUTE
    ptr = std_btn + MUTE_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        {    
            if (user_settings[user_Profile].spkr_en)
            {
                if (!user_settings[user_Profile].mute)
                {
                    RampVolume(0.0f, 1);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"           
                    user_settings[user_Profile].mute = ON;
                }
                else    
                {    //codec1.muteHeadphone();
                    RampVolume(1.0f, 1);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"                        
                    user_settings[user_Profile].mute = OFF;        
                }
                displayMute();
            }        
            return;
        }
    }

    // MENU button
    ptr = std_btn + MENU_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {   
        if (ptr->show)
        {
            popup = 1;
            pop_win(1);
            Sp_Parms_Def[spectrum_preset].spect_wf_colortemp += 10;
            if (Sp_Parms_Def[spectrum_preset].spect_wf_colortemp > 10000)
                Sp_Parms_Def[spectrum_preset].spect_wf_colortemp = 1;              
            Serial.print("spectrum_wf_colortemp = ");
            Serial.println(Sp_Parms_Def[spectrum_preset].spect_wf_colortemp); 
            displayMenu();
            Serial.println("Menu Pressed");
            return;
        }
    }

    // VFO A and B Switching button - Can touch the A/B button or the Frequency Label itself to toggle VFOs.
    ptr = std_btn + VFO_AB_BTN;     // pointer to button object passed by calling function
    struct Frequency_Display *ptrAct  = &disp_Freq[0];   // The frequency display areas itself
    struct Frequency_Display *ptrStby = &disp_Freq[2];   // The frequency display areas itself
    if ((((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh)) && ptr->show) ||
        ((x > ptrAct->bx && x < ptrAct->bx + ptrAct->bw) && ( y > ptrAct->by && y < ptrAct->by + ptrAct->bh)) ||
        ((x > ptrStby->bx && x < ptrStby->bx + ptrStby->bw) && ( y > ptrStby->by && y < ptrStby->by + ptrStby->bh)))
    {
        //if (user_settings[user_Profile].VFO_last) // || ((x > ptrAct->bx && x < ptrAct->bx + ptrAct->bw) && ( y > ptrAct->by && y < ptrAct->by + ptrAct->bh))) 
        //{       
            if (bandmem[curr_band].VFO_AB_Active == VFO_A)
            {
                bandmem[curr_band].VFO_AB_Active = VFO_B;
            }
            else if (bandmem[curr_band].VFO_AB_Active == VFO_B)
            {
                bandmem[curr_band].VFO_AB_Active = VFO_A;
            }
            VFOA = bandmem[curr_band].vfo_A_last;
            VFOB = bandmem[curr_band].vfo_B_last;
            selectFrequency(0);
            selectMode(0);  // No change to mode, jsut set for active VFO
            displayVFO_AB();
            Serial.print("Set VFO_AB_Active to ");
            Serial.println(bandmem[curr_band].VFO_AB_Active,DEC);
            return;
        //}
    }

    // ATTENUATOR button
    ptr = std_btn + ATTEN_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {   
        if (ptr->show)
        {
            if (bandmem[curr_band].attenuator == ATTEN_ON)
                bandmem[curr_band].attenuator = ATTEN_OFF;
            else if (bandmem[curr_band].attenuator == ATTEN_OFF)
                bandmem[curr_band].attenuator = ATTEN_ON;
            displayAttn();
            Serial.print("Set Attenuator to ");
            Serial.println(bandmem[curr_band].attenuator,DEC);
            return;
        }
    }

    // PREAMP button
    ptr = std_btn + PREAMP_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        {
            if (bandmem[curr_band].preamp == PREAMP_ON)
                bandmem[curr_band].preamp = PREAMP_OFF;
            else if (bandmem[curr_band].preamp == PREAMP_OFF)
                bandmem[curr_band].preamp = PREAMP_ON;
            displayPreamp();
            Serial.print("Set Preamp to ");
            Serial.println(bandmem[curr_band].preamp,DEC);
            return;
        }
    }

    // RIT button
    ptr = std_btn + RIT_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        { 
            if (bandmem[curr_band].RIT_en == ON)
                bandmem[curr_band].RIT_en = OFF;
            else if (bandmem[curr_band].RIT_en == OFF)
                bandmem[curr_band].RIT_en = ON;
            displayRIT();
            Serial.print("Set RIT to ");
            Serial.println(bandmem[curr_band].RIT_en);
            return;
        }
    }
    
    // XIT button
    ptr = std_btn + XIT_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        { 
            if (bandmem[curr_band].XIT_en == ON)
                bandmem[curr_band].XIT_en = OFF;
            else if (bandmem[curr_band].XIT_en == OFF)
                bandmem[curr_band].XIT_en = ON;
            displayXIT();
            Serial.print("Set XIT to ");
            Serial.println(bandmem[curr_band].XIT_en);
            return;
        }
    }
    
    // SPLIT button
    ptr = std_btn + SPLIT_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        {
            if (bandmem[curr_band].split == ON)
                bandmem[curr_band].split = OFF;
            else if (bandmem[curr_band].split == OFF)
                bandmem[curr_band].split = ON;
            displaySplit();
            displayFreq();
            Serial.print("Set Split to ");
            Serial.println(bandmem[curr_band].split);
            return;
        }
    }
    
    // XVTR button
    ptr = std_btn + XVTR_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        {
            if (bandmem[curr_band].xvtr_en== ON)
                bandmem[curr_band].xvtr_en = OFF;
            else if (bandmem[curr_band].xvtr_en == OFF)
                bandmem[curr_band].xvtr_en = ON;
            displayXVTR();
            Serial.print("Set Xvtr Enable to ");
            Serial.println(bandmem[curr_band].xvtr_en);
            return;
        }
    }

    // ATU button
    ptr = std_btn + ATU_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        {
            if (bandmem[curr_band].ATU== ON)
                bandmem[curr_band].ATU = OFF;
            else if (bandmem[curr_band].ATU == OFF)
                bandmem[curr_band].ATU = ON;
            displayATU();
            Serial.print("Set ATU to ");
            Serial.println(bandmem[curr_band].ATU);
            return;
        }
    }
    
    // FILTER button
    ptr = std_btn + FILTER_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        {   
            selectBandwidth(bandmem[curr_band].filter - 1); // Change Bandwidth  - cycle down then back to the top
            displayFilter();
            Serial.print("Set Filter to ");
            Serial.println(bandmem[curr_band].filter);
            return;
        }
    }
  
    // AGC button
    ptr = std_btn + AGC_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        {   
            selectAgc(bandmem[curr_band].agc_mode + 1);            
            Serial.print("Set AGC to ");
            Serial.println(bandmem[curr_band].agc_mode);            
            sprintf(std_btn[AGC_BTN].label, "%s", agc_set[bandmem[curr_band].agc_mode].agc_name);
            displayAgc();
            return;
        }
    }
     
    // MODE button
    ptr = std_btn + MODE_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {   
        //uint8_t mndx;
        if (ptr->show)
        {  
            selectMode(1);   // Increment the mode for the Active VFO 
            /*
        	if (bandmem[curr_band].VFO_AB_Active == VFO_A)  // get Active VFO mode
		        mndx = bandmem[curr_band].mode_A;			
	        else
		        mndx = bandmem[curr_band].mode_B;
            strcpy(std_btn[MODE_BTN].label, Mode[mndx]);            
            */
            Serial.print("Set Mode");           
            displayMode();
            return;
        }
    }

    // RATE button
    ptr = std_btn + RATE_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        {   // TODO: fix this for real
            selectStep(0);   //bandmem[curr_band].ant_sw = ON;            
            Serial.print("Set Rate to ");
            //strcpy(std_btn[RATE_BTN].label, tstep[bandmem[curr_band].tune_step].ts_name); 
            Serial.println(tstep[bandmem[curr_band].tune_step].ts_name);
            displayRate();
            return;
        }
    }

    // Fine button
    ptr = std_btn + FINE_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        extern uint8_t enc_ppr_response;
            
        if (ptr->show)
        {
            if (user_settings[user_Profile].fine== ON)
            {
                user_settings[user_Profile].fine = OFF;
                enc_ppr_response /= 1.4;
            }
            else if (user_settings[user_Profile].fine == OFF)
            {
                user_settings[user_Profile].fine = ON;
                enc_ppr_response *= 1.4;
            }
            selectStep(0);   //bandmem[curr_band].ant_sw = ON;   
            displayFine();
            displayRate();
            
            Serial.print("Set Fine to ");
            Serial.println(user_settings[user_Profile].fine);
            return;
        }
    }

    // ANT button
    ptr = std_btn + ANT_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        {
            if (bandmem[curr_band].ant_sw== 1)
                bandmem[curr_band].ant_sw = 2;
            else if (bandmem[curr_band].ant_sw == 2)
                bandmem[curr_band].ant_sw = 1;
            displayANT();
            Serial.print("Set Ant Sw to ");
            Serial.println(bandmem[curr_band].ant_sw);
            return;
        }
    }
    
    // XMIT button
    ptr = std_btn + XMIT_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        {
            if (user_settings[user_Profile].xmit== ON)
                user_settings[user_Profile].xmit = OFF;
            else if (user_settings[user_Profile].xmit == OFF)
                user_settings[user_Profile].xmit = ON;
            displayXMIT();
            displayFreq();
            Serial.print("Set XMIT to ");
            Serial.println(user_settings[user_Profile].xmit);
            return;
        }
    }

    // NB button
    ptr = std_btn + NB_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        {
            if (user_settings[user_Profile].nb_en > NBOFF)
                user_settings[user_Profile].nb_en = NBOFF;
            else if (user_settings[user_Profile].nb_en == NBOFF)
                user_settings[user_Profile].nb_en = NB1;
            displayNB();
            Serial.print("Set NB to ");
            Serial.println(user_settings[user_Profile].nb_en);
            return;
        }
    }
     
    // NR button
    ptr = std_btn + NR_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        {
            if (user_settings[user_Profile].nr_en > NROFF)
                user_settings[user_Profile].nr_en = NROFF;
            else if (user_settings[user_Profile].nr_en == NROFF)
                user_settings[user_Profile].nr_en = NR1;
            displayNR();
            Serial.print("Set NR to ");
            Serial.println(user_settings[user_Profile].nr_en);
            return;
        }
    }

    // Enet button
    ptr = std_btn + ENET_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        {
            if (user_settings[user_Profile].enet_output== ON)
                user_settings[user_Profile].enet_output = OFF;
            else if (user_settings[user_Profile].enet_output == OFF)
                user_settings[user_Profile].enet_output = ON;
            displayEnet();
            Serial.print("Set Ethernet to ");
            Serial.println(user_settings[user_Profile].enet_output);
            return;
        }
    }

    // Spot button
    ptr = std_btn + SPOT_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        {
            if (user_settings[user_Profile].spot== ON)
                user_settings[user_Profile].spot = OFF;
            else if (user_settings[user_Profile].spot == OFF)
                user_settings[user_Profile].spot = ON;
            displaySpot();
            Serial.print("Set Spot to ");
            Serial.println(user_settings[user_Profile].spot);
        // adjust ref Floor   
        Sp_Parms_Def[spectrum_preset].spect_floor += 5;
        if (Sp_Parms_Def[spectrum_preset].spect_floor > -130)
            Sp_Parms_Def[spectrum_preset].spect_floor = -220; 
        //Serial.println(Sp_Parms_Def[spectrum_preset].spect_floor);
    
            return;
        }
    }

    // Notch button
    ptr = std_btn + NOTCH_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        {
            if (user_settings[user_Profile].notch== ON)
                user_settings[user_Profile].notch = OFF;
            else if (user_settings[user_Profile].notch == OFF)
                user_settings[user_Profile].notch = ON;
            displayNotch();
            Serial.print("Set Notch to ");
            Serial.println(user_settings[user_Profile].notch);
            return;
        }
    }

    // BAND UP button
    ptr = std_btn + BANDUP_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        {
            changeBands(1);
            displayBandUp();
            Serial.print("Set Band UP to ");
            Serial.println(bandmem[curr_band].band_num,DEC);
            return;
        }
    }

    // BAND DOWN button
    ptr = std_btn + BANDDN_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        {
            Serial.println("BAND DN");
            changeBands(-1);
            displayBandDn();
            Serial.print("Set Band DN to ");
            Serial.println(bandmem[curr_band].band_num,DEC);
            return;
        }
    }
    
    // BAND button
    ptr = std_btn + BAND_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        {
            popup = 1;
            pop_win(1);
            changeBands(1);  // increment up 1 band for now until the pop up windows buttons and/or MF are working
            displayBand();
            Serial.print("Set Band to ");
            Serial.println(bandmem[curr_band].band_num,DEC);
            return;
        }
    }

    // DISPLAY button
    ptr = std_btn + DISPLAY_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        if (ptr->show)
        {        
            if (Sp_Parms_Def[spectrum_preset].spect_dot_bar_mode)
            {
                display_state = 0;
                Sp_Parms_Def[spectrum_preset].spect_dot_bar_mode = 0;
            }
            else 
            {
                display_state = 1;
                Sp_Parms_Def[spectrum_preset].spect_dot_bar_mode = 1;
            }
            popup = 1;
            pop_win(1);
            displayDisplay();
            Serial.print("Set Display Button to ");
            Serial.println(display_state);
            return;
        }
    }

    // FN button  - Cycles out a list (panel) of buttons
    ptr = std_btn + FN_BTN;     // pointer to button object passed by calling function
    if ((x > ptr->bx && x < ptr->bx + ptr->bw) && ( y > ptr->by && y < ptr->by + ptr->bh))
    {
        std_btn[FN_BTN].enabled += 1;
        if (std_btn[FN_BTN].enabled > PANEL_ROWS-1)
            std_btn[FN_BTN].enabled = 2;

        if (1)  // ptr->enabled)   For now this button is always active.
        {   
            if (std_btn[FN_BTN].enabled == 3)   // Panel 2 buttons
            {
                strcpy(std_btn[FN_BTN].label, "Fn 2\0");
                // Turn OFF Panel 1 buttons
                std_btn[MODE_BTN].show    = OFF;
                std_btn[FILTER_BTN].show  = OFF;
                std_btn[ATTEN_BTN].show   = OFF;
                std_btn[PREAMP_BTN].show  = OFF;
                std_btn[RATE_BTN].show    = OFF;                
                std_btn[BAND_BTN].show    = OFF;
                // Turn ON Panel 2 buttons                
                std_btn[NB_BTN].show      = ON;
                std_btn[NR_BTN].show      = ON;
                std_btn[SPOT_BTN].show    = ON;
                std_btn[NOTCH_BTN].show   = ON;
                std_btn[AGC_BTN].show     = ON;
                std_btn[MUTE_BTN].show    = ON;
                // Turn OFF Panel 3 buttons
                std_btn[MENU_BTN].show    = OFF;
                std_btn[ANT_BTN].show     = OFF;
                std_btn[ATU_BTN].show     = OFF;
                std_btn[XMIT_BTN].show    = OFF;
                std_btn[BANDDN_BTN].show  = OFF;
                std_btn[BANDUP_BTN].show  = OFF;
                // Turn OFF Panel 4 buttons
                std_btn[RIT_BTN].show     = OFF;
                std_btn[XIT_BTN].show     = OFF;
                std_btn[VFO_AB_BTN].show  = OFF;
                std_btn[FINE_BTN].show    = OFF;
                std_btn[DISPLAY_BTN].show = OFF;
                std_btn[SPLIT_BTN].show   = OFF;
            }
            else if (std_btn[FN_BTN].enabled == 4)// Panel 3 buttons
            {           
                strcpy(std_btn[FN_BTN].label, "Fn 3\0");
                // Turn OFF Panel 1 buttons
                std_btn[MODE_BTN].show    = OFF;
                std_btn[FILTER_BTN].show  = OFF;
                std_btn[ATTEN_BTN].show   = OFF;
                std_btn[PREAMP_BTN].show  = OFF;
                std_btn[RATE_BTN].show    = OFF;                
                std_btn[BAND_BTN].show    = OFF;
                // Turn OFF Panel 2 buttons                
                std_btn[NB_BTN].show      = OFF;
                std_btn[NR_BTN].show      = OFF;
                std_btn[SPOT_BTN].show    = OFF;
                std_btn[NOTCH_BTN].show   = OFF;
                std_btn[AGC_BTN].show     = OFF;
                std_btn[MUTE_BTN].show    = OFF;
                // Turn ON Panel 3 buttons
                std_btn[MENU_BTN].show    = ON;
                std_btn[ANT_BTN].show     = ON;
                std_btn[ATU_BTN].show     = ON;
                std_btn[XMIT_BTN].show    = ON;
                std_btn[BANDDN_BTN].show  = ON;
                std_btn[BANDUP_BTN].show  = ON;
                // Turn OFF Panel 4 buttons
                std_btn[RIT_BTN].show     = OFF;
                std_btn[XIT_BTN].show     = OFF;
                std_btn[VFO_AB_BTN].show  = OFF;
                std_btn[FINE_BTN].show    = OFF;
                std_btn[DISPLAY_BTN].show = OFF;
                std_btn[SPLIT_BTN].show   = OFF;
            }
            else if (std_btn[FN_BTN].enabled == 5)// Panel 4 buttons
            {           
                strcpy(std_btn[FN_BTN].label, "Fn 4\0");
                // Turn OFF Panel 1 buttons
                std_btn[MODE_BTN].show    = OFF;
                std_btn[FILTER_BTN].show  = OFF;
                std_btn[ATTEN_BTN].show   = OFF;
                std_btn[PREAMP_BTN].show  = OFF;
                std_btn[RATE_BTN].show    = OFF;                
                std_btn[BAND_BTN].show    = OFF;
                // Turn OFF Panel 2 buttons                
                std_btn[NB_BTN].show      = OFF;
                std_btn[NR_BTN].show      = OFF;
                std_btn[SPOT_BTN].show    = OFF;
                std_btn[NOTCH_BTN].show   = OFF;
                std_btn[AGC_BTN].show     = OFF;
                std_btn[MUTE_BTN].show    = OFF;
                // Turn OFF Panel 3 buttons
                std_btn[MENU_BTN].show    = OFF;
                std_btn[ANT_BTN].show     = OFF;
                std_btn[ATU_BTN].show     = OFF;
                std_btn[XMIT_BTN].show    = OFF;
                std_btn[BANDDN_BTN].show  = OFF;
                std_btn[BANDUP_BTN].show  = OFF;
                // Turn ON Panel 4 buttons
                std_btn[RIT_BTN].show     = ON;
                std_btn[XIT_BTN].show     = ON;
                std_btn[VFO_AB_BTN].show  = ON;
                std_btn[FINE_BTN].show    = ON;
                std_btn[DISPLAY_BTN].show = ON;
                std_btn[SPLIT_BTN].show   = ON; 
            }
            else    // if (std_btn[FN_BTN].enabled == 2)
            {
                strcpy(std_btn[FN_BTN].label, "Fn 1\0");
                // Turn ON Panel 1 buttons
                std_btn[MODE_BTN].show    = ON;
                std_btn[FILTER_BTN].show  = ON;
                std_btn[ATTEN_BTN].show   = ON;
                std_btn[PREAMP_BTN].show  = ON;
                std_btn[RATE_BTN].show    = ON;                
                std_btn[BAND_BTN].show    = ON;
                // Turn OFF Panel 2 buttons                
                std_btn[NB_BTN].show      = OFF;
                std_btn[NR_BTN].show      = OFF;
                std_btn[SPOT_BTN].show    = OFF;
                std_btn[NOTCH_BTN].show   = OFF;
                std_btn[AGC_BTN].show     = OFF;
                std_btn[MUTE_BTN].show    = OFF;
                // Turn OFF Panel 3 buttons
                std_btn[MENU_BTN].show    = OFF;
                std_btn[ANT_BTN].show     = OFF;
                std_btn[ATU_BTN].show     = OFF;
                std_btn[XMIT_BTN].show    = OFF;
                std_btn[BANDDN_BTN].show  = OFF;
                std_btn[BANDUP_BTN].show  = OFF;
                // Turn OFF Panel 4 buttons
                std_btn[RIT_BTN].show     = OFF;
                std_btn[XIT_BTN].show     = OFF;
                std_btn[VFO_AB_BTN].show  = OFF;
                std_btn[FINE_BTN].show    = OFF;
                std_btn[DISPLAY_BTN].show = OFF;
                std_btn[SPLIT_BTN].show   = OFF;              
                //std_btn[ENET_BTN].show    = OFF;          
            }
            displayRefresh();   // redraw button to show new ones and hide old ones.  Set arg =1 to skip calling ourself
            Serial.print("Fn Pressed ");  Serial.println(std_btn[FN_BTN].enabled);
            return;
        }
    }

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
}

// Use gestures (pinch) to adjust the the vertical scaling.  This affects both watefall and spectrum.  YMMV :-)
void Set_Spectrum_Scale(int8_t zoom_dir)
{
    Serial.println(zoom_dir);
    //extern struct Spectrum_Parms Sp_Parms_Def[];    
    if (Sp_Parms_Def[spectrum_preset].spect_wf_scale > 2.0) 
        Sp_Parms_Def[spectrum_preset].spect_wf_scale = 0.5;
    if (Sp_Parms_Def[spectrum_preset].spect_wf_scale < 0.5)
        Sp_Parms_Def[spectrum_preset].spect_wf_scale = 2.0; 
    if (zoom_dir == 1)
    {
        Sp_Parms_Def[spectrum_preset].spect_wf_scale += 0.1;
        Serial.println("ZOOM IN");
    }
    else
    {        
        Sp_Parms_Def[spectrum_preset].spect_wf_scale -= 0.1;
        Serial.println("ZOOM OUT"); 
    }
    Serial.println(Sp_Parms_Def[spectrum_preset].spect_wf_scale);
}

// Use gestures to raise and lower the spectrum reference level relative to the bottom of the window (noise floor)
void Set_Spectrum_RefLvl(int8_t zoom_dir)
{
    Serial.println(zoom_dir);    
    
    if (zoom_dir == 1)
    {
        Sp_Parms_Def[spectrum_preset].spect_floor -= 10;
        Serial.print("RefLvl=UP");
    }        
    else
    {
        Sp_Parms_Def[spectrum_preset].spect_floor += 10;
        Serial.print("RefLvl=DOWN");
    }
    if (Sp_Parms_Def[spectrum_preset].spect_floor < -400)
        Sp_Parms_Def[spectrum_preset].spect_floor = -400; 
    if (Sp_Parms_Def[spectrum_preset].spect_floor > 400)
        Sp_Parms_Def[spectrum_preset].spect_floor = 400;
}
//
//----------------------------------- Skip to Ham Bands only ---------------------------------
//
// Increment band up or down from present.   To be used with touch or physical band UP/DN buttons.
// A alternate method (not in this function) is to use a band button or gesture to do a pop up selection map.  
// A rotary encoder can cycle through the choices and push to select or just touch the desired band.
//
//
// --------------------- Change bands using database -----------------------------------
// Returns 0 if cannot change bands
// Returns 1 if success

void changeBands(int8_t direction)  // neg value is down.  Can jump multiple bandswith value > 1.
{
    
    // TODO search bands column for match toaccount for mapping that does not start with 0 and bands could be in odd order and disabled.
    //Serial.print("\nCurrent Band is "); Serial.println(bandmem[curr_band].band_name);
    bandmem[curr_band].vfo_A_last = VFOA;
    bandmem[curr_band].vfo_B_last = VFOB;

    // Deal with transverters later probably increase BANDS count to cover all transverter bands to (if enabled).
    int8_t target_band = bandmem[curr_band + direction].band_num;
    
    Serial.print("\nTarget Band is "); Serial.println(target_band);

    if (target_band > BAND9)    // go to bottom band
        target_band = BAND9;    // 0 is not used
    if (target_band < BAND0)    // go to top most band  -  
        target_band = BAND0;    // 0 is not used so do not have to adjsut with a -1 here

    Serial.print("\nCorrected Target Band is "); Serial.println(target_band);    
  
//TODO check if band is active and if not, skip down to next until we find one active in the bandmap    
    RampVolume(0.0f, 2);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
    curr_band = target_band;    // Set out new band
    VFOA = bandmem[curr_band].vfo_A_last;
    VFOB = bandmem[curr_band].vfo_B_last;
    Serial.print("New Band is "); Serial.println(bandmem[curr_band].band_name);     
    delay(20);
    selectFrequency(0); 
    selectBandwidth(bandmem[curr_band].filter);
    selectMode(0);  // no change just set for the active VFO
    selectStep(0);
    displayRefresh();
    selectAgc(bandmem[curr_band].agc_mode);
    RampVolume(1.0f, 2);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp" 
}

void pop_win(uint8_t init)
{
    if(init)
    {
        popup_timer.interval(300);
        tft.setActiveWindow(200, 600, 160, 360);
        tft.fillRoundRect(200,160, 400, 200, 20, RA8875_LIGHT_GREY);
        tft.drawRoundRect(200,160, 400, 200, 20, RA8875_RED);
        tft.setTextColor(RA8875_BLUE);
        tft.setCursor(CENTER, CENTER, true);
        tft.print("this is a future keyboard");
        delay(1000);
        tft.fillRoundRect(200,160, 400, 200, 20, RA8875_LIGHT_ORANGE);
        tft.drawRoundRect(200,160, 400, 200, 20, RA8875_RED);
        tft.setCursor(CENTER, CENTER, true);
        tft.print("Thanks for watching, GoodBye!");
        delay(600);
        popup = 0;
   // }
   // else 
   // {
        tft.fillRoundRect(200,160, 400, 290, 20, RA8875_BLACK);
        tft.setActiveWindow();
        popup = 0;   // resume our normal schedule broadcast
        popup_timer.interval(65000);
        drawSpectrumFrame(user_settings[user_Profile].sp_preset);
        displayRefresh();
    }
}
