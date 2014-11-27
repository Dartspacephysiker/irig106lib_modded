/****************************************************************************

 i106_decode_analogf1.c -

 Copyright (c) 2014 Irig106.org

 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

   * Neither the name Irig106.org nor the names of its contributors may
     be used to endorse or promote products derived from this software
     without specific prior written permission.

 This software is provided by the copyright holders and contributors
 "as is" and any express or implied warranties, including, but not
 limited to, the implied warranties of merchantability and fitness for
 a particular purpose are disclaimed. In no event shall the copyright
 owner or contributors be liable for any direct, indirect, incidental,
 special, exemplary, or consequential damages (including, but not
 limited to, procurement of substitute goods or services; loss of use,
 data, or profits; or business interruption) however caused and on any
 theory of liability, whether in contract, strict liability, or tort
 (including negligence or otherwise) arising in any way out of the use
 of this software, even if advised of the possibility of such damage.

 Author: Spencer Hatch, Dartmouth College, Hanover, NH, USA
 *STOLEN* from Hans-Gerhard Flohr's i106_decode_analogf1.c
2014 NOV Initial Version 1.0


 ****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <assert.h>

#include "config.h"
#include "stdint.h"

#include "irig106ch10.h"
#include "i106_time.h"
#include "i106_decode_tmats.h"
#include "i106_decode_analogf1.h"

#ifdef __cplusplus
namespace Irig106 {
#endif


/*
 * Macros and definitions
 * ----------------------
 */

/*
 * Data structures
 * ---------------
 */


/*
 * Module data
 * -----------
 */

    // For test with the data of a known (special) file 
    #ifdef _DEBUG
        //#define DEBUG_OTHER_ANALOG_FILE
        #ifdef DEBUG_OTHER_ANALOG_FILE
            static FILE *FileAnalogTest = NULL;
        #endif
    #endif


/*
 * Function Declaration
 * --------------------
 */

// Local functions
EnI106Status PrepareNextDecodingRun_AnalogF1(SuAnalogF1_CurrMsg * psuMsg);

/* ======================================================================= */

