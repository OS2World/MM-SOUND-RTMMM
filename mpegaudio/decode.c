/**********************************************************************
Copyright (c) 1991 MPEG/audio software simulation group, All Rights Reserved
decode.c
**********************************************************************/
/**********************************************************************
 * MPEG/audio coding/decoding software, work in progress              *
 *   NOT for public distribution until verified and approved by the   *
 *   MPEG/audio committee.  For further information, please contact   *
 *   Davis Pan, 508-493-2241, e-mail: pan@3d.enet.dec.com          *
 *                                                                    *
 * VERSION 3.9                                                       *
 *   changes made since last update:                                  *
 *   date   programmers         comment                               *
 * 2/25/91  Douglas Wong,       start of version 1.0 records          *
 *          Davis Pan                                                 *
 * 3/06/91  Douglas Wong        rename: setup.h to dedef.h            *
 *                                      dfilter to defilter           *
 *                                      dwindow to dewindow           *
 *                              integrated "quantizer", "scalefactor" *
 *                              combined window_samples routine into  *
 *                              filter samples                        *
 * 3/31/91  Bill Aspromonte     replaced read_filter by               *
 *                              create_syn_filter and introduced a    *
 *                              new Sub-Band Synthesis routine called *
 *                              SubBandSynthesis()                    *
 * 5/10/91  Vish (PRISM)        Ported to Macintosh and Unix.         *
 *                              Changed "out_fifo()" so that last     *
 *                              unfilled block is also written out.   *
 *                              "create_syn_filter()" was modified so *
 *                              that calculation precision is same as *
 *                              in specification tables.              *
 *                              Changed "decode_scale()" to reflect   *
 *                              specifications.                       *
 *                              Removed all routines used by          *
 *                              "synchronize_buffer()".  This is now  *
 *                              replaced by "seek_sync()".            *
 *                              Incorporated Jean-Georges Fritsch's   *
 *                              "bitstream.c" package.                *
 *                              Deleted "reconstruct_sample()".       *
 * 27jun91  dpwe (Aware)        Passed outFile and &sampFrames as     *
 *                              args to out_fifo() - were global.     *
 *                              Moved "alloc_*" reader to common.c.   *
 *                              alloc, sblimit, stereo passed via new *
 *                              'frame_params struct (were globals).  *
 *                              Added JOINT STEREO decoding, lyrs I&II*
 *                              Affects: decode_bitalloc,buffer_samps *
 *                              Plus a few other cleanups.            *
 * 6/10/91   Earle Jennings     conditional expansion added in        *
 *                              II_dequantize_sample to handle range  *
 *                              problems in MSDOS version             *
 * 8/8/91    Jens Spille        Change for MS-C6.00                   *
 *10/1/91    S.I. Sudharsanan,  Ported to IBM AIX platform.           *
 *           Don H. Lee,                                              *
 *           Peter W. Farrett                                         *
 *10/3/91    Don H. Lee         implemented CRC-16 error protection   *
 *                              newly introduced functions are        *
 *                              buffer_CRC and recover_CRC_error.     *
 * 2/11/92  W. Joseph Carter    Ported new code to Macintosh.  Most   *
 *                              important fixes involved changing     *
 *                              16-bit ints to long or unsigned in    *
 *                              bit alloc routines for quant of 65535 *
 *                              and passing proper function args.     *
 *                              Removed "Other Joint Stereo" option   *
 *                              and made bitrate be total channel     *
 *                              bitrate, irrespective of the mode.    *
 *                              Fixed many small bugs & reorganized.  *
 * 7/27/92  Juan Pineda         Bug fix in SubBandSynthesis()         *
 **********************************************************************/

#include        "common.h"
#include        "decoder.h"

/***************************************************************
/*
/* This module contains the core of the decoder ie all the
/* computational routines. (Layer I and II only)
/* Functions are common to both layer unless
/* otherwise specified.
/*
/***************************************************************/

/*****************************************************************
/*
/* The following routines decode the system information
/*
/****************************************************************/

/************ Layer I, Layer II & Layer III ******************/

