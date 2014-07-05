//TODO:
// Create structs for samples!
// CHANNEL_BYTE is defined twice
// use standard inttypes


#include <DRUM.h>

const uint8_t NUMBER_OF_VOICES = 3;
#define NUMBER_OF_WAVETABLES 6
#define CONTROL_RATE 64 // used my mozzi. powers of 2 please


int main(void) {
  init();
  setup();
  while(true) {
    loop();
  }
}




MIDI_CREATE_INSTANCE(HardwareSerial,Serial,MIDI);
trinityHW hw; // MOZZI or DEFAULT
Sample<AUDIO_RATE> aSample[NUMBER_OF_VOICES];



const char* WAVE_TABLES[]={
  (char*)KICK2_DATA, (char*)SNARE2_DATA, (char*)HAT2_DATA,(char*)CB3_DATA,(char*)RIDE_DATA, (char*)HALUZ2048_DATA};

const unsigned int WAVE_TABLES_NUM_CELLS[]={
  KICK2_NUM_CELLS, SNARE2_NUM_CELLS, HAT2_NUM_CELLS, CB3_NUM_CELLS,RIDE_NUM_CELLS,HALUZ2048_NUM_CELLS};//, CB_NUM_CELLS};


// for randomizing ?
const uint16_t maxValue[] PROGMEM ={
  1023,255,255, 7,255,255}; //cons

unsigned char variableDepth[NUMBER_OF_VARIABLES];//={1,2,3,6,4,8,2,3,1,5,3,7,8,6,7};
boolean shift;
unsigned char page;
unsigned char sound,lastSound;
unsigned char crush[NUMBER_OF_VOICES];
unsigned char currentSound[NUMBER_OF_VOICES];
unsigned char volume[NUMBER_OF_VOICES];

unsigned char currentPreset; 	// the number of the active preset. range?

unsigned char inputChannel;


boolean test;
boolean bootShift=false;

/*
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aOsc[NUMBER_OF_VOICES]={
 (SIN2048_DATA)};
 Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aMod[NUMBER_OF_VOICES]={
 (SIN2048_DATA)};
 */
//Sample<AUDIO_RATE> aSample[0](KICK_DATA,KICK_NUM_CELLS);
//Sample<AUDIO_RATE> aSample[1](KICK_DATA,KICK_NUM_CELLS);
//Sample<AUDIO_RATE> aSample[2](KICK_DATA,KICK_NUM_CELLS);










void setup() {


  // debug();
  hw.setFreezeType(UNFREEZE_EXTERNALY);
  hw.initialize(DEFAULT);
  //Serial.begin(9600);

  animation();
  initMem();
  initMidi(getMidiChannel());

  startMozzi(CONTROL_RATE);

  //adcReconnectAllDigitalIns();
  //yhw.initialize(DEFAULT);
  for(uint8_t i=0;i<NUMBER_OF_VOICES;i++) {
	  aSample[i].setTable(WAVE_TABLES[i],WAVE_TABLES_NUM_CELLS[i]);
	  aSample[i].setTimeStretch(5);
  }
  hw.update();
  hw.update();
  bootShift=hw.buttonState(SMALL_BUTTON_1);
}


void updateControl(){

  while(MIDI.read(inputChannel)) MIDI.read(inputChannel);
  while(MIDI.read()) MIDI.read();
  MIDI.read();
  MIDI.read();
  MIDI.read();
  MIDI.read();
  MIDI.read();

  hw.update();
  UI();
  while(MIDI.read(inputChannel)) MIDI.read(inputChannel);
  while(MIDI.read()) MIDI.read();
  MIDI.read();
  MIDI.read();
  MIDI.read();
  MIDI.read();
  MIDI.read();

}

void loop() {
  audioHook();
}




//////////////////////////////////////////////////








/*

 osc
 - pitch - freq - res
 - rate - reso - amt -
 - a -s- r  - když s==0 d=r
 - oscCharacter  / lfoCharacter / adsrCharacter 5

 -osc character
 ->wavetable 2 - filter/fm 1 - wavetable 2 bity =>5
 -lfo character
 ->shape 3 - destination 2 =>5

 -adsr character
 -> amt - 4 1 dest decay 3

 */




// variable names
//page 0


//osc mod-freq amt
//atck decay mod type // 2+2+3






int maxVal(int _maxVal){
  return  pgm_read_word_near(maxValue+_maxVal);
}

