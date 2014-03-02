
#ifndef ALL_H
#define ALL_H

typedef float		real;		// float should be enough
typedef short		bool;
typedef unsigned	uint32;		// 32 Bit unsigned integer
	// some compilers may need "typedef unsigned long uint32" instead
typedef int		int32;		// 32 Bit signed integer
	// some compilers may need "typedef long int32" instead
typedef unsigned short	uint16;		// 16 Bit unsigned integer
typedef short		int16;		// 16 Bit signed integer

#ifndef True
#define True	1
#define False	0
#endif

#endif
