#ifdef _MSC_VER
#define MEOW_API static __declspec(noinline)
#include <intrin.h>
#elif __EMSCRIPTEN__
// Build with: emcc -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -o meow.html meow_bench.c
#include <emscripten.h>
#define __rdtsc() ((meow_u64)(emscripten_get_now() * 1e+6));
#define aligned_alloc(_alignment, _size) memalign(_alignment, _size)
#else
// assume clang
#define MEOW_API static __attribute__((noinline))
#define __rdtsc __builtin_readcyclecounter
#endif
#include "../meow_hash_c.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

typedef struct
{
    meow_u64 Size;
    meow_u64 Clocks;
} best_result;

static void
FuddleBuffer(meow_u64 Size, void *Buffer)
{
    // NOTE(casey): This code is here for literally no purpose other than to prevent CLANG from
    // optimizing out loads, since apparently it thinks it doesn't have to actually read from
    // uninitialized memory, WHICH IS ABSURD and this whole undefined behavior thing is completely
    // unacceptable.  Spec writers are ALL FIRED.
    meow_u8 *Dest = (meow_u8 *)Buffer;
    for(meow_u64 Index = 0;
        Index < Size;
        ++Index)
    {
        Dest[Index] = 13*Index;
    }
}

static void
PrintSize(FILE *Stream, double Size, int Fixed)
{
    char *Suffix = Fixed ? (char *)"b " : (char *)"b";
    if(Size >= 1024.0)
    {
        Suffix = (char *)"kb";
        Size /= 1024.0;
        if(Size >= 1024.0)
        {
            Suffix = (char *)"mb";
            Size /= 1024.0;
            if(Size >= 1024.0)
            {
                Suffix = (char *)"gb";
                Size /= 1024.0;
            }
        }
    }
    
    fprintf(Stream, Fixed ? "%4.0f%s" : "%0.0f%s", Size, Suffix);
}

int main()
{
    meow_u64 MaxClocksWithoutDrop = 4000000000ULL/8;
#ifdef __EMSCRIPTEN__
    // Browsers seemingly can't deal with allocations of this size under wasm
    best_result Bests[36] = {};
#else
    best_result Bests[40] = {};
#endif
    double BytesPerCycle[ArrayCount(Bests)] = {};
    
    {
        int BestIndex = 0;
        Bests[BestIndex++].Size = 1;
        Bests[BestIndex++].Size = 7;
        Bests[BestIndex++].Size = 8;
        Bests[BestIndex++].Size = 15;
        Bests[BestIndex++].Size = 16;
        Bests[BestIndex++].Size = 31;
        Bests[BestIndex++].Size = 32;
        Bests[BestIndex++].Size = 63;
        Bests[BestIndex++].Size = 64;
        Bests[BestIndex++].Size = 127;
        Bests[BestIndex++].Size = 128;
        Bests[BestIndex++].Size = 255;
        Bests[BestIndex++].Size = 256;
        Bests[BestIndex++].Size = 511;
        Bests[BestIndex++].Size = 512;
        Bests[BestIndex++].Size = 1023;
        Bests[BestIndex++].Size = 1024;
        meow_u64 Size = Bests[BestIndex - 1].Size;
        while (BestIndex < ArrayCount(Bests))
        {
            Size *= 2;
            Bests[BestIndex++].Size = Size;
        }
    }
    
    {
        fprintf(stderr, "Single-threaded performance:\n");
        for (int Batch = 0; Batch < ArrayCount(Bests); ++Batch)
        {
            best_result *ThisBest = Bests + Batch;
            meow_u64 Size = ThisBest->Size;
            ThisBest->Clocks = (meow_u64)-1ULL;
            
            void *Buffer = aligned_alloc(MEOW_HASH_ALIGNMENT, Size);
            if (Buffer)
            {
                FuddleBuffer(Size, Buffer);
                fprintf(stderr, "  Fewest cycles to hash ");
                PrintSize(stderr, Size, 0);
                
                meow_u64 ClocksSinceLastDrop = 0;
                meow_u64 BestClocks = (meow_u64)-1ULL;
                int TryIndex = 0;
                while ((TryIndex < 10) || (ClocksSinceLastDrop < MaxClocksWithoutDrop))
                {
                    meow_u64 StartClock = __rdtsc();
                    meow_hash h = meow_hash_c(0, Size, Buffer);
                    meow_u64 EndClock = __rdtsc();
                    volatile meow_hash h0;
                    h0 = h;
                    
                    meow_u64 Clocks = EndClock - StartClock;
                    ClocksSinceLastDrop += Clocks;
                    
                    if (BestClocks > Clocks)
                    {
                        ClocksSinceLastDrop = 0;
                        BestClocks = Clocks;
                    }
                    
                    ++TryIndex;
                }
                
                double BPC = (double)Size / (double)BestClocks;
                fprintf(stderr, "%10.0f (%3.03f bytes/cycle)\n", (double)BestClocks, BPC);
                fflush(stderr);
                
                BytesPerCycle[Batch] = BPC;
                
                if (ThisBest->Clocks > BestClocks)
                {
                    ThisBest->Clocks = BestClocks;
                }
                
                free(Buffer);
            }
        }
    }
    
    fprintf(stderr, "\n");
    
    fprintf(stderr, "Leaderboard:\n");
    for (int BestIndex = 0; BestIndex < ArrayCount(Bests); ++BestIndex)
    {
        best_result *Best = Bests + BestIndex;
        fprintf(stderr, "  ");
        PrintSize(stderr, Best->Size, 1);
        double BPC = (double)Best->Size / (double)Best->Clocks;
        fprintf(stderr, ": %10.0f (%3.03f bytes/cycle)\n", (double)Best->Clocks, BPC);
    }
    
    fprintf(stderr, "\n");    
}
