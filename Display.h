#include <RA8875.h>
extern RA8875 tft;

extern String mode;
extern  String bandwidth;
extern String increment;
extern String agc;
extern volatile uint32_t Freq;
extern volatile uint32_t fstep;
extern int attenuator;
extern int preamp;


// The below are fixed numbers based on screen size and other screen object edges
// These will also be be need to declared as extern variables in other files to leverage.
int L_frame_left = 0;
int L_frame_right = 99;
int R_frame_left = 700;
int R_frame_right = 799;
int Bottom_frame = 479;
int Top_frame = 70;
int Center_window_divider_line = 120;   // A line that can be drawn between the two button stack frames if desired.

// These define the button height and stack position and number of buttons, all equal size
int number_of_buttons = 6;                                         // Number of buttons
int B_height = (Bottom_frame - Top_frame) / number_of_buttons;    // scales the vertical column of buttons on each side, all equal size.
int B_num = 1;                                                    // button number to scale.  Use B_num * B_height

int displayColor=RA8875_LIGHT_GREY;
int textcolor = tft.Color565(128, 128, 128);

//////////////////////////////////////////////////////////////
void displayMode()
{
  tft.fillRect(0, 0, 100, 50,RA8875_BLACK );
  tft.setFont(Arial_24);
  tft.setCursor(10,6);
  tft.setTextColor(RA8875_LIGHT_ORANGE);
  tft.print(mode); 
 
}

void displayBandwidth()
{
 tft.fillRect(110, 0, 180, 40,RA8875_BLACK);
 tft.setFont(Arial_20);
 tft.setCursor(130, 6 );
 tft.setTextColor(RA8875_LIGHT_ORANGE);
 tft.print(bandwidth);   
}


void displayFreq()
{ 
 tft.fillRect(305, 0, 210, 40,RA8875_BLACK);
 tft.setFont(Arial_32);
 tft.setCursor(306,6);
 tft.setTextColor(RA8875_LIGHT_ORANGE);
 tft.print(float(Freq)/1000,3);
}

void displayStep()
{
  tft.fillRect(540, 0, 150, 40,RA8875_BLACK);
  tft.setFont(Arial_18);
  tft.setCursor(545,6);
  tft.setTextColor(RA8875_LIGHT_ORANGE);
  tft.print(increment);
}

void displayAgc()
{
  tft.fillRect(695, 0, 120, 40,RA8875_BLACK);
  tft.setFont(Arial_18);
  tft.setCursor(700,6);
  tft.setTextColor(RA8875_LIGHT_ORANGE);
  tft.print(agc);
}

void displayPreamp(void)
{
     tft.setFont(Arial_18);
     tft.setCursor(728,163);
    
    if(preamp>0)
    {
      preamp=0;
    }
    else
    {
      preamp=preamp+1;;
    }
 
    if(preamp==0)
    {
    tft.setTextColor(textcolor); 
    }
    
    if(preamp==1)
    {
    tft.setTextColor(RA8875_GREEN); 
    }
    
    tft.print("Pre");
    delay(450); 
}

