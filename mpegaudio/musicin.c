/**********************************************************************
Copyright (c) 1991 MPEG/audio software simulation group, All Rights Reserved
musicin.c
**********************************************************************/
/**********************************************************************
 * MPEG/audio coding/decoding software, work in progress              *
 *   NOT for public distribution until verified and approved by the   *
 *   MPEG/audio committee.  For further information, please contact   *
 *   Davis Pan, 508-493-2241, e-mail: pan@3d.enet.dec.com             *
 *                                                                    *
 * VERSION 4.0                                                        *
 *   changes made since last update:                                  *
 *   date   programmers         comment                               *
 * 3/01/91  Douglas Wong,       start of version 1.1 records          *
 *          Davis Pan                                                 *
 * 3/06/91  Douglas Wong,       rename: setup.h to endef.h            *
 *                              removed extraneous variables          *
 * 3/21/91  J.Georges Fritsch   introduction of the bit-stream        *
 *                              package. This package allows you      *
 *                              to generate the bit-stream in a       *
 *                              binary or ascii format                *
 * 3/31/91  Bill Aspromonte     replaced the read of the SB matrix    *
 *                              by an "code generated" one            *
 * 5/10/91  W. Joseph Carter    Ported to Macintosh and Unix.         *
 *                              Incorporated Jean-Georges Fritsch's   *
 *                              "bitstream.c" package.                *
 *                              Modified to strictly adhere to        *
 *                              encoded bitstream specs, including    *
 *                              "Berlin changes".                     *
 *                              Modified user interface dialog & code *
 *                              to accept any input & output          *
 *                              filenames desired.  Also added        *
 *                              de-emphasis prompt and final bail-out *
 *                              opportunity before encoding.          *
 *                              Added AIFF PCM sound file reading     *
 *                              capability.                           *
 *                              Modified PCM sound file handling to   *
 *                              process all incoming samples and fill *
 *                              out last encoded frame with zeros     *
 *                              (silence) if needed.                  *
 *                              Located and fixed numerous software   *
 *                              bugs and table data errors.           *
 * 27jun91  dpwe (Aware Inc)    Used new frame_params struct.         *
 *                              Clear all automatic arrays.           *
 *                              Changed some variable names,          *
 *                              simplified some code.                 *
 *                              Track number of bits actually sent.   *
 *                              Fixed padding slot, stereo bitrate    *
 *                              Added joint-stereo : scales L+R.      *
 * 6/12/91  Earle Jennings      added fix for MS_DOS in obtain_param  *
 * 6/13/91  Earle Jennings      added stack length adjustment before  *
 *                              main for MS_DOS                       *
 * 7/10/91  Earle Jennings      conversion of all float to FLOAT      *
 *                              port to MsDos from MacIntosh completed*
 * 8/ 8/91  Jens Spille         Change for MS-C6.00                   *
 * 8/22/91  Jens Spille         new obtain_parameters()               *
 *10/ 1/91  S.I. Sudharsanan,   Ported to IBM AIX platform.           *
 *          Don H. Lee,                                               *
 *          Peter W. Farrett                                          *
 *10/ 3/91  Don H. Lee          implemented CRC-16 error protection   *
 *                              newly introduced functions are        *
 *                              I_CRC_calc, II_CRC_calc and encode_CRC*
 *                              Additions and revisions are marked    *
 *                              with "dhl" for clarity                *
 *11/11/91 Katherine Wang       Documentation of code.                *
 *                                (variables in documentation are     *
 *                                surround by the # symbol, and an '*'*
 *                                denotes layer I or II versions)     *
 * 2/11/92  W. Joseph Carter    Ported new code to Macintosh.  Most   *
 *                              important fixes involved changing     *
 *                              16-bit ints to long or unsigned in    *
 *                              bit alloc routines for quant of 65535 *
 *                              and passing proper function args.     *
 *                              Removed "Other Joint Stereo" option   *
 *                              and made bitrate be total channel     *
 *                              bitrate, irrespective of the mode.    *
 *                              Fixed many small bugs & reorganized.  *
 * 2/25/92  Masahiro Iwadare    made code cleaner and more consistent *
 * 8/07/92  Mike Coleman        make exit() codes return error status *
 *                              made slight changes for portability   *
 *19 aug 92 Soren H. Nielsen    Changed MS-DOS file name extensions.  *
 * 8/25/92  Shaun Astarabadi    Replaced rint() function with explicit*
 *                              rounding for portability with MSDOS.  *
 * 9/22/92  jddevine@aware.com  Fixed _scale_factor_calc() calls.     *
 *10/19/92  Masahiro Iwadare    added info->mode and info->mode_ext   *
 *                              updates for AIFF format files         *
 * 3/10/93  Kevin Peterson      In parse_args, only set non default   *
 *                              bit rate if specified in arg list.    *
 *                              Use return value from aiff_read_hdrs  *
 *                              to fseek to start of sound data       *
 * 7/26/93  Davis Pan           fixed bug in printing info->mode_ext  *
 *                              value for joint stereo condition      *
 * 8/27/93 Seymour Shlien,      Fixes in Unix and MSDOS ports,        *
 *         Daniel Lauzon, and                                         *
 *         Bill Truerniet                                             *
 **********************************************************************/

