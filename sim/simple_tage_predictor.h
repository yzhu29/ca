///////////////////////////////////////////////////////////////////////
////  Copyright 2015 Samsung Austin Semiconductor, LLC.                //
/////////////////////////////////////////////////////////////////////////
// one bit smith predictor: size 1024Kbits
#ifndef _PREDICTOR_H_
#define _PREDICTOR_H_

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include "utils.h"
#include "bt9.h"
#include "bt9_reader.h"

#include <bitset>
//#define PHT_CTR_MAX  3
//#define PHT_CTR_INIT 2

//#define HIST_LEN   17

//NOTE competitors are allowed to change anything in this file include the following two defines
//ver2 #define FILTER_UPDATES_USING_BTB     0     //if 1 then the BTB updates are filtered for a given branch if its marker is not btbDYN
//ver2 #define FILTER_PREDICTIONS_USING_BTB 0     //if 1 then static predictions are made (btbANSF=1 -> not-taken, btbATSF=1->taken, btbDYN->use conditional predictor)

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

class PredictionResult{
    public:
    UINT32 id;
    bool direction;
    UINT32 tagHit;
    UINT32 u;
};

class BasePredictor{

 private:
 UINT32 id;
 UINT32 historyLen;
 UINT32 phtIndexLen;
 UINT32 tagLen;
 UINT32 satLen;        // How many bit is a saturation counter entry
 UINT32 uLen;           // How many bit is a "u"
 UINT32 uMax;
 UINT32 counterMax;

 UINT32 tagHead = tagLen + phtIndexLen - 1;
 UINT32 tagTail = phtIndexLen;
 //UINT32 phtIndex;
 UINT32 *pht;          // pattern history table
 UINT32 *tagTable;      //Tag table
 UINT32 *uTable;        // "u" table (usefulness)

 UINT32 numPhtEntries; // entries in pht

 virtual UINT32 hashIndex(UINT64 PC, UINT64 ghrH, UINT64 ghrL);

 public:
  BasePredictor(UINT32 id, UINT32 historyLen, UINT32 phtIndexLen, UINT32 tagLen=11, UINT32 uLen=2, UINT32 satLen=3);

  // The interface to the functions below CAN NOT be changed
  PredictionResult    GetPrediction(UINT64 PC, UINT64 ghrH, UINT64 ghrL);
  void    UpdatePredictor(UINT64 PC, UINT64 ghrH, UINT64 ghrL, bool decU, bool wasTagHit, UINT32 allocEntryId, bool resolveDir, bool predDir, UINT32 providerId, UINT32 alternateId, bool alternateMispred); // alternateScrewed= 1 means the alternate predictor mis predicted.
  // Contestants can define their own functions below
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

class PREDICTOR{

  // The state is defined for Gshare, change for your design

 private:
    UINT64    ghrH;
    UINT64    ghrL;
    BasePredictor defaultPredictor;
    BasePredictor t1;
    UINT32 M = 1;

    PredictionResult alternateResult;
    PredictionResult providerResult;

    PredictionResult defaultResult;
    PredictionResult result1;
    bool alternateExist;

 public:


  PREDICTOR(void) : defaultPredictor(0, 0, 15, 0),  t1(1, 2, 12) {}

  // The interface to the functions below CAN NOT be changed
  bool    GetPrediction(UINT64 PC);
  void    UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget);
  void    TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget);

  // Contestants can define their own functions below
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

//NOTE contestants are not allowed to use the btb* information from ver2 of the simulation infrastructure. Interface to this function CAN NOT be changed.

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////
////  Copyright 2015 Samsung Austin Semiconductor, LLC.                //
/////////////////////////////////////////////////////////////////////////



BasePredictor::BasePredictor(UINT32 id, UINT32 historyLen, UINT32 phtIndexLen, UINT32 tagLen, UINT32 uLen, UINT32 satLen){
    this->id = id;
    this->historyLen = historyLen;
    this->phtIndexLen = phtIndexLen;
    this->tagLen = tagLen;
    this->uLen = uLen;
    this->satLen = satLen;
    this->numPhtEntries = (1 << phtIndexLen);
    this->counterMax = (1 << satLen);
    this->uMax = (1 << uLen);
    this->tagHead = tagLen + phtIndexLen - 1;
    this->tagTail = phtIndexLen;
    #ifdef DEBUGGING
    std::cout << "numPhtEntries is " << numPhtEntries << ", or " << log2(numPhtEntries) << " bit" std::endl;
    #endif // DEBUGGING
    pht = new UINT32[numPhtEntries];
    if (tagLen!=0) {
        tagTable = new UINT32[numPhtEntries];
    }
    uTable = new UINT32[numPhtEntries];

    for (UINT32 i=0; i< numPhtEntries; i++){
        pht[i] = counterMax / 2; // Initialize everything to weak Taken.
        if (tagLen!=0) {
            tagTable[i] = 0;
        }
        uTable[i] = 0;
    }
}