void decode_info(bs, fr_ps)
Bit_stream_struc *bs;
frame_params *fr_ps;
{
    layer *hdr = fr_ps->header;

    hdr->version = get1bit(bs);
    hdr->lay = 4-getbits(bs,2);
    hdr->error_protection = !get1bit(bs); /* error protect. TRUE/FALSE */
    hdr->bitrate_index = getbits(bs,4);
    hdr->sampling_frequency = getbits(bs,2);
    hdr->padding = get1bit(bs);
    hdr->extension = get1bit(bs);
    hdr->mode = getbits(bs,2);
    hdr->mode_ext = getbits(bs,2);
    hdr->copyright = get1bit(bs);
    hdr->original = get1bit(bs);
    hdr->emphasis = getbits(bs,2);
}

/*******************************************************************
/*
/* The bit allocation information is decoded. Layer I
/* has 4 bit per subband whereas Layer II is Ws and bit rate
/* dependent.
/*
/********************************************************************/

/**************************** Layer II *************/

void II_decode_bitalloc(bs, bit_alloc, fr_ps)
Bit_stream_struc *bs;
unsigned int bit_alloc[2][SBLIMIT];
frame_params *fr_ps;
{
    int i,j;
    int stereo = fr_ps->stereo;
    int sblimit = fr_ps->sblimit;
    int jsbound = fr_ps->jsbound;
    al_table *alloc = fr_ps->alloc;

    for (i=0;i<jsbound;i++) for (j=0;j<stereo;j++)
        bit_alloc[j][i] = (char) getbits(bs,(*alloc)[i][0].bits);

    for (i=jsbound;i<sblimit;i++) /* expand to 2 channels */
        bit_alloc[0][i] = bit_alloc[1][i] =
            (char) getbits(bs,(*alloc)[i][0].bits);

    for (i=sblimit;i<SBLIMIT;i++) for (j=0;j<stereo;j++)
        bit_alloc[j][i] = 0;
}

/**************************** Layer I *************/

void I_decode_bitalloc(bs, bit_alloc, fr_ps)
Bit_stream_struc *bs;
unsigned int bit_alloc[2][SBLIMIT];
frame_params *fr_ps;
{
    int i,j;
    int stereo  = fr_ps->stereo;
    int sblimit = fr_ps->sblimit;
    int jsbound = fr_ps->jsbound;
    int b;

    for (i=0;i<jsbound;i++) for (j=0;j<stereo;j++)
        bit_alloc[j][i] = getbits(bs,4);
    for (i=jsbound;i<SBLIMIT;i++) {
        b = getbits(bs,4);
        for (j=0;j<stereo;j++)
            bit_alloc[j][i] = b;
    }
}

/*****************************************************************
/*
/* The following two functions implement the layer I and II
/* format of scale factor extraction. Layer I involves reading
/* 6 bit per subband as scale factor. Layer II requires reading
/* first the scfsi which in turn indicate the number of scale factors
/* transmitted.
/*    Layer I : I_decode_scale
/*   Layer II : II_decode_scale
/*
/****************************************************************/

/************************** Layer I stuff ************************/

void I_decode_scale(bs, bit_alloc, scale_index, fr_ps)
Bit_stream_struc *bs;
unsigned int bit_alloc[2][SBLIMIT], scale_index[2][3][SBLIMIT];
frame_params *fr_ps;
{
    int i,j;
    int stereo = fr_ps->stereo;
    int sblimit = fr_ps->sblimit;

    for (i=0;i<SBLIMIT;i++) for (j=0;j<stereo;j++)
        if (!bit_alloc[j][i])
            scale_index[j][0][i] = SCALE_RANGE-1;
        else                    /* 6 bit per scale factor */
            scale_index[j][0][i] =  getbits(bs,6);

}

/*************************** Layer II stuff ***************************/

void II_decode_scale(bs,scfsi, bit_alloc,scale_index, fr_ps)
Bit_stream_struc *bs;
unsigned int scfsi[2][SBLIMIT], bit_alloc[2][SBLIMIT],
             scale_index[2][3][SBLIMIT];
