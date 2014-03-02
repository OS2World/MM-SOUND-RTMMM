/*############################################################################*/
/*                                                                           #*/
/*   RTMMM Class Library    OutputAudioStream Class v1.00     9/November/1996      #*/
/*         FISA comps.      F.Sartori      OS/2                              #*/
/*                                                                           #*/
/*############################################################################*/


#ifndef OUTPUT_AUDIO_STREAM_H
#define OUTPUT_AUDIO_STREAM_H

#if defined(__USE_DART__)
#define  INCL_OS2MM                 /* required for MCI and MMIO headers   */
#define  INCL_DOS

#include <os2.h>
#include <os2me.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "rtmmm.h"
#include "GenDefs.h"
#include "AudioStream.h"

static const uint32 MAXCHANNELS = 2;               // max. number of channels
static const uint32 NUMBER_OF_LOCAL_BUFFERS = 16;  // max 1Mbytes if DART = 64K
static const uint32 NUMBER_OF_LOCAL_BUFFERS_MASK = 0xF;


typedef struct{
    int   bufferindex;
    int16 *buffer;
    int   index;
} index_type;

class RTMMMOutputAudioStream : public OutputAudioStream
{
private:
    int                     error_status;          // 0 if everything is ok!
    // client interface
    int                     subbuffer_size;        // the size of a subbuffer = # of samples stereo + mono
    int                     subbuffer_mask;        // to allow circular addressing within subbuffer
    index_type              buffer_index[MAXCHANNELS]; // to allow r-l write on buffers
    int16                   *subbuffers[NUMBER_OF_LOCAL_BUFFERS + 2]; // points to the DART buffers
    int                     buffer_mask;           // to allow circular addressing among subbuffers
    int                     channels;              // 1 = mono 2 = stereo
    // buffers interface
    int                     next_play_buffer;      // the next buffer that must be sent to DART
    int                     buffers_counter_mask;  // the mask to rotate among buffers
    int                     ready_buffers;         // the number of ready buffers
    int                     free_buffers;          // the number of freed buffers
    HEV                     waitsem;
    // RTMMM interface
    HQUEUE                  server_queue;
    RTMMM_InputChannel_type *OutputChannel;
    RTMMM_Definition_type   *MixerInfo;
    int                     thread_id;
    int                     server_id;
    HEV                     news_semaphore;

    void  RTMMMSynchTask(void *args);

public:
          RTMMMOutputAudioStream (uint32 number_of_channels,uint32 sampling_frequency);
         ~RTMMMOutputAudioStream (void);

    ULONG GetError(void){ return error_status;};

    void  append (uint32 channel, int16 value);
    void  write_buffer (){};                       // NOT USED HERE: IT IS EMPTY
    void  SetSourceName(char *s);                  // the name of the source stream  i.e : MAPLAY
    void  SetSongName(char *s);
    int16 *get_buffer(uint32 *samples_number);     // return a buffer where to write to.
    void  return_buffer(void);                     // release the buffer to be played
    void  set_stream_type(ULONG channels,
                          ULONG sampling_rate);    // change parameters on the fly
    void set_stream_volume(ULONG percent);         // set volume in percent

};

static const uint32 LOCAL_BUFFERS_SIZE = 0x4000;   // 16384 samples (16 bit each)
static const uint32 LOCAL_BUFFERS_MASK = 0x3fff;
static const uint32 LOCAL_BUFFERS_BITS = 15;
static const uint32 frequencies[3] = { 44100, 48000, 32000 };

#if defined(__USE_DART__)

