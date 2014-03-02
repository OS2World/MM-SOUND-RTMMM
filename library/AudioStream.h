#if !defined (AUDIO_STREAM_H)
#define AUDIO_STREAM_H

#include "GenDefs.h"

typedef struct {
    ULONG   start;                                 // in samples over the whole buffer for CD a sector is made pf 588 samples
    ULONG   stop;                                  // in samples over the whole buffer
    ULONG   time75;                                // length in 75th of a second it comes from the CD definition
    ULONG   start_sector;                          // samples / 588 (it simulates the CD structure)
    ULONG   stop_sector;                           // samples / 588
    ULONG   byte_size;                             // start - stop * #channels * # of bytes per sample
    ULONG   frequency;                             // 44100 for a CD
    ULONG   sample_size;                           // 2 for a CD
    ULONG   channels;                              // 2 for a CD
} InputAudioStreamTrackInfo_type;

/* CONCEPTS

A track is a complete song
A sector is 588 32 bit words and it can either be 588 samples in case of 16 bit sterep or more if ...
Time 75 is a format of data where three bytes are used thus minutes;seconds;75ths of second
A single WAV file will report 1 song only : 1 track
*/



// Abstract base class for audio streams classes:
class AudioStream
{
public:
    virtual     ~ AudioStream (void) {}            // std destroyer
    virtual ULONG GetError(void) = 0;              // returns the last error code
    virtual void  SetSourceName(char *s) = 0;      // the name of the source stream  i.e : MAPLAY
    virtual void  SetSongName(char *s) = 0;        // the name of the song being played
};

// Abstract base class for audio streams classes:
class OutputAudioStream : public AudioStream
{
public:
    virtual void append (uint32 channel,
                         int16 value) = 0;         // this function takes a 16 Bit PCM sample
    virtual void write_buffer (void) = 0;          // This function tells the system to write the buffers accumulated, it is superfluous if the output is real-time
    virtual int16 *get_buffer(uint32 *samples_number) = 0; // return a buffer where to write to.
    virtual void return_buffer(void) = 0;          // release the buffer to be played
    virtual void set_stream_type(ULONG channels,
                         ULONG sampling_rate) = 0; // change parameters on the fly
    virtual void set_stream_volume(ULONG percent) = 0; // set volume in percent
};

class InputAudioStream : public AudioStream
{
public:
    virtual InputAudioStreamTrackInfo_type  *GetTrackInfo(int track) = 0; // return song selection information
    virtual void Play(int track_start,
                      int track_end) = 0;          // position is in song number (track)
    virtual void SPlay(int sector_start,
                       int sector_end) = 0;        // position is in sector
    virtual void Stop(void) = 0;                   // Ask the player to stop
    virtual void Wait(int delay) = 0;              // Wait for the end of the current program
    virtual void Connect(OutputAudioStream *stream) = 0; // audio_stream must be a valid object, if not standard CD DA is used
    virtual void Seek(ULONG pos) = 0;              // position is in sectors
    virtual void Read(ULONG *buffer,
                      ULONG &long_size) = 0;       // long_size is size as 32 bit
};

class PipeAudioStream : public OutputAudioStream
{
public:
//  Output end
    virtual void append (uint32 channel, int16 value) = 0; // this function takes a 16 Bit PCM sample
    virtual void write_buffer (void) = 0;          // This function tells the system to write the buffers accumulated, it is superfluous if the output is real-time
    virtual int16 *get_buffer(uint32 *samples_number) = 0; // return a buffer where to write to.
    virtual void return_buffer(void) = 0;          // release the buffer to be played
//  Input End
    virtual void Connect(OutputAudioStream *stream) = 0; // audio_stream must be a valid object, if not standard CD DA is used
    virtual void Read(ULONG *buffer, ULONG &long_size) = 0; // long_size is size as 32 bit
};


#endif