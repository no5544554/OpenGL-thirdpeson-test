#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

#include <stdint.h>

#define FPS                 60
#define DEG_TO_RAD          0.01745329
#define VECTOR_NORMALIZE    0.707107f

typedef enum { FALSE, TRUE } BOOL;



typedef struct Color32_s
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Color32;

#endif
