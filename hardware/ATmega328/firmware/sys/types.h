#ifndef _TYPES_H
#define _TYPES_H


#ifdef __cplusplus
#define RO		
#else
#define RO		const
#endif
#define RW		


typedef unsigned char                  uint8_t;
typedef signed   char                  int8_t;
typedef unsigned int                   uint16_t;
typedef signed   int                   int16_t;
typedef unsigned long                  uint32_t;
typedef signed   long                  int32_t;
typedef unsigned long long             uint64_t;
typedef signed   long long             int64_t;

typedef enum { FALSE = 0, TRUE   = 1 } bool_t;
typedef enum { INPUT = 0, OUTPUT = 1 } dir_t;
typedef enum { LOW   = 0, HIGH   = 1 } logic_t;
typedef enum { FALL  = 0, RISE   = 1 } edge_t;

typedef uint8_t                        unused_t;


#endif