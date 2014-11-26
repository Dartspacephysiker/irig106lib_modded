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
void PrepareNewMinorFrameCollection_AnalogF1(SuAnalogF1_Attributes * psuAttributes);
void GetNextBit_AnalogF1(SuAnalogF1_CurrMsg * psuMsg, SuAnalogF1_Attributes * psuAttributes);
int IsSyncWordFound_AnalogF1(SuAnalogF1_Attributes * psuAttributes);
void RenewSyncCounters_AnalogF1(SuAnalogF1_Attributes * psuAttributes, uint64_t ullSyncCount);



/* ======================================================================= */

// Note; This code is tested only with Analog in throughput mode

/* ----------------------------------------------------------------------- */



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

    psuMsg->uBytesRead = 0;
    psuMsg->ulDataLen = psuHeader->ulDataLen;
    psuMsg->uBytesRead += sizeof(SuAnalogF1_ChanSpec);

    // Check for no (more) data
    if (psuMsg->ulDataLen <= psuMsg->uBytesRead)
        return I106_NO_MORE_DATA;

    // Save the time from the packet header
    //    vTimeArray2LLInt(psuHeader->aubyRefTime, &(psuMsg->llBaseIntPktTime));

    // Take the whole remaining data buffer as packet len
    psuMsg->ulSubPacketLen = psuMsg->ulDataLen - psuMsg->uBytesRead;

    // ------------------------------------

    //This was used to check for remaining data based on subpacket--do we need to do anything
    //similar?
    /* if(psuMsg->ulDataLen < psuMsg->uBytesRead + psuMsg->ulSubPacketLen) */
    /*     return I106_NO_MORE_DATA; */

    // Set the pointer to the Analog message data
    psuMsg->pauData = (uint8_t *)((char *)(psuMsg->psuChanSpec) + psuMsg->uBytesRead);

    //what is all this?
    //for(int count= 0; count < 128; count++)
    //  TRACE(" %2.2X", psuMsg->pauData[count]);
    //TRACE("\n");

    //why are we saying we've read the data? All we did was set the pointer...
    psuMsg->uBytesRead += psuMsg->ulSubPacketLen;

    // Prepare the Analog buffers and load the first bits
    if(psuMsg->psuAttributes->bPrepareNextDecodingRun)
    {

        // Set up the data
        EnI106Status enStatus = PrepareNextDecodingRun_AnalogF1(psuMsg);
        if(enStatus != I106_OK)
            return enStatus;
    }

    if( ! psuMsg->psuChanSpec->bThruMode)
    {
        // Intra-packet not tested, so return
        return I106_UNSUPPORTED;
    }

    if(psuMsg->psuChanSpec->bThruMode)
    {

        #ifdef DEBUG_OTHER_ANALOG_FILE
        int nBytes;
        if(FileAnalogTest == NULL)
            return I106_NO_MORE_DATA;
        // Note: skip the datarec3 header, otherwise we will have about 45 sync errore
        fread(psuMsg->pauData, 1, 6, FileAnalogTest); // skip the datarec3 header
        nBytes = fread(psuMsg->pauData, 1, psuMsg->ulSubPacketLen, FileAnalogTest);
        //TRACE("nBytes %d, SubPacketLen %d, filepos %d\n",nBytes, psuMsg->ulSubPacketLen, ftell(FileAnalogTest));
        //Sleep(200);
        if(nBytes < (int32_t)psuMsg->ulSubPacketLen)
        {
            fclose(FileAnalogTest);
            FileAnalogTest = NULL;
            return I106_NO_MORE_DATA;
        }
        #endif

        if( ! psuMsg->psuAttributes->bDontSwapRawData)
        {
            if(SwapBytes_AnalogF1(psuMsg->pauData, psuMsg->ulSubPacketLen))
                return(I106_INVALID_DATA); 
            // Note: Untested 
            if(psuMsg->psuChanSpec->bAlignment)
            {
                if(SwapShortWords_AnalogF1((uint16_t *)psuMsg->pauData, psuMsg->ulSubPacketLen))
                return(I106_INVALID_DATA); 
            }
        }

        // Now start the decode of this buffer
        psuMsg->psuAttributes->ulBitPosition = 0;

        return (DecodeMinorFrame_AnalogF1(psuMsg));
    }

    return I106_OK;
}