#ifdef MS_DOS
#include <dos.h>
#endif
#include "common.h"
#include "encoder.h"

/* Global variable definitions for "musicin.c" */

FILE               *musicin;
Bit_stream_struc   bs;
char               *programName;

/* Implementations */

/************************************************************************
/*
/* obtain_parameters
/*
/* PURPOSE:  Prompts for and reads user input for encoding parameters
/*
/* SEMANTICS:  The parameters read are:
/* - input and output filenames
/* - sampling frequency (if AIFF file, will read from the AIFF file header)
/* - layer number
/* - mode (stereo, joint stereo, dual channel or mono)
/* - psychoacoustic model (I or II)
/* - total bitrate, irrespective of the mode
/* - de-emphasis, error protection, copyright and original or copy flags
/*
/************************************************************************/

void
obtain_parameters(fr_ps,psy,num_samples,original_file_name,encoded_file_name)
frame_params    *fr_ps;
int             *psy;
unsigned long   *num_samples;
char            original_file_name[MAX_NAME_SIZE];
char            encoded_file_name[MAX_NAME_SIZE];
{
    int j;
    long int freq;
    int model, brt;
    char t[50];
    IFF_AIFF pcm_aiff_data;
    layer *info = fr_ps->header;
    long soundPosition;
 
    do  {
       printf("Enter PCM input file name <required>: ");
       gets(original_file_name);
       if (original_file_name[0] == NULL_CHAR)
       printf("PCM input file name is required.\n");
    } while (original_file_name[0] == NULL_CHAR);
    printf(">>> PCM input file name is: %s\n", original_file_name);
 
    if ((musicin = fopen(original_file_name, "rb")) == NULL) {
       printf("Could not find \"%s\".\n", original_file_name);
       exit(1);
    }

#ifdef  MS_DOS
    printf("Enter MPEG encoded output file name <%s>: ",
           new_ext(original_file_name, DFLT_EXT)); /* 92-08-19 shn */
#else
    printf("Enter MPEG encoded output file name <%s%s>: ",
           original_file_name, DFLT_EXT);
#endif
 
    gets(encoded_file_name);
    if (encoded_file_name[0] == NULL_CHAR) {
#ifdef  MS_DOS
        /* replace old extension with new one, 92-08-19 shn */
        strcpy(encoded_file_name,new_ext(original_file_name, DFLT_EXT));
#else
        strcat(strcpy(encoded_file_name, original_file_name), DFLT_EXT);
#endif
    }
        

    printf(">>> MPEG encoded output file name is: %s\n", encoded_file_name);
 
    open_bit_stream_w(&bs, encoded_file_name, BUFFER_SIZE);
 
    if ((soundPosition = aiff_read_headers(musicin, &pcm_aiff_data)) != -1) {

       printf(">>> Using Audio IFF sound file headers\n");

       aiff_check(original_file_name, &pcm_aiff_data);

       if (fseek(musicin, soundPosition, SEEK_SET) != 0) {
          printf("Could not seek to PCM sound data in \"%s\".\n",
                 original_file_name);
          exit(1);
       }

       info->sampling_frequency = SmpFrqIndex((long)pcm_aiff_data.sampleRate);
       printf(">>> %.f Hz sampling frequency selected\n",
              pcm_aiff_data.sampleRate);

       /* Determine number of samples in sound file */
#ifndef MS_DOS
       *num_samples = pcm_aiff_data.numChannels *
                      pcm_aiff_data.numSampleFrames;
#else
       *num_samples = (long)(pcm_aiff_data.numChannels) *
                      (long)(pcm_aiff_data.numSampleFrames);
#endif

    }
    else {    /* Not using Audio IFF sound file headers. */

       printf("What is the sampling frequency? <44100>[Hz]: ");
       gets(t);
       freq = atol(t);
       switch (freq) {
          case 48000 : info->sampling_frequency = 1;
              printf(">>> %ld Hz sampling freq selected\n", freq);
              break;
          case 44100 : info->sampling_frequency = 0;
              printf(">>> %ld Hz sampling freq selected\n", freq);
              break;
          case 32000 : info->sampling_frequency = 2;
              printf(">>> %ld Hz sampling freq selected\n", freq);
              break;
          default:    info->sampling_frequency = 0;
              printf(">>> Default 44.1 kHz samp freq selected\n");
       }
 
       if (fseek(musicin, 0, SEEK_SET) != 0) {
          printf("Could not seek to PCM sound data in \"%s\".\n",
                  original_file_name);
          exit(1);
       }
 
       /* Declare sound file to have "infinite" number of samples. */
       *num_samples = MAX_U_32_NUM;

    }

    printf("Which layer do you want to use?\n");
    printf("Available: Layer (1), Layer (<2>): ");
    gets(t);
    switch(*t){
       case '1': info->lay = 1; printf(">>> Using Layer %s\n",t); break;
       case '2': info->lay = 2; printf(">>> Using Layer %s\n",t); break;
       default:  info->lay = 2; printf(">>> Using default Layer 2\n"); break;
    }

    printf("Which mode do you want?\n");
    printf("Available: (<s>)tereo, (j)oint stereo, ");
    printf("(d)ual channel, s(i)ngle Channel: ");
    gets(t);
    switch(*t){
       case 's':
       case 'S':
          info->mode = MPG_MD_STEREO; info->mode_ext = 0;
          printf(">>> Using mode %s\n",t);
          break;
       case 'j':
       case 'J':
          info->mode = MPG_MD_JOINT_STEREO;
          printf(">>> Using mode %s\n",t);
          break;
       case 'd':
       case 'D':
          info->mode = MPG_MD_DUAL_CHANNEL; info->mode_ext = 0;
          printf(">>> Using mode %s\n",t);
          break;
       case 'i':
       case 'I':
          info->mode = MPG_MD_MONO; info->mode_ext = 0;
          printf(">>> Using mode %s\n",t);
          break;
       default:
          info->mode = MPG_MD_STEREO; info->mode_ext = 0;
          printf(">>> Using default stereo mode\n");
          break;
    }

    printf("Which psychoacoustic model do you want to use? <2>: ");
    gets(t);
    model = atoi(t);
    if (model > 2 || model < 1) {
       printf(">>> Default model 2 selected\n");
       *psy = 2;
    }
    else {
       *psy = model;
       printf(">>> Using psychoacoustic model %d\n", model);
    }
 
    printf("What is the total bitrate? <%u>[kbps]: ", DFLT_BRT);
    gets(t);
    brt = atoi(t);
    if (brt == 0) brt = -10;
    j=0;
    while (j<15) {
       if (bitrate[info->lay-1][j] == brt) break;
       j++;
    }
    if (j==15) {
       printf(">>> Using default %u kbps\n", DFLT_BRT);
       for (j=0;j<15;j++)
          if (bitrate[info->lay-1][j] == DFLT_BRT) {
             info->bitrate_index = j;
             break;
          }
    }
    else{
       info->bitrate_index = j;
       printf(">>> Bitrate = %d kbps\n", bitrate[info->lay-1][j]);
    }
 
    printf("What type of de-emphasis should the decoder use?\n");
    printf("Available: (<n>)one, (5)0/15 microseconds, (c)citt j.17: ");
    gets(t);
    if (*t != 'n' && *t != '5' && *t != 'c') {
       printf(">>> Using default no de-emphasis\n");
       info->emphasis = 0;
    }
    else {
       if (*t == 'n')      info->emphasis = 0;
       else if (*t == '5') info->emphasis = 1;
       else if (*t == 'c') info->emphasis = 3;
       printf(">>> Using de-emphasis %s\n",t);
    }
 
/*  Start 2. Part changes for CD Ver 3.2; jsp; 22-Aug-1991 */
 
    printf("Do you want to set the private bit? (y/<n>): ");
    gets(t);
    if (*t == 'y' || *t == 'Y') info->extension = 1;
    else                        info->extension = 0;
    if(info->extension) printf(">>> Private bit set\n");
    else                printf(">>> Private bit not set\n");
 
/*  End changes for CD Ver 3.2; jsp; 22-Aug-1991 */
 
    printf("Do you want error protection? (y/<n>): ");
    gets(t);
    if (*t == 'y' || *t == 'Y') info->error_protection = TRUE;
    else                        info->error_protection = FALSE;
    if(info->error_protection) printf(">>> Error protection used\n");
    else printf(">>> Error protection not used\n");
 
    printf("Is the material copyrighted? (y/<n>): ");
    gets(t);
    if (*t == 'y' || *t == 'Y') info->copyright = 1;
    else                        info->copyright = 0;
    if(info->copyright) printf(">>> Copyrighted material\n");
    else                printf(">>> Material not copyrighted\n");
 
    printf("Is this the original? (y/<n>): ");
    gets(t);
    if (*t == 'y' || *t == 'Y') info->original = 1;
    else                        info->original = 0;
    if(info->original) printf(">>> Original material\n");
    else               printf(">>> Material not original\n");
 
    printf("Do you wish to exit (last chance before encoding)? (y/<n>): ");
    gets(t);
    if (*t == 'y' || *t == 'Y') exit(0);
}