unsigned char byteCoordinate[NUMBER_OF_VARIABLES];
unsigned char bitCoordinate[NUMBER_OF_VARIABLES];

/*
int buffer[NUMBER_OF_SOUNDS][NUMBER_OF_VARIABLES];
 unsigned char bufferP[NUMBER_OF_STEPS];
 */

unsigned char variable[NUMBER_OF_SOUNDS][NUMBER_OF_BYTES];

void initMem(){

  calculateBitDepth();
  ShouldIClearMemory();
  if(hw.factoryClear()) clearMemmory(), hw.factoryCleared();
  hw.initialize(DEFAULT);
  loadPreset(0);

}

void calculateBitDepth(){
  for(int i=0;i<NUMBER_OF_VARIABLES;i++){ // calculate bitDepth according to the maximum value
    int x=0;
    while(maxVal(i)-pow(2,x)>=0) x++;
    variableDepth[i]=x ;
  }

  for(int i=0;i<NUMBER_OF_VARIABLES;i++){
    int sum=0;
    for(int j=0;j<i;j++){
      sum+=variableDepth[j];
    }
    byteCoordinate[i]=sum/8;
    bitCoordinate[i]=sum%8;
  }
}

int getVar(unsigned char _SOUND, unsigned char _VARIABLE){

  int _value=0;
  unsigned char byteShift=0;
  unsigned char _bitCoordinate=0;
  for(int i=0;i<variableDepth[_VARIABLE];i++){

    if((bitCoordinate[_VARIABLE]+i)>15){
      byteShift=2;
      _bitCoordinate = i-(16-bitCoordinate[_VARIABLE]);
    }
    else if((bitCoordinate[_VARIABLE]+i)>7){
      byteShift=1;
      _bitCoordinate = i-(8-bitCoordinate[_VARIABLE]);//bitCount;
    }
    else {
      _bitCoordinate=bitCoordinate[_VARIABLE]+i;
    }

    boolean bitState=bitRead(variable[_SOUND][byteCoordinate[_VARIABLE]+byteShift],_bitCoordinate);
    bitWrite(_value,i,bitState);

  }
  return _value;

}

void setVar(unsigned char _SOUND, unsigned char _VARIABLE, int _value){

  unsigned char byteShift=0;
  unsigned char _bitCoordinate;
  for(int i=0;i<variableDepth[_VARIABLE];i++){



    if((bitCoordinate[_VARIABLE]+i)>15){
      byteShift=2;
      _bitCoordinate = i-(16-bitCoordinate[_VARIABLE]);
    }
    else if((bitCoordinate[_VARIABLE]+i)>7){
      byteShift=1;
      _bitCoordinate = i-(8-bitCoordinate[_VARIABLE]);
    }
    else {
      _bitCoordinate=bitCoordinate[_VARIABLE]+i;
    }

    boolean bitState=bitRead(_value,i);
    bitWrite(variable[_SOUND][byteCoordinate[_VARIABLE]+byteShift],_bitCoordinate,bitState);
  }

}


void storePreset(unsigned char index) {

  int offset = index * PRESET_SIZE;
  for (int j = 0; j < NUMBER_OF_SOUNDS; j++) {
    for (int k = 0; k < NUMBER_OF_BYTES; k++) {
      EEPROM.write(offset + ((NUMBER_OF_BYTES * j) + k), variable[j][k]);
    }
  }

}

void loadPreset(unsigned char index) {

  currentPreset=index;
  int offset = index * PRESET_SIZE;

  for (int j = 0; j < NUMBER_OF_SOUNDS; j++) {
    for (int k = 0; k < NUMBER_OF_BYTES; k++) {
      variable[j][k]=EEPROM.read(offset + ((NUMBER_OF_BYTES * j) + k));
    }
  }

}



void clearMemmory(){

  static const uint16_t clearTo[] PROGMEM ={370,64,255, 0,0,200};

  for(int x=0;x<NUMBER_OF_PRESETS;x++){
    loadPreset(x);
    for(int i=0;i<NUMBER_OF_SOUNDS;i++){
      for(int j=0;j<NUMBER_OF_VARIABLES;j++){

        if(j==SAMPLE)  setVar(i,j,i*6);
        else if (j==RATE && i>=3) setVar(i,j, pgm_read_word_near(clearTo+j)+350);
        else if (j==STRETCH && i>=3) setVar(i,j, pgm_read_word_near(clearTo+j)+64);
        else setVar(i,j, pgm_read_word_near(clearTo+j));


      }
    }
    //  tempo=120;

    storePreset(x);

  }
}




