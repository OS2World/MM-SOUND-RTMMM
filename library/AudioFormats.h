/*############################################################################*/
/*                                                                           #*/
/*       RTMMM Class Library    AudioFormats definitions v 1.0               #*/
/*         FISA comps.      F.Sartori      OS/2       November 1996          #*/
/*                                                                           #*/
/*############################################################################*/

#if !defined(_AUDIO_FORMAT_H)
#define _AUDIO_FORMAT_H

static const unsigned int  AUDIO_RAW  = 0;
static const unsigned int  AUDIO_RIFF = 1;
static const unsigned int  AUDIO_AIFF = 2;
static const unsigned int  AUDIO_ULAW = 3;
static const unsigned int  AUDIO_AUTODETECT = 99;  // valid only for Input streams

// More to be added.....

#pragma pack (1)

typedef union
{
    struct
    {
        char            RIFF[4];                   // Must contain "RIFF"
        unsigned long   TotSize;                   // Size + Header - 4 (bytes)
        char            WAVEfmt[8];                // Must contain "WAVEfmt"
        unsigned long   UnKnown1;                  // Must contain 0x10 = 16
        unsigned short  UnKnown2;                  // Must contain 1
        unsigned short  Channels;                  // 1 = mono 2 = stereo
        unsigned long   Frequency;                 // 44100 for 44100 Khz
        unsigned long   ByteRate;                  // Frequency * #channels * #bytes/sample
        unsigned short  SampleSize;                // 1 = mono8 2 = stereo8 or mono16 4 = stereo16
        unsigned short  BitSize;                   // 8 = 8 bit 16 = 16 bit
        char            data[4];                   // Must contain "data"
        unsigned long   Size;                      // Size of raw data in bytes
    } item;
    char string[44];
} AudioRIFFheader_type;

#define RIFF_HEADER_STRING "RIFF    WAVEfmt                     data" // To ease programming
#define RIFF_HEADER_SIZE   44

#pragma pack (4)

#endif