class DARTOutputAudioStream : public OutputAudioStream
{
private:
    int                error_status;               // 0 if everything is ok!
    int                subbuffer_size;             // the size of a subbuffer as number of samples it can contain
    int                subbuffer_mask;             // to allow circular addressing within subbuffer
    index_type         buffer_index[MAXCHANNELS];  // to allow r-l write on buffers
    int16              *subbuffers[NUMBER_OF_LOCAL_BUFFERS + 2]; // points to the DART buffers
    int                buffer_mask;                // to allow circular addressing among subbuffers
    int                channels;                   // 1 = mono 2 = stereo
    MCI_MIXSETUP_PARMS MixSetupParms;              // Mixer parameters
    MCI_AMP_OPEN_PARMS AmpOpenParms;               // Parameters of the opening of the device
    MCI_BUFFER_PARMS   BufferParms;                // Device buffer parms
    HEV                waitsem;                    // to reactivate the filling of the buffer when there is free space

public:
          DARTOutputAudioStream (uint32 number_of_channels,uint32 sampling_frequency);
         ~DARTOutputAudioStream (void);

    ULONG GetError(void){ return error_status;};

    void  append (uint32 channel, int16 value);
    void  write_buffer (void){};                   // NOT USED HERE: IT IS EMPTY
    void  SetSourceName(char *s){};                // the name of the source stream  i.e : MAPLAY
    void  SetSongName(char *s){};
    // if you use these two functions, the append functionis not anymore supported
    // and vice versa ..
    int16 *get_buffer(uint32 *samples_number);     // return a buffer where to write to.
    void  return_buffer(void);                     // release the buffer to be played
    void  set_stream_type(ULONG channels,
                          ULONG sampling_rate);    // change parameters on the fly
    void  set_stream_volume(ULONG percent);        // set volume in percent
};

#endif

static const uint32 OBUFFERSIZE = 2 * 1152;        // max. 2 * 1152 samples per frame

class FileOutputAudioStream : public OutputAudioStream
{
private:
    int     error_status;                          // 0 if everything is ok!
    int     audio_format;
    int16   buffer[OBUFFERSIZE];
    int16   *bufferp[MAXCHANNELS];
    uint32  channels;
    uint32  frequency;
    int     fid;                                   // the file handle

    void    pre_append_header(void);               // reserve space in the beginning for the header
    void    append_header(void);                   // fill in the header
public:
        FileOutputAudioStream (uint32 number_of_channels,// 1 = mono 2 = stereo
                         uint32 sampling_frequency,// 44100 etc
                         char *filename,
                         int file_format);         // AUDIO_RIFF AUDIO_RAW ....
       ~FileOutputAudioStream (void);
    ULONG GetError(void){ return error_status;};

    void  append (uint32 channel, int16 value);
    void  write_buffer (void);
    void  SetSourceName(char *s){};                // the name of the source stream  i.e : MAPLAY
    void  SetSongName(char *s){};
    int16 *get_buffer(uint32 *samples_number);     // return a buffer where to write to.
    void  return_buffer(void);                     // release the buffer to be played
    void  set_stream_type(ULONG channels,
                          ULONG sampling_rate);    // change parameters on the fly
    void  set_stream_volume(ULONG percent){};      // set volume in percent
};


class NULLOutputAudioStream : public OutputAudioStream
{
private:
    int    error_status;                           // 0 if everything is ok!
    int16  buffer[1024];
    ULONG  last_time;
    ULONG  counter;
    float  frequency;
public:
        NULLOutputAudioStream (uint32 number_of_channels);
       ~NULLOutputAudioStream (void) {}

    ULONG GetError(void){ return error_status;};

    void  append (uint32 channel, int16 value);
    void  write_buffer (void){};
    void  SetSourceName(char *s){};                // the name of the source stream  i.e : MAPLAY
    void  SetSongName(char *s){};
    // if you use these two functions, the append functionis not anymore supported
    // and vice versa ..
    int16 *get_buffer(uint32 *samples_number){*samples_number = 1024;return buffer;}; // return a buffer where to write to.
    void  return_buffer(void);                     // release the buffer to be played
    void  set_stream_type(ULONG channels,
                          ULONG sampling_rate){};  // change parameters on the fly
    void  set_stream_volume(ULONG percent){};      // set volume in percent
};

#endif
