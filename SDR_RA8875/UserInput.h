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

Metro gesture_timer=Metro(200);  // Change this to tune the button press timing. A drag will be > than this time.
extern Metro popup_timer; // used to check for popup screen request

// Our  extern declarations. Mostly needed for button activities.
//extern RA8875   tft;
//extern int16_t spectrum_preset;   // Specify the default layout option for spectrum window placement and size.
extern void RampVolume(float vol, int16_t rampType);
//extern void Spectrum_Parm_Generator(int);
//extern struct Spectrum_Parms Sp_Parms_Def[];
extern uint8_t curr_band;   // global tracks our current band setting.  
extern volatile uint32_t VFOA;  // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
extern volatile uint32_t VFOB;
extern struct Band_Memory bandmem[];
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
                    Serial.println("-100KHz");
                    VFOA -= 100000;
                    if (VFOA < bandmem[curr_band].edge_lower) 
                        VFOA = bandmem[curr_band].edge_lower;                    
                    RampVolume(0.0f, 1);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
                    selectFrequency(); 
                    RampVolume(1.0f, 1);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"                     
                    return; 
                }
                ////------------------ SWIPE RIGHT  -------------------------------------------
                else  // or larger so a Swipe Right
                {
                    Serial.println("+100KHz");
                    VFOA += 100000;
                    if (VFOA < bandmem[curr_band].edge_lower) 
                        VFOA = bandmem[curr_band].edge_lower;                    
                    RampVolume(0.0f, 1);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
                    selectFrequency(); 
                    RampVolume(1.0f, 1);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"                     
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
                Serial.println("3-point Volume DOWN");
                codec1.volume(bandmem[0].spkr_Vol_last-0.05);  // was 3 finger swipe down
            }
            else
            {
                Serial.println("3-point Volume UP");
                codec1.volume(bandmem[0].spkr_Vol_last+0.05);  // was 3 finger swipe up
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
    
    if (popup)
        popup_timer.reset();

    // MODE
    if((x>0&&x<100)&&(y>0&&y<50))
    {
        // Select MODE
        selectMode(bandmem[curr_band].mode+1); 
        return;
    }

    // BANDWIDTH
    if((x>120&&x<280)&&(y>0&&y<50))
    {
        // Change Bandwidth  - cycle down then back to the top
        selectBandwidth(bandmem[curr_band].bandwidth - 1);  // send index to bw table
        return; 
    }  

    // TUNE STEP
    if((x>520&&x<680)&&(y>0&&y<50))
    {
        // Change Tune Step Rate - Cycle up then to the bottom 
        selectStep();
        return;
    }

    // SETTINGS (hidden button for testing for now)
    if((x>0&&x<100)&&(y>420&&y<480))
    {
        // Settings Button
        popup = 1;
        pop_win(1);
        Sp_Parms_Def[spectrum_preset].spect_wf_colortemp += 10;
        if (Sp_Parms_Def[spectrum_preset].spect_wf_colortemp > 10000)
             Sp_Parms_Def[spectrum_preset].spect_wf_colortemp = 1;              
        Serial.print("spectrum_wf_colortemp = ");
        Serial.println(Sp_Parms_Def[spectrum_preset].spect_wf_colortemp); 
        return;  
    } 

    // ATTENUATOR button
    if((x>0&&x<100)&&(y>60&&y<100))
    {
        if (bandmem[curr_band].attenuator == ATTEN_ON)
            bandmem[curr_band].attenuator = ATTEN_OFF;
        else if (bandmem[curr_band].attenuator == ATTEN_OFF)
            bandmem[curr_band].attenuator = ATTEN_ON;
        displayAttn(bandmem[curr_band].attenuator);
        Serial.print("Set Attenuator to ");
        Serial.println(bandmem[curr_band].attenuator,DEC);
        return;
    }

    // PREAMP button
    if((x>110&&x<220)&&(y>60&&y<100))
    {
        if (bandmem[curr_band].preamp == PREAMP_ON)
            bandmem[curr_band].preamp = PREAMP_OFF;
        else if (bandmem[curr_band].preamp == PREAMP_OFF)
            bandmem[curr_band].preamp = PREAMP_ON;
        displayPreamp(bandmem[curr_band].preamp);
        Serial.print("Set Preamp to ");
        Serial.println(bandmem[curr_band].preamp,DEC);
        return;
    }

    // AGC button
    if((x>700&&x<800)&&(y>0&&y<50))
    {      
        selectAgc(bandmem[curr_band].agc_mode + 1);
        Serial.print("Set AGC to ");
        Serial.println(bandmem[curr_band].agc_mode,DEC);
        return;
    }

    // DISPLAY Test button (hidden area)
    if((x>700&&x<800)&&(y>440&&y<480))
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
        target_band = BAND0;    // 0 is not used
    if (target_band < BAND0)    // go to top most band  -  
        target_band = BAND9;    // 0 is not used so do not have to adjsut with a -1 here

    Serial.print("\nCorrected Target Band is "); Serial.println(target_band);    
    
//TODO check if band is active and if not, skip down to next until we find one active in the bandmap    
    
    RampVolume(0.0f, 1);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
    curr_band = target_band;    // Set out new band
    VFOA = bandmem[curr_band].vfo_A_last;
    VFOB = bandmem[curr_band].vfo_B_last;
    Serial.print("New Band is "); Serial.println(bandmem[curr_band].band_name);     
    selectFrequency(); 
    selectBandwidth(bandmem[curr_band].bandwidth);
    selectMode(bandmem[curr_band].mode);
    selectStep();
    selectAgc(bandmem[curr_band].agc_mode);
    RampVolume(1.0f, 1);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp" 
}

void pop_win(uint8_t init)
{
    if(init)
    {
        popup_timer.interval(300);
        tft.setActiveWindow(200, 600, 160, 450);
        tft.fillWindow();
        tft.drawRoundRect(200,160, 400, 290, 20, RA8875_RED);
        tft.setTextColor(RA8875_BLUE);
        tft.setCursor(CENTER, CENTER, true);
        tft.print("this is a future keyboard");
        delay(3000);
        tft.fillWindow();
        tft.drawRoundRect(200,160, 400, 290, 20, RA8875_RED);
        tft.setCursor(CENTER, CENTER, true);
        tft.print("Thanks for watching, GoodBye!");
        delay(1000);
        popup = 0;
   // }
   // else 
   // {
        tft.fillWindow();
        tft.setActiveWindow();
        popup = 0;   // resume our normal schedule broadcast
        popup_timer.interval(65000);
        drawSpectrumFrame(spectrum_preset);
    }
}