/************************************************************************
/*
/* parse_args
/*
/* PURPOSE:  Sets encoding parameters to the specifications of the
/* command line.  Default settings are used for parameters
/* not specified in the command line.
/*
/* SEMANTICS:  The command line is parsed according to the following
/* syntax:
/*
/* -l  is followed by the layer number
/* -m  is followed by the mode
/* -p  is followed by the psychoacoustic model number
/* -s  is followed by the sampling rate
/* -b  is followed by the total bitrate, irrespective of the mode
/* -d  is followed by the emphasis flag
/* -c  is followed by the copyright/no_copyright flag
/* -o  is followed by the original/not_original flag
/* -e  is followed by the error_protection on/off flag
/*
/* If the input file is in AIFF format, the sampling frequency is read
/* from the AIFF header.
/*
/* The input and output filenames are read into #inpath# and #outpath#.
/*
/************************************************************************/
 
void
parse_args(argc, argv, fr_ps, psy, num_samples, inPath, outPath)
int     argc;
char    **argv;
frame_params  *fr_ps;
int     *psy;
unsigned long *num_samples;
char    inPath[MAX_NAME_SIZE];
char    outPath[MAX_NAME_SIZE];
{
   FLOAT srate;
   int   brate;
   layer *info = fr_ps->header;
   int   err = 0, i = 0;
   IFF_AIFF pcm_aiff_data;
   long samplerate;
   long soundPosition;
 
   /* preset defaults */
   inPath[0] = '\0';   outPath[0] = '\0';
   info->lay = DFLT_LAY;
   switch(DFLT_MOD) {
      case 's': info->mode = MPG_MD_STEREO; info->mode_ext = 0; break;
      case 'd': info->mode = MPG_MD_DUAL_CHANNEL; info->mode_ext=0; break;
      case 'j': info->mode = MPG_MD_JOINT_STEREO; break;
      case 'm': info->mode = MPG_MD_MONO; info->mode_ext = 0; break;
      default:
         fprintf(stderr, "%s: Bad mode dflt %c\n", programName, DFLT_MOD);
         abort();
   }
   *psy = DFLT_PSY;
   if((info->sampling_frequency = SmpFrqIndex((long)(1000*DFLT_SFQ))) < 0) {
      fprintf(stderr, "%s: bad sfrq default %.2f\n", programName, DFLT_SFQ);
      abort();
   }
   if((info->bitrate_index = BitrateIndex(info->lay, DFLT_BRT)) < 0) {
      fprintf(stderr, "%s: bad default bitrate %u\n", programName, DFLT_BRT);
      abort();
   }
   switch(DFLT_EMP) {
      case 'n': info->emphasis = 0; break;
      case '5': info->emphasis = 1; break;
      case 'c': info->emphasis = 3; break;
      default: 
         fprintf(stderr, "%s: Bad emph dflt %c\n", programName, DFLT_EMP);
         abort();
   }
   info->copyright = 0; info->original = 0; info->error_protection = FALSE;
 
   /* process args */
   while(++i<argc && err == 0) {
      char c, *token, *arg, *nextArg;
      int  argUsed;
 
      token = argv[i];
      if(*token++ == '-') {
         if(i+1 < argc) nextArg = argv[i+1];
         else           nextArg = "";
         argUsed = 0;
         while(c = *token++) {
            if(*token /* NumericQ(token) */) arg = token;
            else                             arg = nextArg;
            switch(c) {
               case 'l':        info->lay = atoi(arg); argUsed = 1;
                  if(info->lay<1 || info->lay>2) {
                     fprintf(stderr,"%s: -l layer must be 1 or 2, not %s\n",
                          programName, arg);
                     err = 1;
                  }
                  break;
               case 'm':        argUsed = 1;
                  if (*arg == 's')
                    { info->mode = MPG_MD_STEREO; info->mode_ext = 0; }
                  else if (*arg == 'd')
                    { info->mode = MPG_MD_DUAL_CHANNEL; info->mode_ext=0; }
                  else if (*arg == 'j')
                    { info->mode = MPG_MD_JOINT_STEREO; }
                  else if (*arg == 'm')
                    { info->mode = MPG_MD_MONO; info->mode_ext = 0; }
                  else {
                    fprintf(stderr,"%s: -m mode must be s/d/j/m not %s\n",
                            programName, arg);
                    err = 1;
                  }
                  break;
               case 'p':        *psy = atoi(arg); argUsed = 1;
                  if(*psy<1 || *psy>2) {
                     fprintf(stderr,"%s: -p model must be 1 or 2, not %s\n",
                             programName, arg);
                     err = 1;
                  }
                  break;

               case 's':
                  argUsed = 1;
                  srate = atof( arg );
                  /* samplerate = rint( 1000.0 * srate ); $A  */
                  samplerate = (long) (( 1000.0 * srate ) + 0.5);
                  if( (info->sampling_frequency = SmpFrqIndex((long) samplerate)) < 0 )
                      err = 1;
                  break;
                  
               case 'b':        
   		  argUsed = 1;
		  brate = atoi(arg); 
		  if( (info->bitrate_index = BitrateIndex(info->lay, brate)) < 0)
		     err=1;                  
		 break;
               case 'd':        argUsed = 1;
                  if (*arg == 'n')                    info->emphasis = 0;
                  else if (*arg == '5')               info->emphasis = 1;
                  else if (*arg == 'c')               info->emphasis = 3;
                  else {
                     fprintf(stderr,"%s: -d emp must be n/5/c not %s\n",
                             programName, arg);
                     err = 1;
                  }
                  break;
                case 'c':       info->copyright = 1; break;
                case 'o':       info->original  = 1; break;
                case 'e':       info->error_protection = TRUE; break;
                default:        fprintf(stderr,"%s: unrec option %c\n",
                                        programName, c);
                                err = 1; break;
            }
            if(argUsed) {
               if(arg == token)    token = "";   /* no more from token */
               else                ++i;          /* skip arg we used */
               arg = ""; argUsed = 0;
            }
         }
      }
      else {
         if(inPath[0] == '\0')       strcpy(inPath, argv[i]);
         else if(outPath[0] == '\0') strcpy(outPath, argv[i]);
         else {
            fprintf(stderr,"%s: excess arg %s\n", programName, argv[i]);
            err = 1;
         }
      }
   }

   if(err || inPath[0] == '\0') usage();  /* never returns */
 
   if(outPath[0] == '\0') {
      strcpy(outPath, inPath);
      strcat(outPath, DFLT_EXT);
   }
 
   if ((musicin = fopen(inPath, "rb")) == NULL) {
      printf("Could not find \"%s\".\n", inPath);
      exit(1);
   }
 
   open_bit_stream_w(&bs, outPath, BUFFER_SIZE);

   if ((soundPosition = aiff_read_headers(musicin, &pcm_aiff_data)) != -1) {

      printf(">>> Using Audio IFF sound file headers\n");

      aiff_check(inPath, &pcm_aiff_data);

      if (fseek(musicin, soundPosition, SEEK_SET) != 0) {
         printf("Could not seek to PCM sound data in \"%s\".\n", inPath);
         exit(1);
      }

      info->sampling_frequency = SmpFrqIndex((long)pcm_aiff_data.sampleRate);
      printf(">>> %.f Hz sampling frequency selected\n",
             pcm_aiff_data.sampleRate);

      /* Determine number of samples in sound file */
#ifndef MS_DOS
      *num_samples = pcm_aiff_data.numChannels *
                     pcm_aiff_data.numSampleFrames;
#else
      *num_samples = (long)(pcm_aiff_data.numChannels) *
                     (long)(pcm_aiff_data.numSampleFrames);
#endif
      if ( pcm_aiff_data.numChannels == 1 ) {
        info->mode = MPG_MD_MONO;
        info->mode_ext = 0;
      }
   }
   else {    /* Not using Audio IFF sound file headers. */

      if (fseek(musicin, 0, SEEK_SET) != 0) {
         printf("Could not seek to PCM sound data in \"%s\".\n", inPath);
         exit(1);
      }
 
      /* Declare sound file to have "infinite" number of samples. */
      *num_samples = MAX_U_32_NUM;

   }

}