void ShouldIClearMemory(){
  hw.update();
  hw.update();
  if(ConditionToClear()){
    IndicateClearing(true);
    clearMemmory();
    IndicateClearing(false);
    setMidiChannel(1);
  }

}

boolean ConditionToClear(){
  boolean _shouldClear=false;
  //if something or something else _shouldClear=true;
  if(hw.buttonState(BIG_BUTTON_1) && hw.buttonState(BIG_BUTTON_2) && hw.buttonState(BIG_BUTTON_3)) _shouldClear=true;
  else if(hw.buttonState(BIG_BUTTON_1)) setMidiChannel(1+9*hw.buttonState(SMALL_BUTTON_1));
  else if(hw.buttonState(BIG_BUTTON_2)) setMidiChannel(2+9*hw.buttonState(SMALL_BUTTON_1));
  else if(hw.buttonState(BIG_BUTTON_3)) setMidiChannel(3+9*hw.buttonState(SMALL_BUTTON_1));

  return _shouldClear;
}

void setMidiChannel(unsigned char _channel){
  EEPROM.write(CHANNEL_BYTE,_channel);
}

unsigned char getMidiChannel(){
  return EEPROM.read(CHANNEL_BYTE);
}

void IndicateClearing(boolean _start){

  for(int i=0;i<NUMBER_OF_COLORS;i++) hw.setColor(i), delay(200), hw.update();
  delay(200);
  hw.setColor(0);
  hw.setLed(LED_1,_start);
  hw.setLed(LED_2,_start);
  hw.setLed(LED_3,_start);
  hw.update();

}

boolean inBetween(int val1,int val2,int inBet){

  boolean retVal;

  if(val1>=val2){
    if(inBet<=val1 && inBet>=val2) retVal=true;
    else retVal=false;
  }
  else if(val1<val2){
    if(inBet>=val1 && inBet<=val2) retVal=true;
    else retVal=false;
  }

  return retVal;

}

int scale(int _value, unsigned char _originalBits, unsigned char _endBits){
  if(_originalBits >= _endBits) return _value >> (_originalBits - _endBits);
  else return _value << (_endBits - _originalBits);
}

/*

 void debug(){
 randomSeed(analogread(0));

 Serial.begin(9600);
 Serial.println();

 for(int j=0;j<NUMBER_OF_PATTERNS;j++){
 for(int i=0;i<NUMBER_OF_STEPS;i++){
 bufferP[i]=random(255);
 for(int k=0;k<NUMBER_OF_SOUNDS;k++){
 Serial.print(bitRead(bufferP[i],k),DEC);
 SetStep(j,i,k,bitRead(bufferP[i],k));
 }
 }
 Serial.println();
 }

 Serial.println();
 Serial.println();
 for(int j=0;j<NUMBER_OF_PATTERNS;j++){
 for(int i=0;i<NUMBER_OF_STEPS;i++){
 for(int k=0;k<NUMBER_OF_SOUNDS;k++){
 Serial.print( GetStep(j,i,k),DEC);
 }
 }
 Serial.println();
 }

 StorePreset(0);
 // StorePreset(1);
 // ClearMemmory();
 // LoadPreset(1);

 Serial.println();
 for(int j=0;j<NUMBER_OF_PATTERNS;j++){
 for(int i=0;i<NUMBER_OF_STEPS;i++){
 for(int k=0;k<NUMBER_OF_SOUNDS;k++){
 Serial.print( GetStep(j,i,k),DEC);
 }
 }
 Serial.println();
 }

 Serial.println("original:");
 for(int j=0;j<NUMBER_OF_SOUNDS;j++){
 for(int i=0;i<NUMBER_OF_VARIABLES;i++){
 buffer[j][i]=random(0,maxVal(i));
 Serial.print(buffer[j][i],DEC);
 Serial.print(" , ");
 }
 Serial.println();
 }
 Serial.println();
 Serial.println("comprimed & decomprimed:");

 for(int j=0;j<NUMBER_OF_SOUNDS;j++){
 for(int i=0;i<NUMBER_OF_VARIABLES;i++){
 setVar(j,i,buffer[j][i]);
 }
 }
 for(int j=0;j<NUMBER_OF_SOUNDS;j++){
 for(int i=0;i<NUMBER_OF_VARIABLES;i++){
 Serial.print(getVar(j,i));
 Serial.print(" , ");
 }
 Serial.println();
 }


 Serial.println();
 Serial.println();
 Serial.println();
 StorePreset(0);
 // ClearMemmory();
 LoadPreset(0);
 Serial.println("loaded from eeprom:");
 for(int j=0;j<NUMBER_OF_SOUNDS;j++){
 for(int i=0;i<NUMBER_OF_VARIABLES;i++){
 Serial.print(getVar(j,i));
 Serial.print(" , ");
 }
 Serial.println();
 }




 Serial.println();
 for(int j=0;j<NUMBER_OF_PATTERNS;j++){
 for(int i=0;i<NUMBER_OF_STEPS;i++){
 for(int k=0;k<NUMBER_OF_SOUNDS;k++){
 Serial.print( GetStep(j,i,k),DEC);
 }
 }
 Serial.println();
 }

 }

 */