/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106_Decode_NextAnalogF1(SuAnalogF1_CurrMsg * psuMsg)
{

    if(psuMsg->psuChanSpec->bThruMode)
    {
        return (DecodeMinorFrame_AnalogF1(psuMsg));
    }

    // Check for no (more) data
    if (psuMsg->ulDataLen < psuMsg->uBytesRead)
        return I106_NO_MORE_DATA;
    
    // If not thru mode, we must have an intrapacket header
    // NOTE: UNTESTED
    // May be, it points to outside ...
    psuMsg->psuIntraPktHdr = (SuAnalogF1_IntraPktHeader *) ((char *)(psuMsg->psuChanSpec) + psuMsg->uBytesRead);
    psuMsg->uBytesRead += sizeof(SuAnalogF1_IntraPktHeader);
    // ... so check, if it was successful
    if (psuMsg->ulDataLen <= psuMsg->uBytesRead)
        return I106_NO_MORE_DATA;
            
    // TODO: Check time stamp, alignment, compute the the sub packet len etc
            
    // Fetch the time from the intra packet header
    vFillInTimeStruct(psuMsg->psuHeader, (SuIntraPacketTS *)psuMsg->psuIntraPktHdr, &psuMsg->suTimeRef);
    // and publish it   
    psuMsg->llIntPktTime = psuMsg->suTimeRef.uRelTime;

  // Check for no more data (including the length of the minor frame)
  if(psuMsg->ulDataLen < psuMsg->uBytesRead + psuMsg->ulSubPacketLen)
      return I106_NO_MORE_DATA;
        
  // Set the pointer to the Analog message data
  psuMsg->pauData = (uint8_t *)((char *)(psuMsg->psuChanSpec) + psuMsg->uBytesRead);

  psuMsg->uBytesRead += psuMsg->ulSubPacketLen;

  return I106_OK;

}

/* ----------------------------------------------------------------------- */

