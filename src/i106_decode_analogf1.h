/****************************************************************************

 i106_decode_analogf1.h - 

 Copyright (c) 2008 Irig106.org

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

 Created by Bob Baggerman
 Brought to life by Spencer Hatch in Andøya, Norge, NOV 2014

 ****************************************************************************/

#ifndef _I106_DECODE_ANALOGF1_H
#define _I106_DECODE_ANALOGF1_H

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif


/*
 * Macros and definitions
 * ----------------------
 */


typedef enum
{
    ANALOG_PACKED                 = 0,
    ANALOG_UNPACKED_LSB_PADDED    = 1,
    ANALOG_RESERVED               = 2,
    ANALOG_UNPACKED_MSB_PADDED    = 3,
} ANALOG_MODE;

typedef enum 
{
    ANALOG_MSB_FIRST       = 0,
    ANALOG_LSB_FIRST       = 1,
} ANALOG_BIT_TRANSFER_ORDER;

/*
 * Data structures
 * ---------------
 */

#if defined(_MSC_VER)
#pragma pack(push,1)
#endif

// Channel specific data word
// --------------------------

typedef PUBLIC struct AnalogF1_ChanSpec_S
    {
    uint32_t    uMode           :  2;      // 
    uint32_t    uLength         :  6;      // Bits in A/D value
    uint32_t    uSubChan        :  8;      // Subchannel number
    uint32_t    uTotChan        :  8;      // Total number of subchannels
    uint32_t    uFactor         :  4;      // Sample rate exponent
    uint32_t    bSame           :  1;      // One/multiple Channel Specific
    uint32_t    iReserved       :  3;      //
#if !defined(__GNUC__)
    } SuAnalogF1_ChanSpec;
#else
    } __attribute__ ((packed)) SuAnalogF1_ChanSpec;
#endif

#if defined(_MSC_VER)
#pragma pack(pop)
#endif

// Channel attributes
// Note:
// The SuAnalogF1_Attributes structure covers most of the information needed to decode raw Analog data. 
// Only a part of the relevant data is supplied in the message SuAnalogF1_ChanSpec.
// Most of the attributes must be imported from TMATS or supplied by another source.
typedef struct AnalogF1_Attributes_S
    {
    SuRDataSource * psuRDataSrc;            // Pointer to the corresponding RDataSource
    int             iRecordNum;             // R-x

    char          * szDataSourceID;         //

    int             iTrackNumber;           // Only valid if szTrackNumber != NULL
    int             iPhysicalChanNumber;
    int             bEnabled;               // Only valid if szEnabled != NULL
    char          * szBusDataLinkName;      // R-x\BDLN-n (-04, -05)
    char          * szChanDataLinkName;     // R-x\CDLN-n (-07, -09)
    int             iAnalogDataTypeFormat;  // (R-x\ATF-n)
    int             iAnalogChansPerPkt;     // (R-1\ACH\N-n) //ORIG
    uint64_t        ullAnalogSampleRate;    // (R-1\ASR-n)   //ORIG

    uint32_t        bAnalogIsDataPacked;    // (R-1\ADP-n)   //ORIG
    char          * szAnalogMeasurementNam; // (R-x\AMN-n-m)
    uint32_t        ulAnalogDataLength;     // (R-x\ADL-n-m)
    char          * szAnalogBitMask;        // (R-x\AMSK-n-m)
      //    char          * szAnalogMeasTransfOrd;  // 
    uint32_t        ulAnalogMeasTransfOrd;  // Msb (0)/ LSB (1, unsupported) R-x\AMTO-n-m
    uint32_t        ulAnalogSampleFactor;   // (R-x\ASF-n-m)
    uint64_t        ullAnalogSampleFilter;  // (R-x\ASBW-n-m)
    uint32_t        bAnalogIsDCCoupled;     // D (0) / A (1)  R-x\ACP-n-m
    uint32_t        ulAnalogRecImpedance;   // (R-x\AII-n-m)
    int32_t         lAnalogChanGain;        // (R-x\AGI-n-m)
    uint32_t        ulAnalogFullScaleRange; // (R-x\AFSI-n-m)
    int32_t         lAnalogOffsetVoltage;   // (R-x\AOVI-n-m)
    int32_t         lAnalogLSBValue;        // (R-x\ALSV-n-m)
    /* char          * szAnalogEUCSlope;       // (R-x\AECS-n-m) */
    /* char          * szAnalogEUCOffset;      // (R-x\AECO-n-m) */
    /* char          * szAnalogEUCUnits;       // (R-x\AECU-n-m) */
    /* char          * szAnalogFormat;         // (R-x\AF-n-m) */
    /* char          * szAnalogInputType;      // (R-x\AIT-n-m) */
    uint32_t        bAnalogIsAudio;         // (R-x\AV-n-m)
    uint32_t        ulAnalogAudioFormat;    // (R-x\AVF-n-m)      
      
    // Stuff from SuRRecord
    uint32_t        ulNumDataSources;       // R-x\N
    uint32_t        bIndexEnabled;          // R-x\ID
    uint32_t        bEventsEnabled;         //R-x\EV\E

    // Computed values 

    int32_t     bPrepareNextDecodingRun;            // First bit flag for a complete decoding run: preload a minor frame sync word to the test word

    // The output buffer must be allocated if bPrepareNextDecodingRun is notzero
    // The buffer consists of two parts: A data buffer and an error buffer
    int32_t     ulOutBufSize;               // Size of the output buffer in (64-bit) words
    uint64_t    * paullOutBuf;              // Contains the decoded data of a minor frame
    uint8_t     * pauOutBufErr;             // Contains the error flags (parity error) for each data word in a minor frame

    // Variables for bit decoding
    // Must be kept for the whole decoding run because the data 
    // may overlap the CH10 packets (at least in troughput mode)

    uint64_t    ullSyncCount;               // -1: Nothing found, 0: 1 sync found etc. analog to Min Syncs
    uint64_t    ullSyncErrors;              // Counter for statistics 
    uint64_t    ullTestWord;                // Currently collected word resp. syncword
    uint64_t    ullBitsLoaded;              // Bits already loaded (and shifted through) the TestWord. 
    // The amount must be at least the sync word len to check for a sync word
    uint32_t    ulBitPosition;              // Bit position in the current buffer

    uint32_t    ulDataWordBitCount;         // Counter for the bits of a data word
    int32_t     lSaveData;                  // Save the data (0: do nothing, 1 save, 2: save terminated)


#if !defined(__GNUC__)
    } SuAnalogF1_Attributes;
