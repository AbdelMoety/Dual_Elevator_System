/**
 * Std_Types.h
 * Simple type aliases matching the tutorial style.
 */
#ifndef STD_TYPES_H
#define STD_TYPES_H

typedef signed char         sint8;
typedef unsigned char       uint8;
typedef signed short        sint16;
typedef unsigned short      uint16;
typedef signed long         sint32;
typedef unsigned long       uint32;
typedef unsigned long long  uint64;
typedef signed long long    sint64;
typedef float               float32;
typedef double              float64;
typedef unsigned char       boolean;

#define FALSE   ((boolean)0)
#define TRUE    ((boolean)1)
#ifndef NULL
#define NULL    ((void*)0)
#endif

#define OK      0U
#define NOK     1U

#endif
