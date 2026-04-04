#ifndef _TYPES_H
#define _TYPES_H


#define TRUE				1
#define FALSE				0

#define INPUT				1
#define OUTPUT				0

#define HIGH				1
#define LOW					0

#define RISE				1
#define FALL				0

#define RO					const
#define RW					

#define SFR					volatile


typedef unsigned char       uint8_t;
typedef signed   char       int8_t;
typedef unsigned int        uint16_t;
typedef signed   int        int16_t;
typedef unsigned short long uint24_t;
typedef signed   short long int24_t;
typedef unsigned long       uint32_t;
typedef signed   long       int32_t;

typedef uint8_t             bool_t;
typedef uint8_t             dir_t;
typedef uint8_t             logic_t;
typedef uint8_t             edge_t;
typedef uint8_t             unused_t;


#endif