frame_params *fr_ps;
{
    int i,j;
    int stereo = fr_ps->stereo;
    int sblimit = fr_ps->sblimit;
   
    for (i=0;i<sblimit;i++) for (j=0;j<stereo;j++) /* 2 bit scfsi */
        if (bit_alloc[j][i]) scfsi[j][i] = (char) getbits(bs,2);
    for (i=sblimit;i<SBLIMIT;i++) for (j=0;j<stereo;j++)   
        scfsi[j][i] = 0;

    for (i=0;i<sblimit;i++) for (j=0;j<stereo;j++) {
        if (bit_alloc[j][i])   
            switch (scfsi[j][i]) {
                /* all three scale factors transmitted */
             case 0 : scale_index[j][0][i] = getbits(bs,6);
                scale_index[j][1][i] = getbits(bs,6);
                scale_index[j][2][i] = getbits(bs,6);
                break;
                /* scale factor 1 & 3 transmitted */
             case 1 : scale_index[j][0][i] =
                 scale_index[j][1][i] = getbits(bs,6);
                scale_index[j][2][i] = getbits(bs,6);
                break;
                /* scale factor 1 & 2 transmitted */
             case 3 : scale_index[j][0][i] = getbits(bs,6);
                scale_index[j][1][i] =
                    scale_index[j][2][i] =  getbits(bs,6);
                break;
                /* only one scale factor transmitted */
             case 2 : scale_index[j][0][i] =
                 scale_index[j][1][i] =
                     scale_index[j][2][i] = getbits(bs,6);
                break;
                default : break;
            }
        else {
            scale_index[j][0][i] = scale_index[j][1][i] =
                scale_index[j][2][i] = SCALE_RANGE-1;
        }
    }
    for (i=sblimit;i<SBLIMIT;i++) for (j=0;j<stereo;j++) {
        scale_index[j][0][i] = scale_index[j][1][i] =
            scale_index[j][2][i] = SCALE_RANGE-1;
    }
}

/**************************************************************
/*
/*   The following two routines take care of reading the
/* compressed sample from the bit stream for both layer 1 and
/* layer 2. For layer 1, read the number of bits as indicated
/* by the bit_alloc information. For layer 2, if grouping is
/* indicated for a particular subband, then the sample size has
/* to be read from the bits_group and the merged samples has
/* to be decompose into the three distinct samples. Otherwise,
/* it is the same for as layer one.
/*
/**************************************************************/

/******************************* Layer I stuff ******************/

void I_buffer_sample(bs, sample, bit_alloc, fr_ps)
unsigned int FAR sample[2][3][SBLIMIT];
unsigned int bit_alloc[2][SBLIMIT];
Bit_stream_struc *bs;
frame_params *fr_ps;
{
    int i,j,k;
    int stereo = fr_ps->stereo;
    int sblimit = fr_ps->sblimit;
    int jsbound = fr_ps->jsbound;
    unsigned int s;

    for (i=0;i<jsbound;i++) for (j=0;j<stereo;j++)
        if ( (k = bit_alloc[j][i]) == 0)
            sample[j][0][i] = 0;
        else 
            sample[j][0][i] = (unsigned int) getbits(bs,k+1);
    for (i=jsbound;i<SBLIMIT;i++) {
        if ( (k = bit_alloc[0][i]) == 0)
            s = 0;
        else 
            s = (unsigned int)getbits(bs,k+1);
        for (j=0;j<stereo;j++)
            sample[j][0][i]    = s;
    }
}

/*************************** Layer II stuff ************************/

void II_buffer_sample(bs,sample,bit_alloc,fr_ps)
unsigned int FAR sample[2][3][SBLIMIT];
unsigned int bit_alloc[2][SBLIMIT];
Bit_stream_struc *bs;
frame_params *fr_ps;
{
    int i,j,k,m;
    int stereo = fr_ps->stereo;
    int sblimit = fr_ps->sblimit;
    int jsbound = fr_ps->jsbound;
    al_table *alloc = fr_ps->alloc;

    for (i=0;i<sblimit;i++) for (j=0;j<((i<jsbound)?stereo:1);j++) {
        if (bit_alloc[j][i]) {
            /* check for grouping in subband */
            if ((*alloc)[i][bit_alloc[j][i]].group==3)
                for (m=0;m<3;m++) {
                    k = (*alloc)[i][bit_alloc[j][i]].bits;
                    sample[j][m][i] = (unsigned int) getbits(bs,k);
                }         
            else {              /* bit_alloc = 3, 5, 9 */
                unsigned int nlevels, c=0;

                nlevels = (*alloc)[i][bit_alloc[j][i]].steps;
                k=(*alloc)[i][bit_alloc[j][i]].bits;
                c = (unsigned int) getbits(bs, k);
                for (k=0;k<3;k++) {
                    sample[j][k][i] = c % nlevels;
                    c /= nlevels;
                }
            }
        }
        else {                  /* for no sample transmitted */
            for (k=0;k<3;k++) sample[j][k][i] = 0;
        }
        if(stereo == 2 && i>= jsbound) /* joint stereo : copy L to R */
            for (k=0;k<3;k++) sample[1][k][i] = sample[0][k][i];
    }
    for (i=sblimit;i<SBLIMIT;i++) for (j=0;j<stereo;j++) for (k=0;k<3;k++)
        sample[j][k][i] = 0;
}      

