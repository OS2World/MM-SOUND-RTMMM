          INTERNATIONAL ORGANIZATION FOR STANDARDIZATION
           ORGANISATION INTERNATIONALE DE NORMALISATION
                               ISO/IEC JTC1/SC29/WG 11
            CODING OF MOVING PICTURES AND ASSOCIATED AUDIO

								MPEG92/
								Nov. 1992

Source: Davis Pan (Digital Equipment Corporation),
            Chairman of the MPEG/audio ad hoc committee on software
            simulation
Title: Working Draft of MPEG/Audio Technical Report

Disclaimer of Warranty
	These software programs are available to the user without any 
license fee or royalty on an "as is" basis.  ISO disclaims any and all 
warranties,  whether express, implied, or statuary, including any 
implied warranties or merchantability or of fitness for a particular 
purpose.  In no event shall ISO be liable for any incidental, punitive, 
or consequential damages of any kind whatsoever arising from the 
use of these programs.

	This disclaimer of warranty extends to the user of these 
programs and user's customers, employees, agents, transferees, 
successors, and assigns,

	ISO does not represent or warrant that the programs furnished 
hereunder are free of infringement or any third-party patents, 
copyrights or trade secrets.

	The purpose of this software is to provide a tool to help in the 
learning and understanding of the MPEG/audio compression and 
decompression algorithm.  It is not an efficient implementation.

Organization of this Report

	The main body of this report describes the organization and 
use of the software.  The listings of the software, sample makefiles, 
and test bitstreams are contained in the appendices:

	Appendix  A contains the source code for the MPEG/audio 
software, written in the C programming language.  This software has 
been run and verified a large variety of computers and operating 
systems.

	Appendix B contains sample "makefiles" which can be used to
compile the software.  Before compiling, please examine the file
common.h to determine if any of the "#define" declarations should be
reactivated by removing it from a comment statement.

	Appendix  C contains a minimal bitstream test that can be used 
to verify the proper operation of the MPEG/audio software.  The 
bitstream test consists of three bitstreams:
   orig.mpg	- The original, coded MPEG/audio bitstream
   deco.dec	- The audio data resulting from decoding orig.mpg
   renc.dec	- The encoded MPEG/audio bitstream obtained by 
		   encoding deco.dec

	The software is functioning properly if the following equations
hold:
 	a. decoded(orig.mpg) == deco.dec
	    byte-swapping of deco.dec will be necessary for this 
	   equation to hold for little-endian computers
	b. encoded(deco.dec) == renc.mpg

	    (encode with the default options except for the following:
	    48 kHz sampling rate and 256 kbits/sec coded bit rate)

	If the bitstream tests fail, make sure that the following 
variable types have at least the precision listed below:

	integer	-	16 or 32bits
	float	-	32 bits
	double	-	64 bits.

Electronic Distribution

	All the data listed above may be obtained in electronic form 
(e-mail) by contacting
:
	 Mr. Frank Laczko
	 tel:		214-997-3988
	 FAX:		214-997-5763
	 e-mail:	frank@laczko.ti.com

	Other electronic distribution methods may become available 
soon.  Managers of public access FTP sites  are encouraged to make 
this software available on their sites.

Organization of the Code

The MPEG/audio Layer 1 and Layer 2 ** software package consists 
of: 
	21 data files tables
	8 source files (*.c)
	3 definitions files (*.h)
	3 test bitstreams
	* makefiles

** The layer 3 software is currently work in progress and will be 
included in a future revision of this report.

Table 1 illustrates how the encoder and decoder is formed from the 
component files.  In this table the definition files are enclosed in 
parenthesis and listed immediately below the primary source file 
which uses them.  The data file names are listed within braces and 
also placed immediately below the source file which uses them.

			Table 1

encoder			common			decoder
files			files			files
----------      ------------    ------------
musicin.c		common.c		musicout.c
encode.c		(common.h)		decode.c
(encoder.h) 	{alloc_0}  		(decoder.h)
{enwindow}		{alloc_1}		{dewindow}
psy.c, subs.c	{alloc_2}
{absthr_0}		{alloc_3}
{absthr_1}
{absthr_2}
tonal.c
{1cb0}, {1cb1}, {1cb2}
{2cb0}, {2cb1}, {2cb2}
{1th0}, {1th1}, {1th2}
{2th0}, {2th1}, {2th2}

Running the Software

To run this software, compile the programs to form an encoder 
executable file, musicin, and a decoder executable file, musicout.
	To run the code type the name of the file followed by a 
carriage return.  The programs will prompt you to input the 
appropriate parameters.  The sound input file for the encoder should 
be sound data, monophonic or stereophonic, sampled at 32, 44.1, or 
48 kHz with 16 bits per sample.  For stereophonic data the left 
channel sample should precede the right channel sample.  The sound 
output file of the decoder will be the same format as the sound input 
file used by the decoder,  except for possible byte order differences 
if the encoder and decoder programs are run on different computer
systems which have different byte ordering conventions.

		Special notes for MSDOS users:
1. The default bitrate option does not work.
2. The input/output filename defaults not compatible with MSDOS.
3. Use the large memory model for compilation.

Notes on the Software

	The encoder and decoder software are configured to output the
coded audio bitstreams as a string of hexadecimal ascii characters. 
For greater compression efficiency,  compile flag, BS_FORMAT, in 
common.h can be switched to configure the bitstream reading and 
writing routines to process raw binary bitstreams.

	The decoder program has a very crude implementation of 
bitstream synchword detection.  It may not be able to correctly 
decode valid bitstreams which have false synchword patterns in the 
ancillary data portion of the bitstream.

Appendix A MPEG/audio Source Code
(included elsewhere)

Appendix B Sample "makefiles"
(included elsewhere)

Appendix C Test Bitstreams
(included elsewhere)

Appendix D List of Contributors
Bill Aspromonte
Shaun Astarabadi
R. Bittner
Karlheinz Brandenburg
W. Joseph Carter
Jack Chang
Mike Coleman
Johnathan Devine
Ernst Eberlein
Dan Ellis
Peter Farrett
Jean-Georges Fritsch
Vlad Fruchter
Hendrik Fuchs
Bernhard Grill
Amit Gulati
Munsi Haque
Chuck Hsiao
Toshiyuki Ishino
Masahiro Iwadare
Earl Jennings
James Johnston
Leon v.d. Kerkhof
Don Lee
Mike Li
Yu-Tang Lin
Soren Neilsen
Simao F. Campos Neto
Mark Paley
Davis Pan
Tan Ah Peng
Kevin Peterson
Juan Pineda
Ernst F. Schroeder
Peter Siebert
Jens Spille
Sam Stewart
Al Tabayoyon
Kathy Wang
Franz-Otto Witte
Douglas Wong
