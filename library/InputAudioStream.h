/*############################################################################*/
/*                                                                           #*/
/*       RTMMM Class Library    CDInputAudioStream Class v1.00  9/November/1996         #*/
/*         FISA comps.      F.Sartori      OS/2                              #*/
/*                                                                           #*/
/*############################################################################*/


#ifndef INPUT_AUDIO_STREAM_H
#define INPUT_AUDIO_STREAM_H

#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#include <os2.h>

#include "rtmmm.h"
#include "GenDefs.h"
#include "AudioStream.h"



/**************************************************************************************************/

static const unsigned int SECTOR_LONG_SIZE            = 588;
static const unsigned int RESERVE_SECTOR_SIZE         = 384; // Number of sectors to cache
static const unsigned int RESERVE_WARNING_SECTOR_SIZE = 128; // Number of sectors used up before panicking
static const unsigned int RESERVE_PREFILL_SECTOR_SIZE = 32;  // Number of sectors still to load before starting sound
static const unsigned int RESERVE_STEP_SECTOR_SIZE    = 16;  // Number of sectors to be read in one shot
static const unsigned int RESERVE_LONG_SIZE           = (SECTOR_LONG_SIZE * RESERVE_SECTOR_SIZE);

class CDInputAudioStream:public InputAudioStream
{
private:
    char                    drive_letter;
    HFILE                   hCD;
    APIRET                  error_status;          // Last error
    unsigned long           track_number;          // How many tracks ?
    unsigned long           first_track;           // Is it not always 1 ?
    unsigned long           last_track;            // Last playable track
    InputAudioStreamTrackInfo_type *tracks;        // a vector containing the list of songs informations
//  Support for Read & Seek
    unsigned long           reserve_offset;        // the current read pos in the reserve as offset in the subbuffer
    unsigned long           reserve_index;         // the current read pos in the reserve as index of the subbuffer
    unsigned long           reserve_size;          // the amount of data in reserve as long
    // thread point of view
    unsigned long           reserve_next_pos;      // the next upgrading pos for the caching_thread
    unsigned long           reserve_next_sector;   // the next sector to be read
    unsigned long           reserve_free_sectors;  // the amount of free sectors in reserve, free = been read
    unsigned long           reserve[RESERVE_LONG_SIZE]; // reserve of data to be used by read
    unsigned long           *reservep[RESERVE_SECTOR_SIZE]; // pointers of sectors within the reserve
    OutputAudioStream       *audio_stream;         // if NULL use CD DA
// Support for Caching thread
    TID                     reserve_thread;
    HEV                     reservesem;            // wait here when done
    HMTX                    muxsem;                // do not touch the data for the task while it is playing with it.
//  Support for Output thread
    unsigned long           sector;                // the last read sector from reserve
    TID                     thread;                // the playing thread
    unsigned long           last_playing_sector;   // the limit for the player
    ULONG                   new_last_playing_sector; // the new limit
    HEV                     commandsem;            // wait here for commands
    HEV                     answersem;             // wait here for answer
    HEV                     finishsem;             // post it when finished
    int                     exit_thread;           // 1 to keep playimg thread living ..
    int                     exit_thread2;          // 1 to keep reading thread living ..
    int                     stop_reading;          // 1 to force the read to abort the current read..
    // THREADS
    void                    CDInputAudioStreamThread(void *args);
    void                    ReserveThread(void *args);

public:
                            CDInputAudioStream (char letter);
                            ~CDInputAudioStream (void);

    InputAudioStreamTrackInfo_type  *GetTrackInfo(int track); // return song selection information
    void                    Play(int track_start, int track_end); // position is in song number (track)
    void                    SPlay(int sector_start, int sector_end); // position is in sector
    void                    Stop(void);            // Ask the player to stop
    void                    Wait(int delay);       // Wait for the end of the current program
    void                    Connect(OutputAudioStream *stream); // audio_stream must be a valid object, if not standard CD DA is used
    void                    Seek(ULONG pos);       // position is in sectors
    void                    Read(ULONG *buffer, ULONG &long_size); // long_size is size as 32 bit
    ULONG                   GetError(void){return error_status;}; // Get last error
    void                    SetSourceName(char *s){};// the name of the source stream
    void                    SetSongName(char *s){};// the name of the song being played
// CDInputAudioStream specific functions
    ULONG                   GetStatus(void);       // Get Information about Drive
    void                    AcceptDisk(void);      // Get disk information accepting it if it is an AUDIO CD
    void                    ShowInfo(void);        // Prints track information on stdout

};


/**************************************************************************************************/


class FileInputAudioStream:public InputAudioStream
{
private:
    FILE                    *file;                 // the file
    char                    songname[80];          // the song name
    OutputAudioStream       *audio_stream;         // if NULL shut up!!
    ULONG                   file_type;             // RAW,WAV etc
    APIRET                  error_status;          // Last error
    void                    *header;               // a copy of the header
    InputAudioStreamTrackInfo_type *tracks;        // a vector containing the list of songs informations
    ULONG                   actual_start;          // the position in the file for a good start
    ULONG                   actual_position;       // the current position in the file
    ULONG                   actual_size;           // the file size in bytes - header size
    ULONG                   sector;                // the current sector
    ULONG                   baudrate;              // freq * chan * size
    TID                     thread;                // the playing thread
    HEV                     commandsem;            // wait here for commands
    HEV                     answersem;             // wait here for answer
    HEV                     finishsem;             // post it when finished
    int                     exit_thread;           // 1 to keep playimg thread living ..
    ULONG                   last_playing_sector;   // the limit for the player
    ULONG                   new_last_playing_sector; // the new limit
// private functions
    ULONG                   Autodetect(void);
    void                    FileInputAudioStreamThread(void *args);

public:
                            FileInputAudioStream (char  *filename,
                                                  int   file_format,
                                                  ULONG number_of_channels, // 1,2..
                                                  ULONG sampling_frequency, // 44100
                                                  ULONG samplesize);        // 1byte 2bytes ....
                            ~FileInputAudioStream (void);

    InputAudioStreamTrackInfo_type  *GetTrackInfo(int track); // return song selection information
    void                    Play(int track_start, int track_end); // position is in song number (track)
    void                    SPlay(int sector_start, int sector_end); // position is in sector
    void                    Stop(void);            // Ask the player to stop
    void                    Wait(int delay);       // Wait for the end of the current program
    void                    Connect(OutputAudioStream *stream); // audio_stream must be a valid object, if not standard CD DA is used
    void                    Seek(ULONG pos);       // position is in sectors
    void                    Read(ULONG *buffer, ULONG &long_size); // long_size is size as 32 bit
    ULONG                   GetError(void){return error_status;}; // Get last error
    void                    SetSourceName(char *s){};// the name of the source stream
    void                    SetSongName(char *s){};// the name of the song being played

    void                    ShowInfo(void);        // Prints track information on stdout

};


#endif