UINT32 BasePredictor::hashIndex(UINT64 PC, UINT64 ghrH, UINT64 ghrL){
    UINT32 resultIndex = (PC ^ ghrL) % numPhtEntries;
    return resultIndex;
}

PredictionResult BasePredictor::GetPrediction(UINT64 PC, UINT64 ghrH, UINT64 ghrL){
    UINT32 resultIndex = hashIndex(PC, ghrH, ghrL);
    PredictionResult result;
    result.id = id;
    result.direction = pht[resultIndex];

    if (tagLen!=0) {
        UINT32 tag = tagTable[resultIndex];
        UINT32 tagPC = (PC % (tagHead+1)) >> tagTail;
        if (tag == tagPC){
            result.tagHit = 1;
        }
        else {
            result.tagHit = 0;
        }
    }
    else{
        result.tagHit = 0;  // when no tag
    }
    result.u = uTable[resultIndex];
    return result;
}

void BasePredictor::UpdatePredictor(UINT64 PC, UINT64 ghrH, UINT64 ghrL, bool decU, bool wasTagHit, UINT32 allocEntryId, bool resolveDir, bool predDir, UINT32 providerId, UINT32 alternateId, bool alternateMispred){
    UINT32 resultIndex = hashIndex(PC, ghrH, ghrL);
    // updating U
    if (wasTagHit && alternateMispred && (providerId==id)){
        UINT32 resultU = uTable[resultIndex];
        if (resolveDir == predDir){
            uTable[resultIndex] = SatIncrement(resultU, uMax);
        }
        else{
            uTable[resultIndex] = SatDecrement(resultU);
        }
    }

    if (decU){
        UINT32 resultU = uTable[resultIndex];
        uTable[resultIndex] = SatDecrement(resultU);
    }

    // updating saturation counter
    //if (wasTagHit && (providerId==id)){ // try change to:   if (wasTagHit)
    if (wasTagHit){
        UINT32 resultPhtEntry = pht[resultIndex];
        if (resolveDir) {
            pht[resultIndex] = SatIncrement(resultPhtEntry, counterMax);
        }
        else {
            pht[resultIndex] = SatDecrement(resultPhtEntry);
        }
    }

    // Allocate Entry
    if (allocEntryId == id){
        if (tagLen!=0) {
            UINT32 tagPC = (PC % (tagHead+1)) >> tagTail;
            tagTable[resultIndex] = tagPC;
        }
        if (resolveDir) {
            pht[resultIndex] = counterMax/2;
        }
        else{
            pht[resultIndex] = counterMax/2 - 1;
        }
        uTable[resultIndex] = 0;
    }
    return;
}






//NOTE contestants are not allowed to use the btb* information from ver2 of the simulation infrastructure. Interface to this function CAN NOT be changed.
bool   PREDICTOR::GetPrediction(UINT64 PC){
    defaultResult = defaultPredictor.GetPrediction(PC, ghrH, ghrL);
    result1 = t1.GetPrediction(PC, ghrH, ghrL);
    if (result1.tagHit){
        providerResult = result1;
        alternateResult = defaultResult;
        alternateExist = 1;
        return result1.direction;
    }
    else {
        providerResult = defaultResult;
        alternateExist = 0;
        return defaultResult.direction;
    }
}

void  PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget){
    bool alternateMispred = 0;
    UINT32 alternateId = 69; // Means No alternate
    UINT32 allocEntryId = 69;
    bool decU0 = 0;
    bool decU1 = 0;

    // update u
    if(alternateExist){
        alternateId = alternateResult.id;
        if (alternateResult.direction != resolveDir){
            alternateMispred = 1;
        }
    }


    if(resolveDir!=predDir){
        if (providerResult.id != M){
            if (result1.id > providerResult.id){
                if (result1.u == 0){
                    allocEntryId = result1.id;
                }
                else {
                    decU0 = 1;
                }
            }
        }
    }

    defaultPredictor.UpdatePredictor(PC, ghrH, ghrL,decU0, defaultResult.tagHit, allocEntryId, resolveDir, predDir, providerResult.id, alternateId, alternateMispred);
    t1.UpdatePredictor(PC, ghrH, ghrL,decU1, result1.tagHit, allocEntryId, resolveDir, predDir, providerResult.id, alternateId, alternateMispred);

  // update the GHR
  bool ghrLMSB = ghrL >> 63;
  ghrH = (ghrH << 1);
  ghrH = ghrH + ghrLMSB;
  ghrL = (ghrL << 1);

  if(resolveDir == TAKEN){
    ghrL++;
  }
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void    PREDICTOR::TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget){

  // This function is called for instructions which are not
  // conditional branches, just in case someone decides to design
  // a predictor that uses information from such instructions.
  // We expect most contestants to leave this function untouched.

  return;
}




/***********************************************************/
#endif