/////////////////////////////////////////////////////////////






unsigned char midiSound;
unsigned char sysExArray[NUMBER_OF_SYSEX_BYTES];

void HandleNoteOn(byte channel, byte note, byte velocity) {
  if(channel==inputChannel){
    if (velocity == 0) {
      HandleNoteOff(channel,note,velocity);
    }
    else{
      playSound(note%6, velocity);
      if(shift && (note%6)>=3) hw.setLed(note%3,true);
      else if(note%6<3) hw.setLed(note%3,true);
      midiSound=note%6;
    }
  }
}

void HandleNoteOff(byte channel, byte note, byte velocity){
  /*
  unsigned char voice=note%3;
   unsigned char _sound=note%6;
   if(channel==inputChannel){
   if(currentSound[voice]==_sound) ;
   }
   */
}

void HandleControlChange(byte channel, byte number, byte value){
  // implement knob movement
  if(channel==inputChannel){
    if(number==PRESET_BY_CC_BYTE) loadPreset(map(value,0,128,0,NUMBER_OF_PRESETS)), hw.freezeAllKnobs();

    else if(number==RANDOMIZE_BYTE) randomize(midiSound);

    else if((number>=CONTROL_CHANGE_OFFSET && number<39)){
      number=number-CONTROL_CHANGE_OFFSET;
      midiSound=number/NUMBER_OF_VARIABLES;
      number=number%NUMBER_OF_VARIABLES;

      setVar(midiSound,number,scale(value,CONTROL_CHANGE_BITS,variableDepth[number]));
      hw.freezeAllKnobs();
      renderTweaking((number)/VARIABLES_PER_PAGE);

      /*
    if((number<NUMBER_OF_VARIABLES){
       setVar(midiSound,number,scale(value,CONTROL_CHANGE_BITS,variableDepth[number]));
       hw.freezeAllKnobs();
       renderTweaking((number)/VARIABLES_PER_PAGE);
       }
       */
    }
    /*
    else if(number==PRESET_BY_CC_BYTE)
     else if(number==SUSTAIN_PEDAL_BYTE) sustainPedal=value>>6;
     else if(number==RANDOMIZE_BYTE)
     */
  }
  if(test){
    if(number==SWITCH_BYTE){
      for(int i=0;i<3;i++) hw.setSwitch(i,bitRead(value,i));
    }
    else if(number==PAGE_BYTE){
      page=value;
    }
  }
}

void HandleProgramChange(byte channel, byte number  ){
  // implement preset change
}

void HandlePitchBend(byte channel, int bend){

}

void HandleSystemExclusive(byte *array, byte size){

  if(array[1]==BASTL_BYTE){

    if(array[2]==CHANNEL_BYTE){
      inputChannel=array[2];
      array[2]++;
      MIDI.sendSysEx(size,array);
    }
    else if(array[2]==INSTRUMENT_BYTE){
      hw.setColor(WHITE);
      hw.update();
      for(int i=0;i<NUMBER_OF_SYSEX_BYTES;i++) sysExArray[i]=array[i+1];
      extractSysExArray(sound);
      hw.freezeAllKnobs();
    }
    else if(array[2]==TEST_BYTE) test=true, MIDI.turnThruOff(), MIDI.sendSysEx(NUMBER_OF_MESSAGE_BYTES,array,false);
  }
}


