/**********************************************************************
Copyright (c) 1991 MPEG/audio software simulation group, All Rights Reserved
decoder.h
**********************************************************************/
/**********************************************************************
 * MPEG/audio coding/decoding software, work in progress              *
 *   NOT for public distribution until verified and approved by the   *
 *   MPEG/audio committee.  For further information, please contact   *
 *   Davis Pan, 508-493-2241, e-mail: pan@gauss.enet.dec.com          *
 *                                                                    *
 * VERSION 3.5                                                       *
 *   changes made since last update:                                  *
 *   date   programmers         comment                               *
 * 2/25/91  Doulas Wong,        start of version 1.0 records          *
 *          Davis Pan                                                 *
 * 5/10/91  Vish (PRISM)        Renamed and regrouped all ".h" files  *
 *                              into "common.h" and "decoder.h".      *
 *                              Ported to Macintosh and Unix.         *
 * 27jun91  dpwe (Aware)        New prototype for out_fifo()          *
 *                              Moved "alloc_*" stuff to common.h     *
 *                              Use ifdef PROTO_ARGS for prototypes   *
 *                              prototypes reflect frame_params struct*
 * 10/3/91  Don H. Lee          implemented CRC-16 error protection   *
 * 2/11/92  W. Joseph Carter    Ported new code to Macintosh.  Most   *
 *                              important fixes involved changing     *
 *                              16-bit ints to long or unsigned in    *
 *                              bit alloc routines for quant of 65535 *
 *                              and passing proper function args.     *
 *                              Removed "Other Joint Stereo" option   *
 *                              and made bitrate be total channel     *
 *                              bitrate, irrespective of the mode.    *
 *                              Fixed many small bugs & reorganized.  *
 *                              Modified some function prototypes.    *
 * 08/07/92 Mike Coleman        Made small changes for portability    *
 **********************************************************************/

/***********************************************************************
*
*  Decoder Include Files
*
***********************************************************************/

/***********************************************************************
*
*  Decoder Definitions
*
***********************************************************************/

#define   DFLT_OPEXT        ".dec"  /* default output file name extension */
/*
 NOTE: The value of a multiple-character constant is
 implementation-defined.
*/
#if !defined(MS_DOS) && !defined(AIX)
#define   FILTYP_DEC_AIFF   'AIFF'
#define   FILTYP_DEC_BNRY   'TEXT'
#define   CREATR_DEC_AIFF   'Sd2a'
/*
  The following character constant is ASCII '????'
  It is declared in hex because the character
  constant contains a trigraph, causing an error in
  parsing with ANSI preprocessors.
*/
#define   CREATR_DEC_BNRY   0x3f3f3f3f
#else
#define   FILTYP_DEC_AIFF   "AIFF"
#define   FILTYP_DEC_BNRY   "TEXT"
#define   CREATR_DEC_AIFF   "Sd2a"
#define   CREATR_DEC_BNRY   "????"
#endif

#define   SYNC_WORD         (long) 0xfff
#define   SYNC_WORD_LNGTH   12

#define   MUTE              0

/***********************************************************************
*
*  Decoder Type Definitions
*
***********************************************************************/

/***********************************************************************
*
*  Decoder Variable External Declarations
*
***********************************************************************/

/***********************************************************************
*
*  Decoder Function Prototype Declarations
*
***********************************************************************/

/* The following functions are in the file "musicout.c" */

#ifdef   PROTO_ARGS
extern void   usage(void);
#else
extern void   usage();
#endif

/* The following functions are in the file "decode.c" */

#ifdef   PROTO_ARGS
extern void   decode_info(Bit_stream_struc*, frame_params*);
extern void   II_decode_bitalloc(Bit_stream_struc*, unsigned int[2][SBLIMIT],
                       frame_params*);
extern void   I_decode_bitalloc(Bit_stream_struc*, unsigned int[2][SBLIMIT],
                       frame_params*);
extern void   I_decode_scale(Bit_stream_struc*, unsigned int[2][SBLIMIT],
                       unsigned int[2][3][SBLIMIT], frame_params*);
extern void   II_decode_scale(Bit_stream_struc*, unsigned int[2][SBLIMIT],
                       unsigned int[2][SBLIMIT], unsigned int[2][3][SBLIMIT],
                       frame_params*);
extern void   I_buffer_sample(Bit_stream_struc*, unsigned int[2][3][SBLIMIT],
                       unsigned int[2][SBLIMIT], frame_params*);
extern void   II_buffer_sample(Bit_stream_struc*, unsigned int[2][3][SBLIMIT],
                       unsigned int[2][SBLIMIT], frame_params*);
extern void   read_quantizer_table(double[17], double[17]);
extern void   II_dequantize_sample(unsigned int[2][3][SBLIMIT], 
                       unsigned int[2][SBLIMIT], double[2][3][SBLIMIT],
                       frame_params*);
extern void   I_dequantize_sample(unsigned int[2][3][SBLIMIT],
                       double[2][3][SBLIMIT], unsigned int[2][SBLIMIT],
                       frame_params*);
extern void   read_scale_factor(double[SCALE_RANGE]);
extern void   II_denormalize_sample(double[2][3][SBLIMIT],
                       unsigned int[2][3][SBLIMIT], frame_params*, int);
extern void   I_denormalize_sample(double[2][3][SBLIMIT],
                       unsigned int[2][3][SBLIMIT], frame_params*);
extern void   create_syn_filter(double[64][SBLIMIT]);
extern int    SubBandSynthesis (double*, int, short*);
extern void   read_syn_window(double[HAN_SIZE]);
extern void   window_sample(double*, double*);
extern void   out_fifo(short[2][3][SBLIMIT], int, frame_params*, int, FILE*,
                       unsigned long*);
extern void   buffer_CRC(Bit_stream_struc*, unsigned int*);
extern void   recover_CRC_error(short[2][3][SBLIMIT], int, frame_params*,
                       FILE*, unsigned long*);
#else
extern void   decode_info();
extern void   II_decode_bitalloc();
extern void   I_decode_bitalloc();
extern void   I_decode_scale();
extern void   II_decode_scale();
extern void   I_buffer_sample();
extern void   II_buffer_sample();
extern void   read_quantizer_table();
extern void   II_dequantize_sample();
extern void   I_dequantize_sample();
extern void   read_scale_factor();
extern void   II_denormalize_sample();
extern void   I_denormalize_sample();
extern void   create_syn_filter();
extern int    SubBandSynthesis ();
extern void   read_syn_window();
extern void   window_sample();
extern void   out_fifo();
extern void   buffer_CRC();
extern void   recover_CRC_error();
#endif

