#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream.h>
#include <iomanip.h>
#include <signal.h>

#define  INCL_DOS
#define  INCL_OS2MM
#include <os2.h>
#include <os2me.h>

#include "AudioFormats.h"
#include "OutputAudioStream.h"
#include "InputAudioStream.h"
 
char                 *filename="DATA.raw";
char                 *wav_filename="DATA.wav";
FileInputAudioStream *audiofile = NULL;
OutputAudioStream    *stream  = NULL;

typedef struct {
   unsigned int start_track;
   unsigned int stop_track;
   unsigned int start_sector;
   unsigned int stop_sector;
} command_type;

static void Exit (int returncode)
  // delete some objects and exit
{
  DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,0,0);
  delete audiofile;        // delete on NULL-pointers are harmless
  delete stream;
  exit (returncode);
}

void breakfunc(int signo){
    Exit(99);
}


int main(int argc,char **argv)
{
    int16            *mybuff;
    int16            *buffer;
    unsigned int     size;
    unsigned int     arg = 1;
    int              i,err;  
    command_type     commands[1024];
    int              command_no = 0;
    InputAudioStreamTrackInfo_type *track;
#if _EMX_
    struct sigaction   sa;

    sa.sa_handler = breakfunc;
    sa.sa_flags   = 0;
    sigemptyset(&sa.sa_mask);
    for (i=SIGHUP;i<=SIGTERM;i++)  sigaction(i,&sa,NULL);
#endif

cerr <<"Ciao\n";

    if (argc < 2 || !strncmp (argv[1], "-h", 2))
    {
    usage:
        cerr << "\nusage: " << argv[0]
            << " Filename output-option  selector \n"
               "  --- OUTPUT OPTIONS ---\n"
                "  -F         write RAW samples to file CDDATA.raw\n"
                "  -W         write WAV samples to file CDDATA.wav\n"
                "  -R         use RTMMM\n"
#if defined(__USE_DART__)
                "  -D         use DART\n"
#endif
                "  -N         use NULL device\n"
                "  -C         use CD own DA\n";
        return 0;   
    }

    stream = NULL;

    audiofile = new FileInputAudioStream(argv[1],AUDIO_RIFF,2,44100,2);
    if ((err = audiofile->GetError()) !=0) { cerr << "Error Init CD err #"<<err<<"\n";Exit(-1); }
    track = audiofile->GetTrackInfo(1); 

    // parse arguments:
    for (i = 2; i < argc; ++i){
        if (argv[i][0] == '-' && argv[i][1])
            switch ((int)argv[i][1])
            {
                case 'F':
                    delete stream;
                    stream = NULL;
                    stream = new FileOutputAudioStream(track->channels,track->frequency,filename,AUDIO_RAW);
                break;
                case 'W':
                    delete stream;
                    stream = NULL;
                    stream = new FileOutputAudioStream(track->channels,track->frequency,wav_filename,AUDIO_RIFF);
                break;
                case 'R':
                case 'D':
                    delete stream;
                    stream = NULL;
                    stream = new RTMMMOutputAudioStream(track->channels,track->frequency);
                    if (stream->GetError() !=0 ){
                        delete stream;
#if defined(__USE_DART__)
                        stream = new DARTOutputAudioStream(track->channels,track->frequency);
#endif
                        if (stream->GetError() !=0 ){
                            cerr << "Cannot allocate neither RTMMM or DART \n";
                        }
                    }
                    stream->SetSongName("CD-PLAYER");
                break;
                case 'N':
                    delete stream;
                    stream = NULL;
                    stream = new NULLOutputAudioStream(track->channels);
                break;
                case 'C':
                    delete stream;
                    stream = NULL;
                break;
                default:
                    goto usage;
            }
        }


    audiofile->ShowInfo();
    if ((err = audiofile->GetError()) !=0) { cerr << "Error NOT Audio CD err #"<<err<<" \n";Exit(-1); }

    audiofile->Connect(stream);
    if ((err = audiofile->GetError()) !=0) { cerr << "Error Getting output Stream ??? err #"<<err<<"\n";Exit(-1); }

cerr << "PLAY\n";
    audiofile->Play(1,1);
    if ((err = audiofile->GetError()) !=0) { cerr << "Error Play CD err #"<<err<<"\n";Exit(-1); }

cerr << "WAIT\n";
    audiofile->Wait(-1);
    if ((err = audiofile->GetError()) !=0) { cerr << "Error Wait CD err #"<<err<<"\n";Exit(-1); }  

    Exit(-1);
}