void HandleSongSelect(byte songnumber){

}

void HandleClock(){

}
void HandleStart(){

}
void HandleContinue(){

}
void HandleStop(){

}

void indicateMidiChannel(unsigned char _channel){

  boolean highChannel=false;
  if(_channel>=10)   hw.setLed(LED_1,true), hw.setLed(LED_2,true), hw.setLed(LED_3,true), highChannel=true;
  for(int i=0;i<3;i++){
    hw.setLed(_channel-1-highChannel*9,false);
    hw.update();
    delay(150);
    hw.setLed(_channel-1-highChannel*9,true);
    hw.update();
    delay(150);
  }

}


void initMidi(unsigned char _channel){

  MIDI.begin(0);
  inputChannel=_channel;
  indicateMidiChannel(_channel);

  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandleNoteOff(HandleNoteOff);

  MIDI.setHandlePitchBend(HandlePitchBend);

  MIDI.setHandleControlChange(HandleControlChange);
  MIDI.setHandleProgramChange(HandleProgramChange);
  MIDI.setHandleSystemExclusive(HandleSystemExclusive);
  MIDI.setHandleSongSelect(HandleSongSelect);

  MIDI.setHandleClock(HandleClock);
  MIDI.setHandleStart(HandleStart);
  MIDI.setHandleContinue(HandleContinue);
  MIDI.setHandleStop(HandleStop);

  MIDI.turnThruOn(midi::Full);
  // MIDI.turnThruOff();
}






void sendSysExArray(){
  MIDI.sendSysEx(NUMBER_OF_MESSAGE_BYTES,sysExArray,false);
}


void makeSysExArray(unsigned char _sound){
  unsigned char _bit=0;
  unsigned char _byte=0;
  sysExArray[0]=BASTL_BYTE;
  sysExArray[1]=INSTRUMENT_BYTE;
  for(int i=0;i<NUMBER_OF_VARIABLES;i++){
    int readFrom=getVar(_sound,i);
    for(int j=0;j<variableDepth[i];j++){
      _bit++;
      if(_bit>=7) _bit=0,_byte++;
      bitWrite(sysExArray[_byte+2],_bit, bitRead(readFrom,j));
    }
  }
}


void extractSysExArray(unsigned char _sound){
  unsigned char _bit=0;
  unsigned char _byte=0;
  for(int i=0;i<NUMBER_OF_VARIABLES;i++){
    int writeTo=0;
    for(int j=0;j<variableDepth[i];j++){
      _bit++;
      if(_bit>=7) _bit=0,_byte++;
      bitWrite(writeTo,j,bitRead(sysExArray[_byte+2],_bit));
    }
    setVar(_sound,i,writeTo);
  }
}






////////////////////////////////////////////////////////////////







int updateAudio(){

  long output=0;

  for(int voice=0;voice<NUMBER_OF_VOICES;voice++){
   if(aSample[voice].isPlaying()) output+=(volume[voice]*(aSample[voice].next()^crush[voice]))>>7;
  }


  return output<<5;

}

/////////////////////////////////////////////







boolean combo;
int var[NUMBER_OF_VOICES][6]; //??






void UI(){
  if(test) testMode();
  else{
    hw.setColor(page+2);
    renderSmallButtons();
    renderCombo();
    renderBigButtons();
    renderKnobs();
    renderTweaking(page);
    renderTimeStretch();
  }

}



void playSound(unsigned char _SOUND, unsigned char _VELOCITY){
  unsigned char voice=_SOUND%3;
  currentSound[voice]=_SOUND;
  unsigned char _sample=map(getVar(_SOUND,SAMPLE),0,8,0,NUMBER_OF_WAVETABLES);
  aSample[voice].setTable(WAVE_TABLES[_sample],WAVE_TABLES_NUM_CELLS[_sample]);
  aSample[voice].setFreq((float)(getVar(_SOUND,RATE)+1)/16);
  aSample[voice].setTimeStretch((getVar(_SOUND,STRETCH)<<2)+1);
  aSample[voice].setEnd(map(getVar(_SOUND,CUT),0,255,16,aSample[voice].getLength()));
  volume[voice]=(getVar(_SOUND,VOLUME)*(_VELOCITY))>>7;
  crush[voice]=getVar(_SOUND,CRUSH);

  aSample[voice].start();


}


