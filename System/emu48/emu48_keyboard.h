#pragma once


#include "pch.h"
#include "emu48.h"
#include "io.h"


typedef struct emu48_key_t
{
    uint32_t keyid;
    uint16_t out;
    uint16_t in;
}emu48_key_t;


#define DEC_EMU48_KEY(name, id, _out_, _in_)    \
    extern emu48_key_t name;

#define DEF_EMU48_KEY(name, id, _out_, _in_)    \
    emu48_key_t name = {.keyid = id, .out = _out_, .in = _in_ };