/************************************************************************
/*
/* print_config
/*
/* PURPOSE:  Prints the encoding parameters used
/*
/************************************************************************/
 
void
print_config(fr_ps, psy, num_samples, inPath, outPath)
frame_params *fr_ps;
int     *psy;
unsigned long *num_samples;
char    inPath[MAX_NAME_SIZE];
char    outPath[MAX_NAME_SIZE];
{
 layer *info = fr_ps->header;
 
   printf("Encoding configuration:\n");
   if(info->mode != MPG_MD_JOINT_STEREO)
      printf("Layer=%s   mode=%s   extn=%d   psy model=%d\n",
             layer_names[info->lay-1], mode_names[info->mode],
             info->mode_ext, *psy);
   else printf("Layer=%s   mode=%s   extn=data dependant   psy model=%d\n",
               layer_names[info->lay-1], mode_names[info->mode], *psy);
   printf("samp frq=%.1f kHz   total bitrate=%d kbps\n",
          s_freq[info->sampling_frequency],
          bitrate[info->lay-1][info->bitrate_index]);
   printf("de-emph=%d   c/right=%d   orig=%d   errprot=%d\n",
          info->emphasis, info->copyright, info->original,
          info->error_protection);
   printf("input file: '%s'   output file: '%s'\n", inPath, outPath);
}
 