void displayAttn()
{
 tft.setFont(Arial_18);
 tft.setCursor(726,95);

  if(attenuator>0)
  {
    attenuator=0;
  }
  else
  {
    attenuator=attenuator+1;;
  }
 
 if(attenuator==0)
 {
 tft.setTextColor(textcolor); 
 }
 
 if(attenuator==1)
 {
  tft.setTextColor(RA8875_GREEN); 
 }
 tft.print("Attn");
 delay(450); 

}
//
//--------------------------------------------------  initDisplay ------------------------------------------------------------------------
//
void initDisplay()
{
    //tft.begin(RA8875_800x480);      // usually done in setup() already.  Do not call here if using spectrum object which is called before this function.
    tft.setTextColor(textcolor);
    tft.setFont(Arial_18);
    tft.setRotation(0); //rotate text and graphics
    int display_width = tft.width();
    int display_height = tft.height();
    Serial.print("Display Type: RA8875_800x480 ");Serial.print("Display Size: ");Serial.print(display_width);Serial.print("x");Serial.println(display_height);    
    
    // Outer Button Frames lines
    tft.drawRect(R_frame_left,Top_frame,R_frame_right,Bottom_frame,displayColor);   // Draw Right side button stack outer frame
    tft.drawRect(L_frame_left,Top_frame,L_frame_right,Bottom_frame,displayColor);   // Draw Left side button stack outer frame
    //tft.drawLine(L_frame_right,Bottom_frame,R_frame_left,Bottom_frame,displayColor);   //Fill in the bottom
    
    // If used draw a divider line over the top of the spectrum object
    //tft.drawLine(L_frame_right,Center_window_divider_line,R_frame_left,Center_window_divider_line,displayColor);   //Fill in the bottom

    //tft.setCursor(L_frame_right+200, Top_frame + 10);
    //tft.print(" Add Cool Stuff Here");
  
    //for screen test and maintenance
    //tft.drawLine(400+x,0+y,400+x,460+y,RA8875_GREEN); 
    // tft.drawLine(50+x,0+y,50+x,430+y,RA8875_GREEN); 
  
/////////////////////////Display left side/////////////////////////////
    B_num = 1;
    tft.setCursor(19, Top_frame+(B_height*B_num)-(B_height/2)-10);
    tft.print("Mode"); 
    tft.drawLine(L_frame_left,Top_frame+(B_height*B_num),L_frame_right-1,Top_frame+(B_height*B_num),displayColor);
  
    B_num = 2;
    tft.setCursor(12, Top_frame+(B_height*B_num)-(B_height/2)-10);
    tft.print("Bw Up"); 
    tft.drawLine(L_frame_left,Top_frame+(B_height*B_num),L_frame_right-1,Top_frame+(B_height*B_num),displayColor);
  
    B_num = 3;
    tft.setCursor(12, Top_frame+(B_height*B_num)-(B_height/2)-10);
    tft.print("Bw Dn");
    tft.drawLine(L_frame_left,Top_frame+(B_height*B_num),L_frame_right-1,Top_frame+(B_height*B_num),displayColor);
  
    B_num = 4;
    tft.setCursor(16, Top_frame+(B_height*B_num)-(B_height/2)-10);
    tft.print("Ts Up"); 
    tft.drawLine(L_frame_left,Top_frame+(B_height*B_num),L_frame_right-1,Top_frame+(B_height*B_num),displayColor); 
  
    B_num = 5;
    tft.setCursor(16, Top_frame+(B_height*B_num)-(B_height/2)-10);
    tft.print("Ts Dn");
    tft.drawLine(L_frame_left,Top_frame+(B_height*B_num),L_frame_right-1,Top_frame+(B_height*B_num),displayColor);
  
    B_num = 6;
    tft.setCursor(18, Top_frame+(B_height*B_num)-(B_height/2)-10);
    tft.print("Setup");
  
  ///////////////////////////Right Side/////////////////////////////////////////
    B_num = 1;
    tft.setCursor(726, Top_frame+(B_height*B_num)-(B_height/2)-10);
    tft.print("Attn");
    tft.drawLine(R_frame_left,Top_frame+(B_height*B_num),R_frame_right,Top_frame+(B_height*B_num),displayColor);
    
    B_num = 2;
    tft.setCursor(730, Top_frame+(B_height*B_num)-(B_height/2)-10);
    //tft.print("Pre");  
    tft.drawLine(R_frame_left,Top_frame+(B_height*B_num),R_frame_right,Top_frame+(B_height*B_num),displayColor);
    displayPreamp();
     
    B_num = 3;  
    tft.setCursor(722, Top_frame+(B_height*B_num)-(B_height/2)-10);
    tft.print("AGC");  
    tft.drawLine(R_frame_left,Top_frame+(B_height*B_num),R_frame_right,Top_frame+(B_height*B_num),displayColor);
    
    B_num = 4;  
    tft.setCursor(716, Top_frame+(B_height*B_num)-(B_height/2)-10);
    //tft.setCursor(716,375);
    tft.print("Dsply");   
    tft.drawLine(R_frame_left,Top_frame+(B_height*B_num),R_frame_right,Top_frame+(B_height*B_num),displayColor);
    
    B_num = 5;
    tft.setCursor(710, Top_frame+(B_height*B_num)-(B_height/2)-10);
    tft.print("Band +");  
    tft.drawLine(R_frame_left,Top_frame+(B_height*B_num),R_frame_right,Top_frame+(B_height*B_num),displayColor);
    
    B_num = 6;
    tft.setCursor(710, Top_frame+(B_height*B_num)-(B_height/2)-10);
    tft.print("Band -");
}
 