/**************************************************************
/*
/*   Restore the compressed sample to a factional number.
/*   first complement the MSB of the sample
/*    for layer I :
/*    Use s = (s' + 2^(-nb+1) ) * 2^nb / (2^nb-1)
/*   for Layer II :
/*   Use the formula s = s' * c + d
/*
/**************************************************************/

static double c[17] = { 1.33333333333, 1.60000000000, 1.14285714286,
                        1.77777777777, 1.06666666666, 1.03225806452,
                        1.01587301587, 1.00787401575, 1.00392156863,
                        1.00195694716, 1.00097751711, 1.00048851979,
                        1.00024420024, 1.00012208522, 1.00006103888,
                        1.00003051851, 1.00001525902 };

static double d[17] = { 0.500000000, 0.500000000, 0.250000000, 0.500000000,
                        0.125000000, 0.062500000, 0.031250000, 0.015625000,
                        0.007812500, 0.003906250, 0.001953125, 0.0009765625,
                        0.00048828125, 0.00024414063, 0.00012207031,
                        0.00006103516, 0.00003051758 };

/************************** Layer II stuff ************************/

void II_dequantize_sample(sample, bit_alloc, fraction, fr_ps)
unsigned int FAR sample[2][3][SBLIMIT];
unsigned int bit_alloc[2][SBLIMIT];
double FAR fraction[2][3][SBLIMIT];
frame_params *fr_ps;
{
    int i, j, k, x;
    int stereo = fr_ps->stereo;
    int sblimit = fr_ps->sblimit;
    al_table *alloc = fr_ps->alloc;

    for (i=0;i<sblimit;i++)  for (j=0;j<3;j++) for (k=0;k<stereo;k++)
        if (bit_alloc[k][i]) {

            /* locate MSB in the sample */
            x = 0;
#ifndef MS_DOS
            while ((1L<<x) < (*alloc)[i][bit_alloc[k][i]].steps) x++;
#else
            /* microsoft C thinks an int is a short */
            while (( (unsigned long) (1L<<(long)x) <
                    (unsigned long)( (*alloc)[i][bit_alloc[k][i]].steps)
                    ) && ( x < 16) ) x++;
#endif

            /* MSB inversion */
            if (((sample[k][j][i] >> x-1) & 1) == 1)
                fraction[k][j][i] = 0.0;
            else  fraction[k][j][i] = -1.0;

            /* Form a 2's complement sample */
            fraction[k][j][i] += (double) (sample[k][j][i] & ((1<<x-1)-1)) /
                (double) (1L<<x-1);

            /* Dequantize the sample */
            fraction[k][j][i] += d[(*alloc)[i][bit_alloc[k][i]].quant];
            fraction[k][j][i] *= c[(*alloc)[i][bit_alloc[k][i]].quant];
        }
        else fraction[k][j][i] = 0.0;   
   
    for (i=sblimit;i<SBLIMIT;i++) for (j=0;j<3;j++) for(k=0;k<stereo;k++)
        fraction[k][j][i] = 0.0;
}

/***************************** Layer I stuff ***********************/

void I_dequantize_sample(sample, fraction, bit_alloc, fr_ps)
unsigned int FAR sample[2][3][SBLIMIT];
unsigned int bit_alloc[2][SBLIMIT];
double FAR fraction[2][3][SBLIMIT];
frame_params *fr_ps;
{
    int i, nb, k;
    int stereo = fr_ps->stereo;
    int sblimit = fr_ps->sblimit;

    for (i=0;i<SBLIMIT;i++)
        for (k=0;k<stereo;k++)
            if (bit_alloc[k][i]) {
                nb = bit_alloc[k][i] + 1;
                if (((sample[k][0][i] >> nb-1) & 1) == 1) fraction[k][0][i] = 0.0;
                else fraction[k][0][i] = -1.0;
                fraction[k][0][i] += (double) (sample[k][0][i] & ((1<<nb-1)-1)) /
                    (double) (1L<<nb-1);

                fraction[k][0][i] =
                    (double) (fraction[k][0][i]+1.0/(double)(1L<<nb-1)) *
                        (double) (1L<<nb) / (double) ((1L<<nb)-1);
            }
            else fraction[k][0][i] = 0.0;
}

