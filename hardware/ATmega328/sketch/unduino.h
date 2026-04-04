#ifndef _UNDUINO_H
#define _UNDUINO_H


// cancel standard arduino defines


#ifdef INPUT
#undef INPUT
#undef OUTPUT
#endif

#ifdef LOW
#undef LOW
#undef HIGH
#endif

#ifdef HEX
#undef HEX
#endif


#endif