#else
    } __attribute__ ((packed)) SuAnalogF1_Attributes;
#endif


// Current Analog message (I don't remember when/if I ripped this off PCM stuff)
typedef struct
    {
        SuI106Ch10Header       * psuHeader;        // The overall packet header
        SuAnalogF1_ChanSpec    * psuChanSpec;      // Header in the data stream
        SuAnalogF1_Attributes  * psuAttributes;    // Pointer to the Pcm Format structure, values must be imported from TMATS 
                                                   // or another source
      //        SuAnalogF1_IntraPktHeader * psuIntraPktHdr;// Optional intra packet header, consists of the time 
        // suIntraPckTime (like SuIntraPacketTS) and the header itself
        unsigned int        uBytesRead;            // Number of bytes read in this message
        uint32_t            ulDataLen;             // Overall data packet length
      //        int64_t             llIntPktTime;          // Intrapacket or header time ! Relative Time !
      //        int64_t             llBaseIntPktTime;      // Intrapacket or header time ! Relative Time !
      //        uint32_t            ulSubPacketLen;        // MinorFrameLen in Bytes padded, see bAlignment. 
        // In throughput mode it's the length of the whole packet
      //        uint32_t            ulSubPacketBits;       // MinorFrameLen in Bits
        uint8_t             * pauData;             // Pointer to the start of the data
        SuTimeRef           suTimeRef;

#if !defined(__GNUC__)
    } SuAnalogF1_CurrMsg;
#else
    } __attribute__ ((packed)) SuAnalogF1_CurrMsg;
#endif


/*
 * Function Declaration
 * --------------------
 */


EnI106Status I106_CALL_DECL 
    enI106_Decode_FirstAnalogF1(SuI106Ch10Header     * psuHeader,
                                  void            * pvBuff,
                                  SuAnalogF1_CurrMsg * psuMsg);

EnI106Status I106_CALL_DECL 
    enI106_Decode_NextAnalogF1(SuAnalogF1_CurrMsg * psuMsg);

EnI106Status I106_CALL_DECL 
    Set_Attributes_AnalogF1(SuRDataSource * psuRDataSrc, SuAnalogF1_Attributes * psuAttributes);

EnI106Status I106_CALL_DECL 
    CreateOutputBuffers_AnalogF1(SuAnalogF1_Attributes * psuAttributes);

EnI106Status  I106_CALL_DECL
    FreeOutputBuffers_AnalogF1(SuAnalogF1_Attributes * psuPcmAttributes);

// Help functions



#ifdef __cplusplus
} // end namespace
} // end extern c  
#endif

#endif // _I106_DECODE_ANALOGF1_H