/************************************************************
/*
/*   Restore the original value of the sample ie multiply
/*    the fraction value by its scalefactor.
/*
/************************************************************/

/************************* Layer II Stuff **********************/

void II_denormalize_sample(fraction, scale_index,fr_ps,x)
double FAR fraction[2][3][SBLIMIT];
unsigned int scale_index[2][3][SBLIMIT];
frame_params *fr_ps;
int x;
{
    int i,j,k;
    int stereo = fr_ps->stereo;
    int sblimit = fr_ps->sblimit;

    for (i=0;i<sblimit;i++) for (j=0;j<stereo;j++) {
        fraction[j][0][i] *= multiple[scale_index[j][x][i]];
        fraction[j][1][i] *= multiple[scale_index[j][x][i]];
        fraction[j][2][i] *= multiple[scale_index[j][x][i]];
    }
}

/**************************** Layer I stuff ******************************/

void I_denormalize_sample(fraction,scale_index,fr_ps)
double FAR fraction[2][3][SBLIMIT];
unsigned int scale_index[2][3][SBLIMIT];
frame_params *fr_ps;
{
    int i,j,k;
    int stereo = fr_ps->stereo;
    int sblimit = fr_ps->sblimit;

    for (i=0;i<SBLIMIT;i++) for (j=0;j<stereo;j++)
        fraction[j][0][i] *= multiple[scale_index[j][0][i]];
}

/*****************************************************************
/*
/* The following are the subband synthesis routines. They apply
/* to both layer I and layer II stereo or mono. The user has to
/* decide what parameters are to be passed to the routines.
/*
/***************************************************************/

/*************************************************************
/*
/*   Pass the subband sample through the synthesis window
/*
/**************************************************************/

/* create in synthesis filter */

void create_syn_filter(filter)
double FAR filter[64][SBLIMIT];
{
    register int i,k;

    for (i=0; i<64; i++)
        for (k=0; k<32; k++) {
            if ((filter[i][k] = 1e9*cos((double)((PI64*i+PI4)*(2*k+1)))) >= 0)
                modf(filter[i][k]+0.5, &filter[i][k]);
            else
                modf(filter[i][k]-0.5, &filter[i][k]);
            filter[i][k] *= 1e-9;
        }
}

/***************************************************************
/*
/*   Window the restored sample
/*
/***************************************************************/

/* read in synthesis window */

void read_syn_window(window)
double FAR window[HAN_SIZE];
{
    int i,j[4];
    FILE *fp;
    double f[4];
    char t[150];

    if (!(fp = OpenTableFile("dewindow") )) {
        printf("Please check synthesis window table 'dewindow'\n");
        exit(1);
    }
    for (i=0;i<512;i+=4) {
        fgets(t, 150, fp);
        sscanf(t,"D[%d] = %lf D[%d] = %lf D[%d] = %lf D[%d] = %lf\n",
               j, f,j+1,f+1,j+2,f+2,j+3,f+3);
        if (i==j[0]) {
            window[i] = f[0];
            window[i+1] = f[1];
            window[i+2] = f[2];
            window[i+3] = f[3];
        }
        else {
            printf("Check index in synthesis window table\n");
            exit(1);
        }
        fgets(t,150,fp);
    }
    fclose(fp);
}