/************************************************************************
/*
/* main
/*
/* PURPOSE:  MPEG I Encoder supporting layers 1 and 2, and
/* psychoacoustic models 1 (MUSICAM) and 2 (AT&T)
/*
/* SEMANTICS:  One overlapping frame of audio of up to 2 channels are
/* processed at a time in the following order:
/* (associated routines are in parentheses)
/*
/* 1.  Filter sliding window of data to get 32 subband
/* samples per channel.
/* (window_subband,filter_subband)
/*
/* 2.  If joint stereo mode, combine left and right channels
/* for subbands above #jsbound#.
/* (*_combine_LR)
/*
/* 3.  Calculate scalefactors for the frame, and if layer 2,
/* also calculate scalefactor select information.
/* (*_scale_factor_calc)
/*
/* 4.  Calculate psychoacoustic masking levels using selected
/* psychoacoustic model.
/* (*_Psycho_One, psycho_anal)
/*
/* 5.  Perform iterative bit allocation for subbands with low
/* mask_to_noise ratios using masking levels from step 4.
/* (*_main_bit_allocation)
/*
/* 6.  If error protection flag is active, add redundancy for
/* error protection.
/* (*_CRC_calc)
/*
/* 7.  Pack bit allocation, scalefactors, and scalefactor select
/* information (layer 2) onto bitstream.
/* (*_encode_bit_alloc,*_encode_scale,II_transmission_pattern)
/*
/* 8.  Quantize subbands and pack them into bitstream
/* (*_subband_quantization, *_sample_encoding)
/*
/************************************************************************/

