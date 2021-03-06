extern int fndx;
extern volatile uint32_t fstep;
extern String increment;

void selectStep(int ndx)
{
  if(fndx==0)
  {
            Serial.println("Lets set the step to 1 Hz");
            increment="Ts 1 Hz";
            fstep=1;
  }
  if(fndx==1)
  {
            Serial.println("Lets set the step to 10 Hz");
            increment="Ts 10 Hz";
            fstep=10;
  }
  if(fndx==2)
  {
            Serial.println("Lets set the step to 100 Hz");
            increment="Ts 100 Hz";
            fstep=100;
  }
 if(fndx==3)
  {
            Serial.println("Lets set the step to 250 Hz");
            increment="Ts 250 Hz";
            fstep=250;
  }

  if(fndx==4)
  {
            Serial.println("Lets set the step to 1000 Hz");
            increment="Ts 1 kHz";
            fstep=1000;
  }

  if(fndx==5)
  {
            Serial.println("Lets set the step to 10 KHz");
            increment="Ts 10 kHz";
            fstep=10000;
  }
  displayStep();
}
