// Arduino libraries
#include <EEPROM.h>
#include <MIDI.h>
#include <avr/pgmspace.h>
#include <MozziGuts.h>
#include <mozzi_rand.h>
#include <mozzi_analog.h>

// bastl libraries
#include <trinityHW.h>
#include <bastlSample.h>


// Samples
//#include <bastlSamples/kick.h>
#include <bastlSamples/KICK2.h>
#include <bastlSamples/SNARE2.h>
//#include <bastlSamples/snare.h>
//#include <bastlSamples/hat.h>
#include <bastlSamples/HAT2.h>
#include <bastlTables/haluz2048_int8.h>
//#include <bastlSamples/CB.h>
#include <bastlSamples/CB3.h>
#include <bastlSamples/ride.h>
//#include <samples/ride.h>
//#include <bastlSamples/tom.h>










//mem
#define PAGE_G 0
#define PAGE_B 1
#define VARIABLES_PER_PAGE 3
#define RATE 0 //  1023 - 10
#define STRETCH 1 // 255 - 8
#define CUT 2// 255 - 8
//26
#define SAMPLE 3 // 7 - 3
#define CRUSH 4 // 255 - 8
#define VOLUME 5 // 255 -8
//19
// 4*8= 32+13= 45 bitu => 6 bytů
//6 zvuku *6 bytů = 36 bytů / preset
#define KNOB_BITS 10
#define NUMBER_OF_VARIABLES 6
#define NUMBER_OF_BYTES 6
#define NUMBER_OF_PRESETS 6
#define PRESET_SIZE 36
#define NUMBER_OF_SOUNDS 6
#define CHANNEL_BYTE 1023

void updateControl();
int scale(int _value, unsigned char _originalBits, unsigned char _endBits);
void initMem();
void calculateBitDepth();
void ShouldIClearMemory();
void clearMemmory();
void loadPreset(unsigned char index);
boolean ConditionToClear();
void IndicateClearing(boolean _start);
void setMidiChannel(unsigned char _channel);
int getVar(unsigned char _SOUND, unsigned char _VARIABLE);
void storePreset(unsigned char index);
int maxVal(int _maxVal);
void setVar(unsigned char _SOUND, unsigned char _VARIABLE, int _value);

//midi
#define CONTROL_CHANGE_BITS 7
#define CONTROL_CHANGE_OFFSET 3

#define SWITCH_BYTE 14
#define PAGE_BYTE 13

#define BASTL_BYTE 0x7D
#define CHANNEL_BYTE 0x00
#define INSTRUMENT_BYTE 0X01 // poly 1.0
#define TEST_BYTE 0x0A
#define NUMBER_OF_MESSAGE_BYTES 2

#define NUMBER_OF_MESSAGE_BYTES 16
#define NUMBER_OF_SYSEX_BYTES 14

#define PRESET_BY_CC_BYTE 0
#define RANDOMIZE_BYTE 127

void initMidi(unsigned char _channel);
unsigned char getMidiChannel();
void HandleNoteOff(byte channel, byte note, byte velocity);
void extractSysExArray(unsigned char _sound);



//render audio
#define DOWN_SHIFT 3 // zkusit min

//UI
#define NUMBER_OF_PAGES 2
#define KNOB_MODE SMALL_BUTTON_2
#define DEFAULT_VELOCITY 127
#define VOLUME_COMPENSATION 100
void playSound(unsigned char _SOUND, unsigned char _VELOCITY);
void animation();
void UI();
void testMode();
void renderSmallButtons();
void renderCombo();
void renderBigButtons();
void renderKnobs();
void renderTweaking(unsigned char _page);
void renderTimeStretch();
void randomize(unsigned char _sound);
boolean inBetween(int val1,int val2,int inBet);



void anySound();
void stopSound();
int decreaseValue(int _VALUE, int _OVERFLOW);