main(argc, argv)
int     argc;
char    **argv;
{
typedef double SBS[2][3][SCALE_BLOCK][SBLIMIT];
    SBS  FAR        *sb_sample;
typedef double JSBS[3][SCALE_BLOCK][SBLIMIT];
    JSBS FAR        *j_sample;
typedef double IN[2][HAN_SIZE];
    IN   FAR        *win_que;
typedef unsigned int SUB[2][3][SCALE_BLOCK][SBLIMIT];
    SUB  FAR        *subband;
 
    frame_params fr_ps;
    layer info;
    char original_file_name[MAX_NAME_SIZE];
    char encoded_file_name[MAX_NAME_SIZE];
    short FAR **win_buf;
static short FAR buffer[2][1152];
static unsigned int bit_alloc[2][SBLIMIT], scfsi[2][SBLIMIT];
static unsigned int scalar[2][3][SBLIMIT], j_scale[3][SBLIMIT];
static double FAR ltmin[2][SBLIMIT], lgmin[2][SBLIMIT], max_sc[2][SBLIMIT];
    FLOAT snr32[32];
    short sam[2][1056];
    int whole_SpF, extra_slot = 0;
    double avg_slots_per_frame, frac_SpF, slot_lag;
    int model, stereo, error_protection;
static unsigned int crc;
    int i, j, k, adb;
    unsigned long bitsPerSlot, samplesPerFrame, frameNum = 0;
    unsigned long frameBits, sentBits = 0;
    unsigned long num_samples;

#ifdef  MACINTOSH
    console_options.nrows = MAC_WINDOW_SIZE;
    argc = ccommand(&argv);
#endif

    /* Most large variables are declared dynamically to ensure
       compatibility with smaller machines */

    sb_sample = (SBS FAR *) mem_alloc(sizeof(SBS), "sb_sample");
    j_sample = (JSBS FAR *) mem_alloc(sizeof(JSBS), "j_sample");
    win_que = (IN FAR *) mem_alloc(sizeof(IN), "Win_que");
    subband = (SUB FAR *) mem_alloc(sizeof(SUB),"subband");
    win_buf = (short FAR **) mem_alloc(sizeof(short *)*2, "win_buf");
 
    /* clear buffers */
    memset((char *) buffer, 0, sizeof(buffer));
    memset((char *) bit_alloc, 0, sizeof(bit_alloc));
    memset((char *) scalar, 0, sizeof(scalar));
    memset((char *) j_scale, 0, sizeof(j_scale));
    memset((char *) scfsi, 0, sizeof(scfsi));
    memset((char *) ltmin, 0, sizeof(ltmin));
    memset((char *) lgmin, 0, sizeof(lgmin));
    memset((char *) max_sc, 0, sizeof(max_sc));
    memset((char *) snr32, 0, sizeof(snr32));
    memset((char *) sam, 0, sizeof(sam));
 
    fr_ps.header = &info;
    fr_ps.tab_num = -1;             /* no table loaded */
    fr_ps.alloc = NULL;
    info.version = MPEG_AUDIO_ID;

    programName = argv[0];
    if(argc==1)     /* no command-line args */
       obtain_parameters(&fr_ps, &model, &num_samples,
                         original_file_name, encoded_file_name);
    else
       parse_args(argc, argv, &fr_ps, &model, &num_samples,
                  original_file_name, encoded_file_name);
    print_config(&fr_ps, &model, &num_samples,
                 original_file_name, encoded_file_name);

    hdr_to_frps(&fr_ps);
    stereo = fr_ps.stereo;
    error_protection = info.error_protection;
 
    if (info.lay == 1) { bitsPerSlot = 32; samplesPerFrame = 384;  }
    else               { bitsPerSlot = 8;  samplesPerFrame = 1152; }
    /* Figure average number of 'slots' per frame. */
    /* Bitrate means TOTAL for both channels, not per side. */
    avg_slots_per_frame = ((double)samplesPerFrame /
                           s_freq[info.sampling_frequency]) *
                          ((double)bitrate[info.lay-1][info.bitrate_index] /
                           (double)bitsPerSlot);
    whole_SpF = (int) avg_slots_per_frame;
    printf("slots/frame = %d\n",whole_SpF);
    frac_SpF  = avg_slots_per_frame - (double)whole_SpF;
    slot_lag  = -frac_SpF;
    printf("frac SpF=%.3f, tot bitrate=%d kbps, s freq=%.1f kHz\n",
           frac_SpF, bitrate[info.lay-1][info.bitrate_index],
           s_freq[info.sampling_frequency]);
 
    if (frac_SpF != 0)
       printf("Fractional number of slots, padding required\n");
    else info.padding = 0;
 
    while (get_audio(musicin, buffer, num_samples, stereo, info.lay) > 0) {

       fprintf(stderr, "{%4lu}", frameNum++); fflush(stderr);
       win_buf[0] = &buffer[0][0];
       win_buf[1] = &buffer[1][0];
       if (frac_SpF != 0) {
          if (slot_lag > (frac_SpF-1.0) ) {
             slot_lag -= frac_SpF;
             extra_slot = 0;
             info.padding = 0;
             /*  printf("No padding for this frame\n"); */
          }
          else {
             extra_slot = 1;
             info.padding = 1;
             slot_lag += (1-frac_SpF);
             /*  printf("Padding for this frame\n");    */
          }
       }
       adb = (whole_SpF+extra_slot) * bitsPerSlot;

       switch (info.lay) {
 
/***************************** Layer I **********************************/
 
          case 1 :
             for (j=0;j<SCALE_BLOCK;j++)
             for (k=0;k<stereo;k++) {
                window_subband(&win_buf[k], &(*win_que)[k][0], k);
                filter_subband(&(*win_que)[k][0], &(*sb_sample)[k][0][j][0]);
             }

             I_scale_factor_calc(*sb_sample, scalar, stereo);
             if(fr_ps.actual_mode == MPG_MD_JOINT_STEREO) {
                I_combine_LR(*sb_sample, *j_sample);
                I_scale_factor_calc(j_sample, &j_scale, 1);
             }
 
             put_scale(scalar, &fr_ps, max_sc);
 
             if (model == 1) I_Psycho_One(buffer, max_sc, ltmin, &fr_ps);
             else {
                for (k=0;k<stereo;k++) {
                   psycho_anal(&buffer[k][0],&sam[k][0], k, info.lay, snr32,
                               (FLOAT)s_freq[info.sampling_frequency]*1000);
                   for (i=0;i<SBLIMIT;i++) ltmin[k][i] = (double) snr32[i];
                }
             }
 
             I_main_bit_allocation(ltmin, bit_alloc, &adb, &fr_ps);
 
             if (error_protection) I_CRC_calc(&fr_ps, bit_alloc, &crc);
 
             encode_info(&fr_ps, &bs);
 
             if (error_protection) encode_CRC(crc, &bs);
 
             I_encode_bit_alloc(bit_alloc, &fr_ps, &bs);
             I_encode_scale(scalar, bit_alloc, &fr_ps, &bs);
             I_subband_quantization(scalar, *sb_sample, j_scale, *j_sample,
                                    bit_alloc, *subband, &fr_ps);
             I_sample_encoding(*subband, bit_alloc, &fr_ps, &bs);
             for (i=0;i<adb;i++) put1bit(&bs, 0);
          break;
 
/***************************** Layer 2 **********************************/
 
          case 2 :
             for (i=0;i<3;i++) for (j=0;j<SCALE_BLOCK;j++)
                for (k=0;k<stereo;k++) {
                   window_subband(&win_buf[k], &(*win_que)[k][0], k);
                   filter_subband(&(*win_que)[k][0], &(*sb_sample)[k][i][j][0]);
                }
 
                II_scale_factor_calc(*sb_sample, scalar, stereo, fr_ps.sblimit);
                pick_scale(scalar, &fr_ps, max_sc);
                if(fr_ps.actual_mode == MPG_MD_JOINT_STEREO) {
                   II_combine_LR(*sb_sample, *j_sample, fr_ps.sblimit);
                   II_scale_factor_calc(j_sample, &j_scale, 1, fr_ps.sblimit);
                }       /* this way we calculate more mono than we need */
                        /* but it is cheap */
 
                if (model == 1) II_Psycho_One(buffer, max_sc, ltmin, &fr_ps);
                else {
                   for (k=0;k<stereo;k++) {
                      psycho_anal(&buffer[k][0],&sam[k][0], k, 
                                 info.lay, snr32,
                                 (FLOAT)s_freq[info.sampling_frequency]*1000);
                      for (i=0;i<SBLIMIT;i++) ltmin[k][i] = (double) snr32[i];
                   }
                }
 
                II_transmission_pattern(scalar, scfsi, &fr_ps);
                II_main_bit_allocation(ltmin, scfsi, bit_alloc, &adb, &fr_ps);
 
                if (error_protection)
                   II_CRC_calc(&fr_ps, bit_alloc, scfsi, &crc);
 
                encode_info(&fr_ps, &bs);
 
                if (error_protection) encode_CRC(crc, &bs);
 
                II_encode_bit_alloc(bit_alloc, &fr_ps, &bs);
                II_encode_scale(bit_alloc, scfsi, scalar, &fr_ps, &bs);
                II_subband_quantization(scalar, *sb_sample, j_scale,
                                      *j_sample, bit_alloc, *subband, &fr_ps);
                II_sample_encoding(*subband, bit_alloc, &fr_ps, &bs);
                for (i=0;i<adb;i++) put1bit(&bs, 0);
          break;
 
/***************************** Layer 3 **********************************/

          case 3 : break;

       }
 
       frameBits = sstell(&bs) - sentBits;
       if(frameBits%bitsPerSlot)   /* a program failure */
          fprintf(stderr,"Sent %ld bits = %ld slots plus %ld\n",
                  frameBits, frameBits/bitsPerSlot,
                  frameBits%bitsPerSlot);
       sentBits += frameBits;

    }

    close_bit_stream_w(&bs);

    printf("Avg slots/frame = %.3f; b/smp = %.2f; br = %.3f kbps\n",
           (FLOAT) sentBits / (frameNum * bitsPerSlot),
           (FLOAT) sentBits / (frameNum * samplesPerFrame),
           (FLOAT) sentBits / (frameNum * samplesPerFrame) *
           s_freq[info.sampling_frequency]);

    if (fclose(musicin) != 0){
       printf("Could not close \"%s\".\n", original_file_name);
       exit(2);
    }

#ifdef  MACINTOSH
    set_mac_file_attr(encoded_file_name, VOL_REF_NUM, CREATOR_ENCODE,
                      FILETYPE_ENCODE);
#endif

    printf("Encoding of \"%s\" with psychoacoustic model %d is finished\n",
           original_file_name, model);
    printf("The MPEG encoded output file name is \"%s\"\n",
            encoded_file_name);
    exit(0);
}
 
