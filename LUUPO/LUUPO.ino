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
const int bank2 = 16;
const int timePin = A2;
const int modePot = A4;
const int offsetBtn = A3;
const int erasePin = A5;
const int nstepp = 8;//16;

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

}

void loop() {
  if (stepp >= nstepp) stepp = 0;

  laststate[stepp] = 0;

  int lastbank = banks[stepp];
  int bank = analogRead(bankb) / 256; //se 0 legge il campione se 1 legge il bank
  int butt = readButtons(analogRead(buttons));//legge quale dei 4 bottoni sono stati premuti

  int lastoffset = offset;
  offset = 4 * analogRead(offsetBtn);
  if (offset > lastoffset + 20 || offset < lastoffset - 20) offsets[stepp] = offset - 40;
  if (offset <= 40) offsets[stepp] = 0;

  if (analogRead(modePot) > 500) {
    if (butt < 100) {

      sequencer[stepp] = butt;
      banks[stepp] = bank;

      printstuff(banks[stepp], sequencer[stepp], offset);
      playT (sequencer[stepp] + banks[stepp] * 4, offsets[stepp] );
      laststate[stepp] = 1;
    }
    else {
      //laststate[butt + bank] = 0;
    }

    if (analogRead(erasePin) > 500) {
      sequencer[stepp] = 100;
      banks[stepp] = 0;
    }
    
    if (sequencer[stepp] < 100 && laststate[stepp] == 0 ) {
      playT (sequencer[stepp] + banks[stepp] * 4, offsets[stepp] );
      Serial.print(" <--playing ");
      laststate[stepp] = 0;
    }
    Serial.print(" \n ");
    delay(analogRead(timePin));
  }

  else {
    if (butt < 100) {
      printstuff(bank, butt, offset);
      playT ((bank * 4) + butt , offset );
    }
    else {
      //laststate[butt + bank] = 0;
    }
  }

  Serial.print(" \n ");
  stepp++;
}


void playT (int h, int offs) {
  if (MP3player.isPlaying()) {
    MP3player.stopTrack();
  }
  //uint8_t result = MP3player.playMP3(filename[h]);
  //uint8_t result;
  result = MP3player.playMP3(filename[h], offs);

}


int readBank(int first, int seconD) {
  return first + (seconD << 1);
}


byte readButtons(int bUtt) {
  byte i ;
  if (bUtt > 500) {
    i = 0;
    return i;
  }
  if (bUtt > 100 && bUtt < 250) {
    i = 1;
    return i;
  }
  if (bUtt < 350 && bUtt >= 250) {
    i = 2;
    return i;
  }
  if (bUtt < 500 && bUtt >= 350) {
    i = 3;
    return i;
  }
  if (bUtt <= 100) {
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
  Serial.print("seq ");
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

