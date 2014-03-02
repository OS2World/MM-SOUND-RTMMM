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

#include "OutputAudioStream.h"

char              *filename="DATA.raw";
OutputAudioStream *stream  = NULL;


static void Exit (int returncode)
  // delete some objects and exit
{
  DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,0,0);
//  DosSleep(10000);
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
    unsigned int     size,bigsize;
    unsigned int     arg = 1;
    int              i,j,k,err;  
    float            freq;

#if _EMX_
    struct sigaction   sa;

    sa.sa_handler = breakfunc;
    sa.sa_flags   = 0;
    sigemptyset(&sa.sa_mask);
    for (i=SIGHUP;i<=SIGTERM;i++)  sigaction(i,&sa,NULL);
#endif

    stream = new RTMMMOutputAudioStream(2,0);
    if (stream->GetError() !=0 ){
        delete stream;
#if defined(__USE_DART__)
        stream = new DARTOutputAudioStream(2,0);
#endif
        if (stream->GetError() !=0 ){
            cerr << "Cannot allocate neither RTMMM or DART \n";
        }
    }
    stream->SetSongName("CD-PLAYER");

//    stream = new FileOutputAudioStream(2,filename);
//    stream = new NULLOutputAudioStream(2);
    buffer = stream->get_buffer(&size);
    stream->return_buffer();
    bigsize = size *16;   
 
    mybuff = (int16 *)malloc(bigsize * 2); 

    freq = 6.283 / bigsize;
    freq *= 20.0;
    for (i=0; i<bigsize;i+=2) {
        mybuff[i]   = (int16)(30000.0 *sin(freq * i * 32.0));
        mybuff[i+1] = (int16)(30000.0 *cos(freq * i * 33.0));
    }

    DosSleep(100);

    DosSetPriority(PRTYS_THREAD,PRTYC_TIMECRITICAL,RTMMM_MIXER_PRI-4,0);
    k = 0;
    while(1){
        for (j = 0;j<16;j++){
cerr << "BUFFER" <<j <<"\n";
            buffer = stream->get_buffer(&size);
cerr << buffer <<"\n";
            for (i=0;i<size;i++) buffer[i] = mybuff[i+size*j];
            stream->return_buffer();
            stream->write_buffer();
            stream->set_stream_volume((ULONG) (50.0 + 50.0 * sin(k++ * 0.1)));
        }

    }
   
    Exit(0);
}