/************************************************************************
/*
/* usage
/*
/* PURPOSE:  Writes command line syntax to the file specified by #stderr#
/*
/************************************************************************/

static void usage()  /* print syntax & exit */
{
    fprintf(stderr,
    "usage: %s                         queries for all arguments, or\n",
            programName);
    fprintf(stderr,
    "       %s [-l lay][-m mode][-p psy][-s sfrq][-b br][-d emp]\n",
            programName);
    fprintf(stderr,
    "          [-c][-o][-e] inputPCM [outBS]\n");
    fprintf(stderr,"where\n");
    fprintf(stderr," -l lay   use layer <lay> coding   (dflt %4u)\n",DFLT_LAY);
    fprintf(stderr," -m mode  channel mode : s/d/j/m   (dflt %4c)\n",DFLT_MOD);
    fprintf(stderr," -p psy   psychoacoustic model 1/2 (dflt %4u)\n",DFLT_PSY);
    fprintf(stderr," -s sfrq  input smpl rate in kHz   (dflt %4.1f)\n",DFLT_SFQ);
    fprintf(stderr," -b br    total bitrate in kbps    (dflt %4u)\n",DFLT_BRT);
    fprintf(stderr," -d emp   de-emphasis n/5/c        (dflt %4c)\n",DFLT_EMP);
    fprintf(stderr," -c       mark as copyright\n");
    fprintf(stderr," -o       mark as original\n");
    fprintf(stderr," -e       add error protection\n");
    fprintf(stderr," inputPCM input PCM sound file (standard or AIFF)\n");
    fprintf(stderr," outBS    output bit stream of encoded audio (dflt inName+%s)\n",
            DFLT_EXT);
    exit(1);
}

