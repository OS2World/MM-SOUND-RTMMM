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
#include "AudioStream.h"
#include "InputAudioStream.h"
#include "OutputAudioStream.h"
#include "PipeServer/PipeServer.h"

char                *filename="CDDATA.raw";
char                *wav_filename="CDDATA.wav";
CDInputAudioStream  *audiocd = NULL;
OutputAudioStream   *stream  = NULL;
int                 use_pipe = 0; // do not use pipe

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
  delete audiocd;        // delete on NULL-pointers are harmless
  delete stream;
  exit (returncode);
}

void breakfunc(int signo){
    Exit(99);
}

int getsingle(char *s,unsigned int *start){
     char *tok1;

     tok1 = strtok(s," ,");

     if (tok1==NULL)      return 0; // finished
     if (strlen(tok1)==0) return 0; // finished

     *start = atol(tok1);
     return 1;   // OK
}

int getdouble(char *s,unsigned int *start,unsigned int *stop){
     char *tok1,*tok2;

     tok1 = strtok(s," ,");
     if (tok1==NULL)      return 0; // finished
     if (strlen(tok1)==0) return 0; // finished

     tok2 = strtok(NULL," ,");
     if (tok2==NULL)      return 0; // finished
     if (strlen(tok2)==0) return 0; // finished

     *start = atol(tok1);
     *stop =  atol(tok2);
     return 1;   // OK
}

/* PIPE SERVER EXPORTED FUNCTIONS */

void OPEN(char *msg,char *answer,int answersize) {
    int i=0;
    printf("OPEN CD %s\n",msg);

    audiocd = new CDInputAudioStream(msg[0]); 
    if (audiocd->GetError() !=0) {
        delete audiocd;
        sprintf(answer,"-1\n");
    } else {
        audiocd->Connect(stream);
        while (audiocd->GetTrackInfo(i++) != NULL);
        sprintf(answer,"%i\n",i); 
    }
}

void INFO(char *msg,char *answer,int answersize) {
    int i,j;
    InputAudioStreamTrackInfo_type *track;
    printf("INFO %s\n",msg);

    if (audiocd == NULL) { sprintf(answer,"-1\n"); return; }

    sscanf(msg,"%i",&i);
    track = audiocd->GetTrackInfo(i);
    if (track == NULL) { sprintf(answer,"-1\n"); return; }
    sprintf(answer,"%i %i\n",track->start_sector,track->stop_sector); 
}

void CLOSE(char *msg,char *answer,int answersize) {

    printf("CLOSE %s\n",msg);

    delete audiocd;
    sprintf(answer,"0\n");
}

void PLAY(char *msg,char *answer,int answersize) {
    int tracks,tracke;

    printf("PLAY %s\n",msg);

    if (audiocd != NULL) {
        sscanf(msg,"%i %i",&tracks,&tracke);
        audiocd->Play(tracks,tracke);
        sprintf(answer,"0\n");
        return;
    }
    sprintf(answer,"-1\n");
}

void SPLAY(char *msg,char *answer,int answersize) {
    int tracks,tracke;

    printf("SPLAY %s\n",msg);

    if (audiocd != NULL) {
        sscanf(msg,"%i %i",&tracks,&tracke);
        audiocd->SPlay(tracks,tracke);
        sprintf(answer,"0\n");
        return;
    }
    sprintf(answer,"-1\n");
}

void STOP(char *msg,char *answer,int answersize) {
    int tracks,tracke;

    printf("STOP %s\n",msg);

    if (audiocd != NULL) {
        audiocd->Stop();
        audiocd->Wait(-1);
        sprintf(answer,"0\n");
        return;
    }
    sprintf(answer,"-1\n");
}

void WAIT(char *msg,char *answer,int answersize) {
    int tracks,tracke;

    printf("STOP %s\n",msg);

    if (audiocd != NULL) {
        audiocd->Wait(-1);
        sprintf(answer,"0\n");
        return;
    }
    sprintf(answer,"-1\n");
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
            << " CD-drive output-option  selector \n"
               "  --- OUTPUT OPTIONS ---\n"
                "  -F         write RAW samples to file CDDATA.raw\n"
                "  -W         write WAV samples to file CDDATA.wav\n"
                "  -R         use RTMMM\n"
                "  -D         use DART\n"
                "  -N         use NULL device\n"
                "  -C         use CD own DA\n"
                "  --- SELECTOR --- \n"
                "  -t x1,x2,x3..  play these selected tracks\n"
                "  -p s1,e1,s2,e2..  play these selected tracks ranges\n"
                "  -s a,b,c,d..   play from sector a to b then from c to d\n"
                "  -r  use remote control on pipe /PIPE/RTMMMCD\n"
                "  syntax : OPEN disk_letter            ret= #of tracks\n"
                "           CLOSE                       ret= 0\n"
                "           PLAY trackstart trackend    ret= 0\n"
                "           SPLAY sectorstart sectorend ret= 0\n"
                "           STOP                        ret= 0\n"
                "           WAIT                        ret= 0\n"
                "           INFO  track                 ret= sectorstart sectorend\n"
                "  --- CREDITS  ---\n"
                "  rtmmmcd v 1.0 is a product of FISA 1996\n"
                "  RTMMM project is a product of FISA 1996\n"
                "  Thanks for the product to:\n"
                "  Norbert Heller for the help to the DIGITAL TRANSFER\n"
                "  Giovanni Pagni & Stefano Zamprogno for the help to the CD interface\n"
                "  RTMMM is a freeware project : join in!! \n"
                "  Use RTMMM instead of DART !!! \n";
        return 0;
    }

    stream = NULL;

