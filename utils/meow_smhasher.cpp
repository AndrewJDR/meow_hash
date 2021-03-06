/* ========================================================================

   meow_smhasher.cpp - smhasher-compatible calls for the Meow hash
   (C) Copyright 2018 by Molly Rocket, Inc. (https://mollyrocket.com)
   
   See https://mollyrocket.com/meowhash for license and details.
   
   ======================================================================== */

#if _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#include "meow_hash.h"

//
// NOTE(casey): 128-bit wide implementation (Meow1)
//

void
Meow1_32(const void * key, int len, meow_u32 seed, void * out)
{
    meow_hash Result = MeowHash1(seed, len, (void *)key);
    *(meow_u32 *)out = Result.u32[0];
}

void
Meow1_64(const void * key, int len, meow_u32 seed, void * out)
{
    meow_hash Result = MeowHash1(seed, len, (void *)key);
    ((meow_u64 *)out)[0] = Result.u64[0];
}

void
Meow1_128(const void * key, int len, meow_u32 seed, void * out)
{
    meow_hash Result = MeowHash1(seed, len, (void *)key);
    ((meow_u64 *)out)[0] = Result.u64[0];
    ((meow_u64 *)out)[1] = Result.u64[1];
}

#if MEOW_HASH_AVX512

//
// NOTE(casey): 256-bit wide implementation (Meow2)
//

void
Meow2_32(const void * key, int len, meow_u32 seed, void * out)
{
    meow_hash Result = MeowHash2(seed, len, (void *)key);
    *(meow_u32 *)out = Result.u32[0];
}

void
Meow2_64(const void * key, int len, meow_u32 seed, void * out)
{
    meow_hash Result = MeowHash2(seed, len, (void *)key);
    ((meow_u64 *)out)[0] = Result.u64[0];
}

void
Meow2_128(const void * key, int len, meow_u32 seed, void * out)
{
    meow_hash Result = MeowHash2(seed, len, (void *)key);
    ((meow_u64 *)out)[0] = Result.u64[0];
    ((meow_u64 *)out)[1] = Result.u64[1];
}

//
// NOTE(casey): 512-bit wide implementation (Meow4)
//

void
Meow4_32(const void * key, int len, meow_u32 seed, void * out)
{
    meow_hash Result = MeowHash4(seed, len, (void *)key);
    *(meow_u32 *)out = Result.u32[0];
}

void
Meow4_64(const void * key, int len, meow_u32 seed, void * out)
{
    meow_hash Result = MeowHash4(seed, len, (void *)key);
    ((meow_u64 *)out)[0] = Result.u64[0];
}

void
Meow4_128(const void * key, int len, meow_u32 seed, void * out)
{
    meow_hash Result = MeowHash4(seed, len, (void *)key);
    ((meow_u64 *)out)[0] = Result.u64[0];
    ((meow_u64 *)out)[1] = Result.u64[1];
}

#endif