int shiftSpeed;

void renderTimeStretch(){
  for(int voice=0;voice<NUMBER_OF_VOICES;voice++){
    if(aSample[voice].isPlaying()) aSample[voice].timeStretchUpdate();
  }
}

void renderTweaking(unsigned char _page){

  for(int voice=0;voice<NUMBER_OF_VOICES;voice++){

    switch(_page){
    case PAGE_G:
      aSample[voice].setFreq((float)(getVar(currentSound[voice],RATE)+1)/16);
      aSample[voice].setTimeStretch((getVar(currentSound[voice],STRETCH)<<2)+1);
      aSample[voice].setEnd(map(getVar(currentSound[voice],CUT),0,255,16,aSample[voice].getLength()));
      break;

    case PAGE_B:
      volume[voice]=getVar(currentSound[voice],VOLUME);
      crush[voice]=getVar(currentSound[voice],CRUSH);
      break;
    }


  }
  //tweaking end
}




void renderSmallButtons(){
  // knobMode
  if(hw.justReleased(KNOB_MODE) && !combo){
    if(page<NUMBER_OF_PAGES-1) page++;
    else page=0;
    hw.freezeAllKnobs();
  }

  if(hw.justPressed(SMALL_BUTTON_1)) shift=!shift;
  if(hw.justReleased(SMALL_BUTTON_1) && !bootShift) shift=!shift;
  if(hw.justPressed(EXTRA_BUTTON_1)) shift=!shift;


}

#define LONG_THRESHOLD 40
boolean countLong, longPress;
int longCount;
void renderCombo(){

  if(countLong) {
    longCount++;
    if(longCount>LONG_THRESHOLD) longPress=true;
  }
  //loading saving presets
  // if(hw.buttonState(SMALL_BUTTON_2) && hw.justPressed(SMALL_BUTTON_1)) loadPreset(0), hw.freezeAllKnobs(), combo=true;
  for(int i=0;i<3;i++){
    if(hw.buttonState(EXTRA_BUTTON_2) && hw.justPressed(i)) randomize(i+3*shift); // randomize
    if(hw.buttonState(SMALL_BUTTON_1) && hw.buttonState(SMALL_BUTTON_2) && hw.justPressed(i)) countLong=true,longCount=0, combo=true; // save&load
    if(hw.buttonState(SMALL_BUTTON_1) && hw.buttonState(SMALL_BUTTON_2) && hw.justReleased(i)){
      if(longPress) storePreset(currentPreset),loadPreset(i+3), hw.freezeAllKnobs(), combo=true; // save&load
      else storePreset(currentPreset), loadPreset(i+3*hw.buttonState(EXTRA_BUTTON_2)), hw.freezeAllKnobs(), combo=true;
      // countLong=false, longCount=0,longPress=false;
    }

    if(hw.buttonState(SMALL_BUTTON_2) && hw.justPressed(i)) countLong=true,longCount=0, combo=true; // load
    if(hw.buttonState(SMALL_BUTTON_2) && hw.justReleased(i)){
      if(longPress) loadPreset(i+3), hw.freezeAllKnobs();
      else loadPreset(i+3*hw.buttonState(EXTRA_BUTTON_2)), hw.freezeAllKnobs();
      // countLong=false, longCount=0, longPress=false;
    }
  }

  if(combo){
    //turn off combo when all buttons are released
    unsigned char _count=0;
    for(int i=0;i<NUMBER_OF_BUTTONS;i++)  _count+=hw.buttonState(i); // if(!hw.buttonState(i)) combo=false;
    if(_count==0) combo=false,countLong=false, longCount=0,longPress=false;
    //else combo=true;
  }


}