// Fill the attributes from TMATS 
// ToDo: Check if all needed definitions found
EnI106Status I106_CALL_DECL Set_Attributes_AnalogF1(SuRDataSource * psuRDataSrc, SuAnalogF1_Attributes * psuAnalogF1_Attributes)
{

    SuRRecord           * psuRRecord;
    uint32_t            uBitCount;

    if(psuAnalogF1_Attributes == NULL) return I106_INVALID_PARAMETER; // Set Attributes

    memset(psuAnalogF1_Attributes, 0, sizeof(SuAnalogF1_Attributes));
    psuRRecord = psuRDataSrc->psuRRecord;

    if(psuRRecord == NULL) return I106_INVALID_PARAMETER; // Set Attributes

    // Collect the TMATS values
    // ------------------------
    //What values do we need to process analog data, Spence?
    //      uint32_t    uMode           :  2;      // 
    //      uint32_t    uLength         :  6;      // Bits in A/D value
    //      uint32_t    uSubChan        :  8;      // Subchannel number
    //      uint32_t    uTotChan        :  8;      // Total number of subchannels
    //      uint32_t    uFactor         :  4;      // Sample rate exponent
    //      uint32_t    bSame           :  1;      // One/multiple Channel Specific
    

    psuAnalogF1_Attributes->psuRDataSrc                = psuRDataSrc; // May be, we need it in the future

    psuAnalogF1_Attributes->iRecordNum                 = psuRRecord->iRecordNum; // R-x

    if(psuRDataSrc->szNumDataSources != NULL)
      
    //Get number of chans per packet
    if(psuRRDataSrc->szAnalogChansPerPkt != NULL)
      psuAnalogF1_Attributes->iAnalogChansPerPkt = atoi(psuRRecord->szAnalogChansPerPkt);

    //Get sample rate
    if(psuRDataSrc->szAnalogSampleRate != NULL)
       psuAnalogF1_Attributes->ullAnalogSampleRate = strtoull(psuRDataSrc->szAnalogSampleRate, NULL, 10);

    //Get size of a data sample on this channel
    if(psuRDataSrc->szAnalogDataLength != NULL)
       psuAnalogF1_Attributes->ulAnalogDataLength = strtoul(psuRDataSrc->szAnalogDataLength, NULL, 10);

    //Get Recorder Input Impedance
    if(psuRDataSrc->szAnalogRecImpedance != NULL)
       psuAnalogF1_Attributes->ulAnalogRecImpedance = strtoul(psuRDataSrc->szAnalogRecImpedance, NULL, 10);

    //Get Channel Gain
    if(psuRDataSrc->szAnalogChanGain != NULL)
       psuAnalogF1_Attributes->lAnalogChanGain = strtoul(psuRDataSrc->szAnalogChanGain, NULL, 10);

    //Get Full-Scale Range
    if(psuRDataSrc->szAnalogFullScaleRange != NULL)
       psuAnalogF1_Attributes->ulAnalogFullScaleRange = strtoul(psuRDataSrc->szAnalogFullScaleRange, NULL, 10);

    //Get Offset Voltage
    if(psuRDataSrc->szAnalogOffsetVoltage != NULL)
       psuAnalogF1_Attributes->lAnalogOffsetVoltage = strtoul(psuRDataSrc->szAnalogOffsetVoltage, NULL, 10);

    //Get LSB Value
    if(psuRDataSrc->szAnalogLSBValue != NULL)
       psuAnalogF1_Attributes->lAnalogLSBValue = strtoul(psuRDataSrc->szAnalogLSBValue, NULL, 10);

    //
    if(psuRDataSrc->szAnalogMeasTransferOrder != NULL)    // R-x\AMTO-n-m most significant bit "M", least significant bit "L". default: M
    {
        /*
        Measurement Transfer Order. Which bit is being transferred first is specified as – Most Significant Bit (M), 
        Least Significant Bit (L), or Default (D). The default is specified in the P-Group - (P-x\F2:M).
        D-1\MN3-1-1:M;
        */
        if(psuRDataSrc->szMeasTransferOrder[0] == 'L')
        {
            psuAnalogF1_Attributes->ulWordTransferOrder = ANALOG_LSB_FIRST;
            return(I106_UNSUPPORTED);
        }
    }
        
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

/* /\* ----------------------------------------------------------------------- *\/ */
/* // Fill the attributes from an external source */
/* // Replace the correspondent TMATS values, if the argument value is >= 0 */
/* EnI106Status I106_CALL_DECL  */
/*     Set_Attributes_Ext_AnalogF1(SuRDataSource * psuRDataSrc, SuAnalogF1_Attributes * psuAnalogF1_Attributes, */
/*     //      P-x                 P-x\D2               P-x\F1                   P-x\F2 */
/*     int32_t lRecordNum, int32_t lBitsPerSec, int32_t lCommonWordLen, int32_t lWordTransferOrder, */
/*     //       P-x\F3               P-x\F4 */
/*     int32_t lParityType, int32_t lParityTransferOrder, */
/*     //      P-x\MF\N                 P-x\MF1                     P-x\MF2            P-x\MF3 */
/*     int32_t lNumMinorFrames, int32_t lWordsInMinorFrame, int32_t lBitsInMinorFrame, int32_t lMinorFrameSyncType, */
/*     //      P-x\MF4                        P-x\MF5                      P-x\SYNC1  */
/*     int32_t lMinorFrameSyncPatLen, int64_t llMinorFrameSyncPat, int32_t lMinSyncs,  */
/*     //      External                      External */
/*     int64_t llMinorFrameSyncMask, int32_t lNoByteSwap) */
/* { */
/*     uint32_t BitCount; */
/*     if(psuRDataSrc == NULL) return I106_INVALID_PARAMETER; // Set Attributes Ext */

/*     if(psuAnalogF1_Attributes == NULL) return I106_INVALID_PARAMETER; // Set Attributes Ext */

/*     // Transfer the external data */
/*     if(lRecordNum != -1) */
/*         psuAnalogF1_Attributes->iRecordNum = lRecordNum; */
/*     if(lBitsPerSec != -1) */
/*         psuAnalogF1_Attributes->ulBitsPerSec = lBitsPerSec; */
/*     if(lCommonWordLen != -1) */
/*         psuAnalogF1_Attributes->ulCommonWordLen = lCommonWordLen; */
/*     if(lWordTransferOrder != -1) */
/*         psuAnalogF1_Attributes->ulWordTransferOrder = lWordTransferOrder; */
/*     if(lParityType != -1) */
/*         psuAnalogF1_Attributes->ulParityType = lParityType; */
/*     if(lParityTransferOrder != -1) */
/*         psuAnalogF1_Attributes->ulParityTransferOrder = lParityTransferOrder; */
/*     if(lNumMinorFrames != -1) */
/*         psuAnalogF1_Attributes->ulNumMinorFrames = lNumMinorFrames; */
/*     if(lWordsInMinorFrame != -1) */
/*         psuAnalogF1_Attributes->ulWordsInMinorFrame = lWordsInMinorFrame; */
/*     if(lBitsInMinorFrame != -1) */
/*         psuAnalogF1_Attributes->ulBitsInMinorFrame = lBitsInMinorFrame; */
/*     if(lMinorFrameSyncType != -1) */
/*         psuAnalogF1_Attributes->ulMinorFrameSyncType = lMinorFrameSyncType; */
/*     if(lMinorFrameSyncPatLen != -1) */
/*         psuAnalogF1_Attributes->ulMinorFrameSyncPatLen = lMinorFrameSyncPatLen; */
/*     if(llMinorFrameSyncPat != -1) */
/*         psuAnalogF1_Attributes->ullMinorFrameSyncPat = llMinorFrameSyncPat; */
/*     if(llMinorFrameSyncMask != -1) */
/*         psuAnalogF1_Attributes->ullMinorFrameSyncMask = llMinorFrameSyncMask; */
/*     if(lMinSyncs != -1) */
/*         psuAnalogF1_Attributes->ulMinSyncs = lMinSyncs; */
/*     if(lNoByteSwap != -1) */
/*         psuAnalogF1_Attributes->bDontSwapRawData = lNoByteSwap; */

/*     psuAnalogF1_Attributes->ullCommonWordMask = 0; */
/*     for(BitCount = 0; BitCount < psuAnalogF1_Attributes->ulCommonWordLen; BitCount++) */
/*     { */
/*          psuAnalogF1_Attributes->ullCommonWordMask <<= 1; */
/*          psuAnalogF1_Attributes->ullCommonWordMask |= 1; */
/*     } */

/*     psuAnalogF1_Attributes->ullCommonWordMask &= psuAnalogF1_Attributes->ullMinorFrameSyncMask; */

/*     psuAnalogF1_Attributes->dDelta100NanoSeconds = d100NANOSECONDS / psuAnalogF1_Attributes->ulBitsPerSec; */

/*     psuAnalogF1_Attributes->bPrepareNextDecodingRun = 1; // Set_Attributes_Ext_AnalogF1 */

/*   return I106_OK; */

/* } // End Set_Attributes_Ext_ AnalogF1 */

/* ----------------------------------------------------------------------- */

// Create the output buffers for a minor frame (data and error flags)
EnI106Status I106_CALL_DECL 
    CreateOutputBuffers_AnalogF1(SuAnalogF1_Attributes * psuAttributes)
{

    // Allocate the Analog output buffer for a minor frame
    psuAttributes->ulOutBufSize = psuAttributes->ulWordsInMinorFrame;
    psuAttributes->paullOutBuf = (uint64_t *)calloc(sizeof(uint64_t), psuAttributes->ulOutBufSize);
    if(psuAttributes->paullOutBuf == NULL)
        return I106_BUFFER_TOO_SMALL;
    
    psuAttributes->pauOutBufErr = (uint8_t *)calloc(sizeof(uint8_t), psuAttributes->ulOutBufSize);
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
EnI106Status PrepareNextDecodingRun_AnalogF1(SuAnalogF1_CurrMsg * psuMsg)
{
    SuAnalogF1_Attributes * psuAttributes = psuMsg->psuAttributes;

    EnI106Status enStatus = CreateOutputBuffers_AnalogF1(psuAttributes);
    if(enStatus != I106_OK)
        return(enStatus);

    psuAttributes->bPrepareNextDecodingRun = 0;
    
    // If not throughput mode, the work is done 
    // ----------------------------------------
    if( ! psuMsg->psuChanSpec->bThruMode)
        return I106_OK;

    // Prepare the variables for bit decoding in throughput mode
    // --------------------------------------------------------
    psuAttributes->ullSyncCount = -1; // -1 sets all bits to 1
    psuAttributes->ullSyncErrors = 0;
    psuAttributes->ullTestWord = 0; 
    psuAttributes->ulBitPosition = 0; 
    psuAttributes->ullBitsLoaded = 0;
    // Nearly the same as in RenewSyncCounter...
    psuAttributes->ulMinorFrameBitCount = 0;
    psuAttributes->ulMinorFrameWordCount = 0;
    psuAttributes->ulDataWordBitCount = 0;
    psuAttributes->lSaveData = 0;

    return I106_OK;

} // End PrepareNextDecodingRun

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

