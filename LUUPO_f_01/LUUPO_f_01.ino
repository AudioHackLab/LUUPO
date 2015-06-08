/*
just an mp3 player with same intresting function for us...:-)
works also like a sequencer

AHL: info@audiohacklab.org
*/


#include <SPI.h>
//Add the SdFat Libraries
#include <SdFat.h>
#include <SdFatUtil.h>
//and the MP3 Shield Library
#include <SFEMP3Shield.h>
#include "mp3stuff.h"  //file name

SdFat sd;
SFEMP3Shield MP3player;

const int buttons = A0;  //pin where is attacced the 4oe play's button
const int bankb = A1;
const int timePin = A2;
const int modePot = A4;
const int offsetBtn = A3;
const int erasePin = 19;
int nstep = 0;//16;
unsigned long tempo;

int laststate[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint8_t result;

int sequencer[] = {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100};

int banks[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int offsets[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int stepp = 0;

int offset = 0;

int dtime = 0;

void setup() {
  result; //result code from some function as to be tested at later time.
  Serial.begin(115200);

  //Initialize the SdCard.
  if (!sd.begin(SD_SEL, SPI_HALF_SPEED)) sd.initErrorHalt();
  if (!sd.chdir("/")) sd.errorHalt("sd.chdir");

  //Initialize the MP3 Player Shield
  result = MP3player.begin();
  //check result, see readme for error codes.
  if (result != 0) {
    Serial.print(F("Error code: "));
    Serial.print(result);
    Serial.println(F(" when trying to start MP3 player"));
    if ( result == 6 ) {
      Serial.println(F("Warning: patch file not found, skipping.")); // can be removed for space, if needed.
    }
  }
  pinMode(erasePin, INPUT);
}

void loop() {
  nstep = ((1023-analogRead(modePot)) /511)*8; //decite the number of step:0-8-16
  
  if (stepp >= nstep) stepp = 0;//

  laststate[stepp] = 0;   //to avoid that the track play when you press and in the moment which the stepp start

  int bank = analogRead(bankb)>>8; //decide which bank is available
  int butt = readButtons(analogRead(buttons));//read the 4 buttons and store it in butt

  int lastoffset = offset;//is to avoid to record the offset if yuo leave the pot in the same position
  offset = 4 * analogRead(offsetBtn);  // read the offset
  if (offset > lastoffset + 20 || offset < lastoffset - 20) offsets[stepp] = offset - 40;  //write the offset in the array
  if (offset <= 40) offsets[stepp] = 0;  //to reset the offset of the step

  if (nstep!=0) {//enter here if the nstep si more then 0
    //*****************************************SEQUENCER MODE******************************************************
    Serial.println("sequencer");
    if (butt < 100) {  //if a button is pressed
      sequencer[stepp] = butt;  //store the button pressed
      banks[stepp] = bank;      //store the bank chosed
      printstuff(banks[stepp], sequencer[stepp], offset); //just printing someting
      playT (sequencer[stepp] + banks[stepp] * 4, offsets[stepp] );  //play the track of the bank with the offset
      laststate[stepp] = 1;  //to avoid to play also when the sequencer normaly play
    }

    if (digitalRead(erasePin)) {//to erase the bank and track of this step
      sequencer[stepp] = 103;
      banks[stepp] = 0;
    }

    if (sequencer[stepp] < 100 && laststate[stepp] == 0 ) {  //playing the normaly sequencer 
      Serial.println("playing");
      playT (sequencer[stepp] + banks[stepp] * 4, offsets[stepp] );
      laststate[stepp] = 0;  
    }
    Serial.print(" \n ");
    //delay(analogRead(timePin));
  }

  else {//enter here if the nstep is 0
    //***********************************FREE MODE***************************************
    if (butt < 100) {
      printstuff(bank, butt, offset);
      playT ((bank * 4) + butt , offset );
    }
    else {
      //laststate[butt + bank] = 0;
    }
    if (digitalRead(erasePin)) {  //if you erase when you are not in the sequencer mode you erase the whole pattern
      for (int u = 0; u < nstep; u++) {
        sequencer[stepp] = 103;
        banks[stepp] = 0;
        offsets[stepp] = 0;
      }
    }
  }
  unsigned long taim = ( long((1023 - analogRead(timePin)) / 14) * long((1023 - analogRead(timePin)) / 14));//the time of reproducing
  //Serial.println(taim);
  for (unsigned long d = 0; d < (long(5) + taim); d++) { // to have time from few ms to more then a second whit a precision of 100us
    delayMicroseconds(100);
  }
  //Serial.print(millis() - tempo);
  Serial.print(" \n ");
  tempo = millis();
  stepp++;
}


void playT (int h, int offs) {
  if (MP3player.isPlaying()) {
    MP3player.stopTrack();
  }
  result = MP3player.playMP3(filename[h], offs);

}



byte readButtons(int bUtt) {
  byte i ;
  if (bUtt > 400) {
    i = 0;
    return i;
  }
  if (bUtt > 70 && bUtt < 130) {
    i = 1;
    return i;
  }
  if (bUtt < 210 && bUtt >= 130) {
    i = 2;
    return i;
  }
  if (bUtt < 400 && bUtt >= 210) {
    i = 3;
    return i;
  }
  if (bUtt <= 70) {
    i = 100;
    return i;
  }
  //return i;
}




void printstuff(int b1, int button, int offs) {
  Serial.print(" i'm playng track :");
  Serial.print(button + b1 * 4 );
  Serial.print(" the bank is :");
  Serial.print(b1);
  Serial.print(" the button is :");
  Serial.print(button);
  Serial.print(" seq ");
  Serial.print(sequencer[stepp]);
  Serial.print(" step ");
  Serial.print(stepp);
  Serial.print("  con offsets ");
  Serial.print(offsets[stepp]);
  Serial.print(" offset ");
  Serial.print(offset);
  /*Serial.print(" end time :");
  Serial.print(endTime);
  Serial.print(" with a offset of :");
  Serial.println(offs);*/
}