/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106_Decode_FirstAnalogF1(SuI106Ch10Header     * psuHeader,
    void            * pvBuff,
    SuAnalogF1_CurrMsg * psuMsg)
{
    uint32_t ulSubPacketLen;
    uint32_t uRemainder;

    // Check for attributes available
    if(psuMsg->psuAttributes == NULL)
        return I106_UNSUPPORTED;

    // Set pointers to the beginning of the Analog buffer
    psuMsg->psuHeader = psuHeader; 
    psuMsg->psuChanSpec = (SuAnalogF1_ChanSpec *)pvBuff; 

    // Check whether number of subchannels reported by TMATS matches number reported by CSDW
    if(psuMsg->psuAttributes->iAnalogChansPerPkt != psuMsg->apsuChanSpec[

    psuMsg->uBytesRead = 0;
    psuMsg->ulDataLen = psuHeader->ulDataLen;
    psuMsg->uBytesRead += sizeof(SuAnalogF1_ChanSpec);

    // Check for no (more) data
    if (psuMsg->ulDataLen <= psuMsg->uBytesRead)
        return I106_NO_MORE_DATA;

    // ------------------------------------

    //This was used to check for remaining data based on subpacket--do we need to do anything
    //similar?
    /* if(psuMsg->ulDataLen < psuMsg->uBytesRead + psuMsg->ulSubPacketLen) */
    /*     return I106_NO_MORE_DATA; */

    // Set the pointer to the Analog message data
    psuMsg->pauData = (uint8_t *)((char *)(psuMsg->psuChanSpec) + psuMsg->uBytesRead);

    // Prepare the Analog buffers and load the first bits
    if(psuMsg->psuAttributes->bPrepareNextDecodingRun)
    {

        // Set up the data
        EnI106Status enStatus = PrepareNextDecodingRun_AnalogF1(psuMsg);
        if(enStatus != I106_OK)
            return enStatus;
    }

    // Now start the decode of this buffer
    psuMsg->psuAttributes->ulBitPosition = 0;

    return (DecodeBuff_AnalogF1(psuMsg));
}


/* ----------------------------------------------------------------------- */

//SPENCE CHECK--Maybe this function is useless now...
EnI106Status I106_CALL_DECL 
    enI106_Decode_NextAnalogF1(SuAnalogF1_CurrMsg * psuMsg)
{
  return (DecodeBuff_AnalogF1(psuMsg));
}

/* ----------------------------------------------------------------------- */

// Fill the attributes from TMATS 
// ToDo: Check if all needed definitions found
//SPENCE CHECK--this one compiles

EnI106Status I106_CALL_DECL Set_Attributes_AnalogF1(SuRDataSource * psuRDataSrc, SuAnalogF1_Attributes * psuAnalogF1_Attributes)
{

    uint32_t            uBitCount;

    if(psuAnalogF1_Attributes == NULL) return I106_INVALID_PARAMETER; // Set Attributes

    memset(psuAnalogF1_Attributes, 0, sizeof(SuAnalogF1_Attributes));

    // Collect the TMATS values
    // ------------------------
    //NOTE, THESE ARE ALL CSDWs, NOT TMATS INFO
    //What values do we need to process analog data, Spence?
    //      uint32_t    uMode           :  2;      // 
    //      uint32_t    uLength         :  6;      // Bits in A/D value
    //      uint32_t    uSubChan        :  8;      // Subchannel number
    //      uint32_t    uTotChan        :  8;      // Total number of subchannels
    //      uint32_t    uFactor         :  4;      // Sample rate exponent
    //      uint32_t    bSame           :  1;      // One/multiple Channel Specific
    

    psuAnalogF1_Attributes->psuRDataSrc                = psuRDataSrc; // May be, we need it in the future

    psuAnalogF1_Attributes->iDataSourceNum             = psuRDataSrc->iDataSourceNum; // R-x

    //Get number of chans per packet
    if(psuRDataSrc->szAnalogChansPerPkt != NULL)
      psuAnalogF1_Attributes->iAnalogChansPerPkt = atoi(psuRDataSrc->szAnalogChansPerPkt);

    //Get sample rate
    if(psuRDataSrc->szAnalogSampleRate != NULL)
       psuAnalogF1_Attributes->ullAnalogSampleRate = strtoull(psuRDataSrc->szAnalogSampleRate, NULL, 10);

    //Get size of a data sample on this channel
    if(psuRDataSrc->szAnalogDataLength != NULL)
       psuAnalogF1_Attributes->ulAnalogDataLength = strtoul(psuRDataSrc->szAnalogDataLength, NULL, 10);

    if(psuRDataSrc->szAnalogMeasTransfOrd != NULL)    // R-x\AMTO-n-m most significant bit "M", least significant bit "L". default: M
    {
        /* Measurement Transfer Order. Which bit is being transferred first is specified as – Most Significant Bit (M), 
        Least Significant Bit (L), or Default (D).
        D-1\MN3-1-1:M;
        */
        if(psuRDataSrc->szAnalogMeasTransfOrd[0] == 'L')
        {
            psuAnalogF1_Attributes->ulAnalogMeasTransfOrd = ANALOG_LSB_FIRST;
            return(I106_UNSUPPORTED);
        }
    }

    //Get Sample Filter 3dB Bandwidth (in Hz)
    if(psuRDataSrc->szAnalogSampleFilter != NULL)
       psuAnalogF1_Attributes->ullAnalogSampleFilter = strtoull(psuRDataSrc->szAnalogSampleFilter, NULL, 10);

    //Get whether AC/DC Coupling
    if(psuRDataSrc->szAnalogIsDCCoupled != NULL)
      psuAnalogF1_Attributes->bAnalogIsDCCoupled = psuRDataSrc->bAnalogIsDCCoupled;

    //Get Recorder Input Impedance 
    if(psuRDataSrc->szAnalogRecImpedance != NULL)
       psuAnalogF1_Attributes->ulAnalogRecImpedance = strtoul(psuRDataSrc->szAnalogRecImpedance, NULL, 10);

    //Get Channel Gain in milli units (10x = 010000)
    if(psuRDataSrc->szAnalogChanGain != NULL)
       psuAnalogF1_Attributes->ulAnalogChanGain = strtoul(psuRDataSrc->szAnalogChanGain, NULL, 10);

    //Get Full-Scale Range (in milliVolts)
    if(psuRDataSrc->szAnalogFullScaleRange != NULL)
       psuAnalogF1_Attributes->ulAnalogFullScaleRange = strtoul(psuRDataSrc->szAnalogFullScaleRange, NULL, 10);

    //Get Offset Voltage (in milliVolts)
    if(psuRDataSrc->szAnalogOffsetVoltage != NULL)
       psuAnalogF1_Attributes->lAnalogOffsetVoltage = strtoul(psuRDataSrc->szAnalogOffsetVoltage, NULL, 10);

    //Get LSB Value
    if(psuRDataSrc->szAnalogLSBValue != NULL)
       psuAnalogF1_Attributes->lAnalogLSBValue = strtoul(psuRDataSrc->szAnalogLSBValue, NULL, 10);

    //Get Analog Format 
    //"1" = One's comp. 
    //"2" = Two's comp.
    //"3" = Sign and magnitude binary [+=0]
    //"4" = Sign and magnitude binary [+=1]
    //"B" = Offset binary
    //"U" = Unsigned binary
    //"F" = IEEE 754 single-precision [IEEE 32] floating point
    if(psuRDataSrc->szAnalogFormat != NULL)
      switch ( psuRDataSrc->szAnalogFormat[0] )
      {
      case '1':
	psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_ONES;
	break;
      case '2':
	psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_TWOS; 
	break;
      case '3':
	psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_SIGNMAG_0;
	break;
      case '4':
	psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_SIGNMAG_1;
	break;
      case 'B':
	psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_OFFSET_BIN;
	break;
      case 'U':
	psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_UNSIGNED_BIN;
	break;
      case 'F':
	psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_SINGLE_FLOAT;
	break;
      default:
	return(I106_UNSUPPORTED);
        break;
      }

    //Get analog input type; 'D' = differential, 'S' = single-ended
    if(psuRDataSrc->szAnalogDifferentialInp != NULL)
       psuAnalogF1_Attributes->bAnalogDifferentialInp = psuRDataSrc->bAnalogDifferentialInp;
 
    //Get whether audio
    if(psuRDataSrc->szAnalogIsAudio != NULL)
       psuAnalogF1_Attributes->bAnalogIsAudio = psuRDataSrc->bAnalogIsAudio;

    // Some post processing
    /* if(psuAnalogF1_Attributes->ulBitsInMinorFrame == 0) */
    /* { */
    /*     psuAnalogF1_Attributes->ulBitsInMinorFrame = psuAnalogF1_Attributes->ulCommonWordLen * (psuAnalogF1_Attributes->ulWordsInMinorFrame - 1) + */
    /*         psuAnalogF1_Attributes->ulMinorFrameSyncPatLen; */
    /* } */
    /* for(uBitCount = 0; uBitCount < psuAnalogF1_Attributes->ulCommonWordLen; uBitCount++) */
    /* { */
    /*     psuAnalogF1_Attributes->ullCommonWordMask <<= 1; */
    /*     psuAnalogF1_Attributes->ullCommonWordMask |= 1; */
    /* } */
        
    psuAnalogF1_Attributes->bPrepareNextDecodingRun = 1; // Set_Attributes_AnalogF1
        
    return I106_OK;
} // End Set_Attributes _AnalogF1

/* ----------------------------------------------------------------------- */

// Create the output buffers for a minor frame (data and error flags)
//SPENCE CHECK--THIS SHOULD WORK FINE
EnI106Status I106_CALL_DECL 
  CreateOutputBuffers_AnalogF1(SuAnalogF1_Attributes * psuAttributes, uint32_t ulDataLen)
{

    // Allocate the Analog output buffer
    psuAttributes->ulOutBufSize = ulDataLen;
    psuAttributes->paullOutBuf = (uint64_t *)calloc(sizeof(uint64_t), ulDataLen);
    if(psuAttributes->paullOutBuf == NULL)
        return I106_BUFFER_TOO_SMALL;
    
    psuAttributes->pauOutBufErr = (uint8_t *)calloc(sizeof(uint8_t), ulDataLen);
    if(psuAttributes->pauOutBufErr == NULL)
    {
        free(psuAttributes->paullOutBuf); psuAttributes->paullOutBuf = NULL;
        return I106_BUFFER_TOO_SMALL;
    }
    return(I106_OK);
} // End CreateOutputBuffers

/* ----------------------------------------------------------------------- */

// Free the output buffers for a minor frame
EnI106Status I106_CALL_DECL FreeOutputBuffers_AnalogF1(SuAnalogF1_Attributes * psuAnalogAttributes)
{

    if(psuAnalogAttributes->paullOutBuf)
    {
        free(psuAnalogAttributes->paullOutBuf);
        psuAnalogAttributes->paullOutBuf = NULL;
    }
    if(psuAnalogAttributes->pauOutBufErr)
    {
        free(psuAnalogAttributes->pauOutBufErr);
        psuAnalogAttributes->pauOutBufErr = NULL;
    }
    psuAnalogAttributes->bPrepareNextDecodingRun = 1; 

    return(I106_OK);
} // End FreeOutputBuffers

/* ----------------------------------------------------------------------- */

// Prepare a new decoding run 
// Creates the output buffers and resets values and counters
//SPENCE CHECK--Might need some edits to remove PCM stuff, but should be OK
EnI106Status PrepareNextDecodingRun_AnalogF1(SuAnalogF1_CurrMsg * psuMsg)
{
    SuAnalogF1_Attributes * psuAttributes = psuMsg->psuAttributes;

    EnI106Status enStatus = CreateOutputBuffers_AnalogF1(psuAttributes, psuMsg->ulDataLen);
    if(enStatus != I106_OK)
        return(enStatus);

    psuAttributes->bPrepareNextDecodingRun = 0;
    
    psuAttributes->lSaveData = 0;

    return I106_OK;

} // End PrepareNextDecodingRun

/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    DecodeBuff_AnalogF1(SuAnalogF1_CurrMsg * psuMsg)
{

    SuAnalogF1_Attributes * psuAttributes = psuMsg->psuAttributes;

    while(psuAttributes->ulBitPosition < psuMsg->ulDataLen)
    {

        GetNextBit_PcmF1(psuMsg, psuAttributes);

        // Check for a sync word

        if(IsSyncWordFound_PcmF1(psuAttributes))
        {   
            // Prevent an overflow after a terabyte of bits
            if(psuAttributes->ullBitsLoaded > 1000000000000)
                psuAttributes->ullBitsLoaded = 1000000000;

            psuAttributes->ullSyncCount++;

            //TRACE("Sync word found at BitPos %6d, MFBitCnt %5d, 0x%08X, SyncCnt %6d\n", 
            //  psuAttributes->ulBitPosition, psuAttributes->ulMinorFrameBitCount, (int32_t)psuAttributes->ullTestWord, psuAttributes->ullSyncCount);

            if(psuAttributes->ulMinorFrameBitCount == psuAttributes->ulBitsInMinorFrame)
            {
                // A sync word found at the correct offset to the previous one

                RenewSyncCounters_PcmF1(psuAttributes, psuAttributes->ullSyncCount); // with the current sync counter
                
                // If there are enough syncs, release the previous filled outbuf
                // Note: a minor frame is released only, if it is followed by a sync word at the correct offset. 
                // i.e. the sync word are used as brackets
                if((psuAttributes->ullSyncCount >= psuAttributes->ulMinSyncs) && (psuAttributes->lSaveData > 1)) 
                {

                    // Compute the intrapacket time of the start sync bit position in the current buffer
                    int64_t llBitPosition = (int64_t)psuAttributes->ulBitPosition - (int64_t)psuAttributes->ulBitsInMinorFrame /*- (int64_t)psuAttributes->ulMinorFrameSyncPatLen*/;

                    double dOffsetIntPktTime = (double)llBitPosition * psuAttributes->dDelta100NanoSeconds;   

                    psuMsg->llIntPktTime = psuMsg->llBaseIntPktTime + (int64_t)dOffsetIntPktTime; // Relative time, omit rounding

                    // Prepare for the next run
                    PrepareNewMinorFrameCollection_PcmF1(psuAttributes);
                    return I106_OK;

                }

            }
            else
            {
                // A sync word at the wrong offset, throw away all
                // Note: a wrong offset is also at the first sync in the whole decoding run

                // Save the sync error for statistics
                if(psuAttributes->ullSyncCount > 0)
                    psuAttributes->ullSyncErrors++;

                // RenewSyncCounters_PcmF1 with a sync counter of zero
                RenewSyncCounters_PcmF1(psuAttributes, 0);
            }

            PrepareNewMinorFrameCollection_PcmF1(psuAttributes);
            continue;

        } // if sync found

        // Collect the data

        if(psuAttributes->lSaveData == 1)
        {
            psuAttributes->ulDataWordBitCount++;
            if(psuAttributes->ulDataWordBitCount >= psuAttributes->ulCommonWordLen)
            {
                psuAttributes->paullOutBuf[psuAttributes->ulMinorFrameWordCount - 1] = psuAttributes->ullTestWord;
                psuAttributes->ulDataWordBitCount = 0;
                //TRACE("MFWC %d 0x%I64x\n", psuAttributes->ulMinorFrameWordCount - 1, psuAttributes->paullOutBuf[psuAttributes->ulMinorFrameWordCount - 1]);
                psuAttributes->ulMinorFrameWordCount++;
            }
        }
        if(psuAttributes->ulMinorFrameWordCount >= psuAttributes->ulWordsInMinorFrame)
        {
            psuAttributes->lSaveData = 2;

            // Don't release the data here but wait for a trailing sync word. 
        }
    } // end while

    // Preset for the next run
    psuAttributes->ulBitPosition = 0;

  return I106_NO_MORE_DATA;
} //End DecodeMinorFram_PcmF1

/* ----------------------------------------------------------------------- */
// Returns I106_OK on success, I106_INVALID_DATA on error
/* EnI106Status I106_CALL_DECL */
/*     CheckParity_AnalogF1(uint64_t ullTestWord, int iWordLen, int iParityType, int iParityTransferOrder) */
/*           // check the parity of a word */
/* { */
/*     uint64_t ullTestBit = 1; */
/*     unsigned int uBitSum = 0; */

/*     switch(iParityType) */
/*     { */
/*     case ANALOG_PARITY_NONE: */
/*         break; */
/*     case ANALOG_PARITY_EVEN: */
/*         while(iWordLen-- > 0) */
/*         { */
/*             if(ullTestWord & ullTestBit) uBitSum++; */
/*             ullTestBit <<= 1; */
/*         } */
/*         if(uBitSum & 1) return(I106_INVALID_DATA); */
/*         break; */
/*     case ANALOG_PARITY_ODD: */
/*         while(iWordLen-- > 0) */
/*         { */
/*             if(ullTestWord & ullTestBit) uBitSum++; */
/*             ullTestBit <<= 1; */
/*         } */
/*         if( ! (uBitSum & 1)) return(I106_INVALID_DATA); */
/*         break; */
/*     default: // none */
/*         break; */
/*     } */
/*     return(I106_OK); */
/* } */

/* ----------------------------------------------------------------------- */
// Swaps nBytes in place
EnI106Status I106_CALL_DECL SwapBytes_AnalogF1(uint8_t *pubBuffer, long nBytes)
{
    uint32_t idata = 0x03020100;
    uint8_t ubTemp;
    if(nBytes & 1)
        return(I106_BUFFER_OVERRUN); // May be also an underrun ...
    while((nBytes -= 2) >= 0)
    {
        ubTemp = *pubBuffer;
        *pubBuffer = *(pubBuffer + 1);
        *++pubBuffer = ubTemp;
        pubBuffer++;
    }
    SwapShortWords_AnalogF1((uint16_t *)&idata, 4);

    return(I106_OK);
}

/* ----------------------------------------------------------------------- */
// Swaps nbytes of 16 bit words in place
EnI106Status I106_CALL_DECL SwapShortWords_AnalogF1(uint16_t *puBuffer, long nBytes)
{
    long Counter = nBytes;
    uint16_t ubTemp;
    if(nBytes & 3)
        return(I106_BUFFER_OVERRUN); // May be also an underrun ...
    Counter >>= 1;
    while((Counter -= 2) >= 0)
    {
        ubTemp = *puBuffer;
        *puBuffer = *(puBuffer + 1);
        *++puBuffer = ubTemp;
        puBuffer++;
    }
    return(I106_OK);
}

#ifdef __cplusplus
} // end namespace
#endif