int SubBandSynthesis (bandPtr, channel, samples)
double *bandPtr;
int channel;
short *samples;
{
    register int i,j,k;
    register double *bufOffsetPtr, sum;
    static int init = 1;
    typedef double NN[64][32];
    static NN FAR *filter;
    typedef double BB[2][2*HAN_SIZE];
    static BB FAR *buf;
    static int bufOffset = 64;
    static double FAR *window;
    int clip = 0;               /* count & return how many samples clipped */

    if (init) {
        buf = (BB FAR *) mem_alloc(sizeof(BB),"BB");
        filter = (NN FAR *) mem_alloc(sizeof(NN), "NN");
        create_syn_filter(*filter);
        window = (double FAR *) mem_alloc(sizeof(double) * HAN_SIZE, "WIN");
        read_syn_window(window);
        bufOffset = 64;
        init = 0;
    }
    if (channel == 0) bufOffset = (bufOffset - 64) & 0x3ff;
    bufOffsetPtr = &((*buf)[channel][bufOffset]);

    for (i=0; i<64; i++) {
        sum = 0;
        for (k=0; k<32; k++)
            sum += bandPtr[k] * (*filter)[i][k];
        bufOffsetPtr[i] = sum;
    }
    /*  S(i,j) = D(j+32i) * U(j+32i+((i+1)>>1)*64)  */
    /*  samples(i,j) = MWindow(j+32i) * bufPtr(j+32i+((i+1)>>1)*64)  */
    for (j=0; j<32; j++) {
        sum = 0;
        for (i=0; i<16; i++) {
            k = j + (i<<5);
            sum += window[k] * (*buf) [channel] [( (k + ( ((i+1)>>1) <<6) ) +
                                                  bufOffset) & 0x3ff];
        }

/*   {long foo = (sum > 0) ? sum * SCALE + 0.5 : sum * SCALE - 0.5; */
     {long foo = sum * SCALE;
     if (foo >= (long) SCALE)      {samples[j] = SCALE-1; ++clip;}
     else if (foo < (long) -SCALE) {samples[j] = -SCALE;  ++clip;}
     else                           samples[j] = foo;
 }
    }
    return(clip);
}

void out_fifo(pcm_sample, num, fr_ps, done, outFile, psampFrames)
short FAR pcm_sample[2][3][SBLIMIT];
int num;
frame_params *fr_ps;
int done;
FILE *outFile;
unsigned long *psampFrames;
{
    int i,j,l;
    int stereo = fr_ps->stereo;
    int sblimit = fr_ps->sblimit;
    static short int outsamp[1600];
    static long k = 0;

    if (!done)
        for (i=0;i<num;i++) for (j=0;j<SBLIMIT;j++) {
            (*psampFrames)++;
            for (l=0;l<stereo;l++) {
                if (!(k%1600) && k) {
                    fwrite(outsamp,2,1600,outFile);
                    k = 0;
                }
                outsamp[k++] = pcm_sample[l][i][j];
            }
        }
    else {
        fwrite(outsamp,2,(int)k,outFile);
        k = 0;
    }
}

void  buffer_CRC(bs, old_crc)
Bit_stream_struc  *bs;
unsigned int  *old_crc;
{
    *old_crc = getbits(bs, 16);
}

void  recover_CRC_error(pcm_sample, error_count, fr_ps, outFile, psampFrames)
short FAR pcm_sample[2][3][SBLIMIT];
int error_count;
frame_params *fr_ps;
FILE *outFile;
unsigned long *psampFrames;
{
    int  stereo = fr_ps->stereo;
    int  num, done, i;
    int  samplesPerFrame, samplesPerSlot;
    layer  *hdr = fr_ps->header;
    long  offset;
    short  *temp;

    num = 3;
    if (hdr->lay == 1) num = 1;

    samplesPerSlot = SBLIMIT * num * stereo;
    samplesPerFrame = samplesPerSlot * 32;

    if (error_count == 1) {     /* replicate previous error_free frame */
        done = 1;
        /* flush out fifo */
        out_fifo(pcm_sample, num, fr_ps, done, outFile, psampFrames);
        /* go back to the beginning of the previous frame */
        offset = sizeof(short int) * samplesPerFrame;
        fseek(outFile, -offset, SEEK_CUR);
        done = 0;
        for (i = 0; i < SCALE_BLOCK; i++) {
            fread(pcm_sample, 2, samplesPerSlot, outFile);
            out_fifo(pcm_sample, num, fr_ps, done, outFile, psampFrames);
        }
    }
    else{                       /* mute the frame */
        temp = (short*) pcm_sample;
        done = 0;
        for (i = 0; i < 2*3*SBLIMIT; i++)
            *temp++ = MUTE;     /* MUTE value is in decoder.h */
        for (i = 0; i < SCALE_BLOCK; i++)
            out_fifo(pcm_sample, num, fr_ps, done, outFile, psampFrames);
    }
}