void randomize(unsigned char _sound){
  for(int i=0;i<NUMBER_OF_VARIABLES;i++) setVar(_sound,i,rand(maxValue[i])), hw.freezeAllKnobs();
}
void renderBigButtons(){

  if(!combo){
    for(int i=BIG_BUTTON_1;i<=BIG_BUTTON_3;i++) {
      hw.setLed(i,hw.buttonState(i));
      if(longPress) hw.setLed(i,true);
      if(hw.justPressed(i)) {
        if(!shift) sound=i, playSound(sound,DEFAULT_VELOCITY), shiftSpeed=0;
        else sound=i+3, playSound(sound,DEFAULT_VELOCITY);
      }
      if(hw.justReleased(i)) ;
    };
    if(sound!=lastSound)  hw.freezeAllKnobs();
    lastSound=sound;
  }

  for(int i=BIG_BUTTON_1;i<=BIG_BUTTON_3;i++){
    if(longPress) hw.setLed(i,true);
  }

}
void renderKnobs(){

  for(int i=0;i<NUMBER_OF_KNOBS;i++){

    unsigned char _variable=i+VARIABLES_PER_PAGE*page;


    if(hw.knobFreezed(i)) {
      if(lastSound==sound){ // knobs are unfreezed when they come across their original value
        if(inBetween( scale(hw.knobValue(i),KNOB_BITS,variableDepth[_variable]), scale(hw.lastKnobValue(i),KNOB_BITS,variableDepth[_variable]),getVar(sound,_variable) ) ) hw.unfreezeKnob(i),hw.setColor(WHITE); //external unfreez
      }
    }

    else{
      setVar(sound,_variable,scale(hw.knobValue(i),KNOB_BITS,variableDepth[_variable]));
    }
  }

}

int increaseValue(int _VALUE, int _OVERFLOW){
  if(_VALUE < _OVERFLOW) _VALUE++;
  else _VALUE=0;
  return _VALUE;
}
int increaseValue(int _VALUE, int _OVERFLOW, int _INCREMENT){
  if(_VALUE < _OVERFLOW) _VALUE+=_INCREMENT;
  return _VALUE;
}
int decreaseValue(int _VALUE, int _OVERFLOW, int _INCREMENT){
  if(_VALUE > _OVERFLOW) _VALUE-=_INCREMENT;
  return _VALUE;
}


unsigned char getBits(unsigned char _val, unsigned char _offset, unsigned char _bits){
  unsigned char returnVal=0;
  for(int i=0;i<_bits;i++){
    bitWrite(returnVal,i,bitRead(_val,_offset+i));
  }
  return  returnVal;
}

void animation(){

  hw.setLed(LED_1,true);
  hw.update();
  delay(150);
  hw.setLed(LED_3,true);
  hw.update();
  delay(150);
  hw.setLed(LED_2,true);
  hw.update();
  delay(150);
  hw.setColor(BLUE);
  hw.update();
  delay(150);
  hw.setColor(WHITE);
  hw.update();
  delay(150);
  hw.setColor(BLACK);
  hw.setLed(LED_1,false);
  hw.setLed(LED_2,false);
  hw.setLed(LED_3,false);
  hw.update();

}



////////////////////////////////////////////////////////////






#define COUNTER_LIMIT 32

int counter;
boolean flip;
boolean sendSwitch;


void testMode(){
  MIDI.read();
  counter++;
  if(counter>COUNTER_LIMIT) counter=0, flip=!flip, randomize(hw.soundFromSwitches());
  if(flip) anySound();
  else stopSound();


  if(hw.justPressed(SMALL_BUTTON_1)) page=increaseValue(page,2) , MIDI.sendControlChange(13,page,1);
  if(hw.justPressed(SMALL_BUTTON_2)) page=decreaseValue(page,2), MIDI.sendControlChange(13,page,1);
  hw.setColor(page+1);


  for(int i=0;i<3;i++){
    if(hw.justPressed(i))  hw.flipSwitch(i), sendSwitch=true;
    hw.setLed(i,hw.switchState(i));
    hw.unfreezeKnob(i);
    if(hw.knobMoved(i)) MIDI.sendControlChange(10+i,hw.knobValue(i)>>3,1),  hw.setColor(WHITE);
  }
  if(sendSwitch) MIDI.sendControlChange(14,hw.soundFromSwitches(),1), sendSwitch=false;


  if(hw.buttonState(EXTRA_BUTTON_1) || hw.buttonState(EXTRA_BUTTON_2) ){
    for(int i=0;i<3;i++)  hw.setSwitch(i,random(2));
  }

}

void anySound(){

  //randomize(1);
  renderTweaking(0);
  renderTimeStretch();
  playSound(1,DEFAULT_VELOCITY);


}
void stopSound(){
renderTimeStretch();
}



int decreaseValue(int _VALUE, int _OVERFLOW){
  if(_VALUE > 0) _VALUE--;
  else _VALUE=_OVERFLOW;
  return _VALUE;
}