cerr << "argc = " << argc <<"\n";

    // parse arguments:
    for (i = 2; i < argc; ++i){
        if (argv[i][0] == '-' && argv[i][1])
            switch ((int)argv[i][1])
            {
                case 'F':
                    delete stream;
                    stream = NULL;
                    stream = new FileOutputAudioStream(2,44100,filename,AUDIO_RAW);
                break;
                case 'W':
                    delete stream;
                    stream = NULL;
                    stream = new FileOutputAudioStream(2,44100,wav_filename,AUDIO_RIFF);
                break;
                case 'R':
                    delete stream;
                    stream = NULL;
                    stream = new RTMMMOutputAudioStream(2,0);
                    if (stream->GetError() !=0 ){
                        delete stream;
                        stream = new DARTOutputAudioStream(2,0);
                        if (stream->GetError() !=0 ){
                            cerr << "Cannot allocate neither RTMMM or DART \n";
                        }
                    }
                break;
                case 'D':
                    delete stream;
                    stream = NULL;
                    stream = new DARTOutputAudioStream(2,0);
                    if (stream->GetError() !=0 ){
                        delete stream;
                        stream = new RTMMMOutputAudioStream(2,0);
                        if (stream->GetError() !=0 ){
                            cerr << "Cannot allocate neither DART or RTMMM \n";
                        }
                    }
                break;
                case 'N':
                    delete stream;
                    stream = NULL;
                    stream = new NULLOutputAudioStream(2);
                break;
                case 'C':
                    delete stream;
                    stream = NULL;
                break;
                case 't':
                if (argc > (i + 1))
                if (argv[i+1][0] != '-')
                {
                    char         *s;
                    unsigned int start,stop;
                    int          ret;

                    i++;
                    s=&argv[i][0];
                    ret = getsingle(s,&start);
                    while (ret==1){
cerr << "Playing from track " << start << " to track " << start <<"\n";
                        commands[command_no].start_track  = start;
                        commands[command_no].stop_track   = start;
                        commands[command_no].start_sector = 0x10000000;
                        commands[command_no].stop_sector  = 0x10000000;
                        ret = getsingle(NULL,&start);
                        command_no++;
                    }
                }
                break;
                case 'p':
                if (argc > (i + 1))
                if (argv[i+1][0] != '-')
                {
                    char         *s;
                    unsigned int start,stop;
                    int          ret;

                    i++;
                    s=&argv[i][0];

                    ret = getdouble(s,&start,&stop);
                    while (ret==1){
cerr << "Playing from track " << start << " to track " << stop <<"\n";
                        commands[command_no].start_track  = start;
                        commands[command_no].stop_track   = stop;
                        commands[command_no].start_sector = 0x10000000;
                        commands[command_no].stop_sector  = 0x10000000;
                        ret = getdouble(NULL,&start,&stop);
                        command_no++;
                    }
                }
                break;
                case 's':
                if (argc > (i + 1))
                if (argv[i+1][0] != '-')
                {
                    char         *s;
                    unsigned int start,stop;
                    int          ret;

                    i++;
                    s=&argv[i][0];

                    ret = getdouble(s,&start,&stop);
                    while (ret==1){
cerr << "Playing from sector " << start << " to sector " << stop <<"\n";
                        commands[command_no].start_sector = start;
                        commands[command_no].stop_sector  = stop;
                        commands[command_no].start_track  = 0x10000000;
                        commands[command_no].stop_track   = 0x10000000;
                        ret = getdouble(NULL,&start,&stop);
                        command_no++;
                    }
                }
                break;
                case 'r': use_pipe = 1;            //use pipe
                break;
                default:
                    goto usage;
            }
        }

    {
        char s[20];
        sprintf(s,"CD_DISK_%c",argv[1][0]);
        stream->SetSongName(s);
        stream->SetSourceName("RTMMM-CD");
    }

    if (use_pipe == 1 ) {
       char *name = "RTMMMCD";
       PipeServer *pipe;
 
        pipe = new PipeServer(name);

        pipe->AddFunction(OPEN, "OPEN");
        pipe->AddFunction(CLOSE, "CLOSE");
        pipe->AddFunction(INFO, "INFO");
        pipe->AddFunction(PLAY, "PLAY");
        pipe->AddFunction(SPLAY, "SPLAY");
        pipe->AddFunction(STOP, "STOP");
        pipe->AddFunction(WAIT, "WAIT");

        pipe->Activate();

        while(1) DosSleep(1000);
        return;
    }

    audiocd = new CDInputAudioStream(argv[1][0]);
    if ((err = audiocd->GetError()) !=0) { cerr << "Error Init CD err #"<<err<<"\n";Exit(-1); }

    audiocd->ShowInfo();
    if ((err = audiocd->GetError()) !=0) { cerr << "Error NOT Audio CD err #"<<err<<" \n";Exit(-1); }

    audiocd->Connect(stream);
    if ((err = audiocd->GetError()) !=0) { cerr << "Error Getting output Stream ??? err #"<<err<<"\n";Exit(-1); }

    for (i=0;i<command_no;i++){
        if (commands[i].start_track == 0x10000000)
            audiocd->SPlay(commands[i].start_sector,commands[i].stop_sector);
        else
            audiocd->Play(commands[i].start_track,commands[i].stop_track);
        if ((err = audiocd->GetError()) !=0) { cerr << "Error Playing err #"<<err<<"\n";Exit(-1); }
        audiocd->Wait(-1);
        if ((err = audiocd->GetError()) !=0) { cerr << "Error Waiting ??? err #"<<err<<"\n";Exit(-1); }
    }

    Exit(0);
}
