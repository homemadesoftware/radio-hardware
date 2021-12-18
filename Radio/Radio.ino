#include <Wire.h>

unsigned char onoffPin = 3;
unsigned char channelKnob0 = 4;
unsigned char channelKnob1 = 5;
unsigned char channelKnob2 = 6;
unsigned char piPower = 8;
unsigned char powerLedOutput = 9;
unsigned char volumeSensorPin = A3;    // select the input pin for the potentiometer
unsigned char i2cSlaveAddress = 8;
unsigned char lastConnect = 0;
const unsigned char masterConnected = 100;
unsigned char powerLedBlinkCount;
unsigned char powerLedOn = 0;
unsigned char powerOn = 0;
unsigned int powerOffCountDown;


class HardwareState
{
  private :
    unsigned char m_size;
    unsigned char m_onOff;
    unsigned char m_channelValue;
    unsigned char m_volumeLow;
    unsigned char m_volumeHigh;
    unsigned char m_cookieLow;
    unsigned char m_cookieHigh;
  
  public :
    void reset()
    {
      m_size = sizeof(HardwareState);
      m_onOff = 0;
      m_channelValue = 0;
      m_volumeLow = 0;
      m_volumeHigh = 0;
      m_cookieLow = 'C';
      m_cookieHigh = 'H';
    }
    
    void setVolume(int level)
    {
      m_volumeLow = level & 0xff;
      m_volumeHigh = (level >> 8) & 0xff;
    }
    
    void setOnOff(int onOff)
    {
      m_onOff = onOff;
    }
    
    void setChannel(int channel)
    {
      m_channelValue = channel;
    }
    
    unsigned char getOnOff()
    {
      return m_onOff;
    }
};

HardwareState state;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(onoffPin, INPUT_PULLUP);     
  pinMode(channelKnob0, INPUT_PULLUP);     
  pinMode(channelKnob1, INPUT_PULLUP);     
  pinMode(channelKnob2, INPUT_PULLUP);
  pinMode(powerLedOutput, OUTPUT);
  pinMode(piPower, OUTPUT);
  
  state.reset();
  
  // start i2c slave
  Wire.begin(i2cSlaveAddress);
  Wire.onRequest(onI2cRequest); 

  
}

void onI2cRequest()
{
    Wire.write((unsigned char*)&state, sizeof(state)); 
    lastConnect = masterConnected;
}


// the loop routine runs over and over again forever:
void loop() 
{
  // Update the state every ten ms
  state.setOnOff(!digitalRead(onoffPin));
  state.setVolume(analogRead(volumeSensorPin));
  state.setChannel(readChannelValue());
  delay(10);               // wait for 10 ms
  
  if (state.getOnOff() == 0)
  {
    powerLedOn = 0;
    if (powerOffCountDown < 1)
    {
      powerOn = 0;
    }
    else
    {
      powerOffCountDown--;
    }
  }
  else
  {
    powerOn = 1;
    powerOffCountDown = 6000; // wait for 60 seconds before powering off Rapsberry pi

    if (lastConnect > 0) // Stop blinking, LED is on
    {
      lastConnect--;
      powerLedOn = 1;
      powerLedBlinkCount = 100;
    }
    else if (lastConnect == 0) // Start blinking 
    {
      if (++powerLedBlinkCount > 10)
      {
        powerLedBlinkCount = 0;
        powerLedOn = !powerLedOn;    
      }
    }
  }
  
  digitalWrite(powerLedOutput, powerLedOn);
  digitalWrite(piPower, powerOn);
}


/**
Reads the channel selector knob value.
*/
int readChannelValue()
{
  int chValues[3];
  chValues[0] = digitalRead(channelKnob0);
  chValues[1] = digitalRead(channelKnob1) << 1;
  chValues[2] = digitalRead(channelKnob2) << 2;
  int value = chValues[0] | chValues[1]  | chValues[2];
  
  switch (value)
  {
      case 6:
        value = 16;
      break;
     
      case 3:
        value = 19;
      break; 
      
      case 4:
        value = 25;
      break;
      
      case 7 : 
        value = 31;
      break;
      
      case 0 : 
        value = 41;
        break; 
      
      case 5 :
        value = 49;
        break; 
  }
 return value;
}
