/*############################################################################*/
/*                                                                           #*/
/*       Real Time MultiMedia Mixer  v1.11    4 November 1996                #*/
/*         FISA comps.      F.Sartori      OS/2                              #*/
/*                                                                           #*/
/*############################################################################*/

#if !defined(__RTMMM__)
#define __RTMMM__

#define  INCL_DOS
#define  INCL_OS2MM
#include <os2.h>
#include <os2me.h>

#define   RTMMM_VERSION        211              /* major release 2 shared memory version 11 */

#define   RTMMM_MEMORY         "\\SHAREMEM\\RTMMM_SHARED"
#define   RTMMM_QUEUE          "\\QUEUES\\RTMMM_QUEUE"
#define   RTMMM_PIPE           "\\PIPE\\RTMMM_PIPE"
#define   RTMMM_SEM            "\\SEM32\\RTMMM_NEWS"
#define   ERR_MCI 1
#define   ERR_DOS 2

/* Constants to define messages to be sent via queue to the RTMMM server */
#define   RTMMM_QUIT           0xFFFF            /* quit the server */
#define   RTMMM_CONNECT        0x1111            /* attach a source of sound */
#define   RTMMM_DISCONNECT     0x2222            /* disconnect a source of sound */
#define   RTMMM_SCOPE          0x3333            /* request for a scope like consumer */
#define   RTMMM_SETOPTIONS     0x4444            /* set the parameters of the MIXER */


#define   FILTER_ORDER         1                 /* first order filter */
#define   RTMMM_MAX_INPUTS     32                /* the maximum number of inputs */
#define   RTMMM_BUFFERS        8                 /* the amount of RTMMM buffers */
#define   RTMMM_BUFFERS_MASK   7                 /* the mask to rotate among buffers */
#define   RTMMM_BUFFERS_SIZE   32768             /* the size of a buffer */
#define   RTMMM_FREQUENCY      44100             /* the sampling rate */

#define   RTMMM_MIXER_PRI      10                /* all the clients priority must be below that if TIME CRITICAL */

/* A Mixer Input Channel Shared Memory */
typedef struct {
    unsigned int            frequency_ratio;     /* the ratio between the USER frequency and the MIXER frequency 2^8 times the number of channels */
    int                     k[FILTER_ORDER];     /* the coeffs for the filter * 2^15 */
    int                     size;                /* 1 for 8 bit samples mono 2 for 16bit */
    int                     channels;            /* 1 for mono 2 stereo */
    void                    *buffer;             /* which buffer to use (must point within the shared memory */
    HEV                     syncsem;             /* the synchronization semaphore to block the source */
    char                    input_type[80];      /* the name of the type of input */
    char                    music_name[80];      /* the name of the file being played */
    int                     preferred_channel;   /* if -1 then automatically allocate a channel otherwise fail if the channel is busy */
} RTMMM_InputChannel_type;

typedef struct {
    int                     old_output[2];       /* the old values */
    int                     kright;              /* the gain of the channel */
    int                     kleft;               /* the gain of the channel */
    int                     mute;                /* the mute */
} RTMMM_InputSettings_type;

/* The \SHAREMEM\RTMMM_SHARED memory Read Only buffer */
typedef struct {
    int                     version_number;      /* the version number */
    HEV                     news_semaphore;      /* The semaphore to tell a client user interface that things have changed  */
    int                     buffers_size;        /* size in bytes */
    int                     buffers_number;      /* the number of buffers */
    int                     inputs_number;       /* the number of input channels */
    RTMMM_InputChannel_type *inputs[RTMMM_MAX_INPUTS]; /* pointer to the input channels */
    RTMMM_InputSettings_type input_settings[RTMMM_MAX_INPUTS]; /* the channels settings */
    int                     master_volume;       /* the master volume * 32768 */
    unsigned short          dart_device_id;      /* Amp Mixer device id  */
    MCI_MIXSETUP_PARMS      MixSetupParms;       /* Mixer parameters     */
    MCI_BUFFER_PARMS        BufferParms;         /* Device buffer parms  */
    MCI_MIX_BUFFER          *buffers;            /* Device buffers   */
    TID                     mixer_thread;        /* The mixer thread */
    TID                     interact_thread;     /* The thread listening to the QUEUE */
    HEV                     syncsemaphore;       /* The semaphore to Sync the mixer with the player */
    HQUEUE                  message_queue;       /* The client messages are sent here */
} RTMMM_Definition_type;


typedef struct {
    unsigned short Right;
    unsigned short Left;
} RTMMM_stereo_type;


void RTMMM_Error(int error,int err_class,char *extra);



#endif