/************************************************************************
/*
/* aiff_check
/*
/* PURPOSE:  Checks AIFF header information to make sure it is valid.
/*           Exits if not.
/*
/************************************************************************/

void aiff_check(file_name, pcm_aiff_data)
char *file_name;          /* Pointer to name of AIFF file */
IFF_AIFF *pcm_aiff_data;  /* Pointer to AIFF data structure */
{

#ifdef IFF_LONG
    if (pcm_aiff_data->sampleType != IFF_ID_SSND) {
#else
    if (strncmp(&pcm_aiff_data->sampleType,IFF_ID_SSND,4)) {
#endif
       printf("Sound data is not PCM in \"%s\".\n", file_name);
       exit(1);
    }

    if(SmpFrqIndex((long)pcm_aiff_data->sampleRate) < 0) {
       printf("in \"%s\".\n", file_name);
       exit(1);
    }

    if (pcm_aiff_data->sampleSize != sizeof(short) * BITS_IN_A_BYTE) {
        printf("Sound data is not %d bits in \"%s\".\n",
               sizeof(short) * BITS_IN_A_BYTE, file_name);
        exit(1);
    }

    if (pcm_aiff_data->numChannels != MONO &&
        pcm_aiff_data->numChannels != STEREO) {
       printf("Sound data is not mono or stereo in \"%s\".\n", file_name);
       exit(1);
    }

    if (pcm_aiff_data->blkAlgn.blockSize != 0) {
       printf("Block size is not %lu bytes in \"%s\".\n", 0, file_name);
       exit(1);
    }

    if (pcm_aiff_data->blkAlgn.offset != 0) {
       printf("Block offset is not %lu bytes in \"%s\".\n", 0, file_name);
       exit(1);
    }

}
