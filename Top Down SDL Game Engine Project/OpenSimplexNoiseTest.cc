/*
 * OpenSimplex (Simplectic) Noise Test in C++
 * Original version by Arthur Tombs * Modified 2014-09-22
 * Cleaned up version by Michaelangel007 * Removed PNG & C++ bload
 *
 * This file is intended to test the function of OpenSimplexNoise.hh.
 *
 * Compile with:
 *   g++ -o OpenSimplexNoiseTest -O2 OpenSimplexNoiseTest.cc
 *
 * Additional optimization can be obtained with -Ofast (at the cost of accuracy)
 * and -msse4 (or the highest level of SSE your CPU supports).
 */

#ifdef _MSC_VER // Shutup Microsoft Visual C++ warnings about fopen()
    #define _CRT_SECURE_NO_WARNINGS
#else
    #include <sys/time.h> // gettimeofday() // Linux, OSX but NOT on Windows
#endif

#include <stdio.h>    // fopen()
//#include <stdlib.h> // rand(), srand() // Optimization: Removed useless randomization
#include <math.h>     // floor()
#include <stdint.h>   // uint8_t
#include <time.h>     // time()
#include <string.h>   // memset()
#include <stdint.h>   // int8_t

#ifdef _MSC_VER
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>

    /*
    typedef uint16_t WORD ;
    typedef uint32_t DWORD;

    typedef struct _FILETIME {
        DWORD dwLowDateTime;
        DWORD dwHighDateTime;
    } FILETIME;

    typedef struct _SYSTEMTIME {
          WORD wYear;
          WORD wMonth;
          WORD wDayOfWeek;
          WORD wDay;
          WORD wHour;
          WORD wMinute;
          WORD wSecond;
          WORD wMilliseconds;
        } SYSTEMTIME, *PSYSTEMTIME;
    */

    // WTF!?!? Exists in winsock2.h
    typedef struct timeval {
        long tv_sec;
        long tv_usec;
    } timeval;

    // *sigh* no gettimeofday on Win32/Win64
    int gettimeofday(struct timeval * tp, struct timezone * tzp)
    {
        static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

        SYSTEMTIME  system_time;
        FILETIME    file_time;
        uint64_t    time;

        GetSystemTime( &system_time );
        SystemTimeToFileTime( &system_time, &file_time );
        time =  ((uint64_t)file_time.dwLowDateTime )      ;
        time += ((uint64_t)file_time.dwHighDateTime) << 32;

        tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
        tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
        return 0;
    }
#endif

#include "OpenSimplexNoise.hh"

const int   WIDTH        = 1024;
const int   HEIGHT       = 1024;
const float FEATURE_SIZE = 24.0f;

bool Targa_Save( const char *filename, const uint16_t width, const uint16_t height, const void *texels, const int bitsPerPixel )
{
    if( (!filename)
    ||  (!texels)
    ||  (!width)
    ||  (!height)
    ||  (!bitsPerPixel))
        return false;

    if( (bitsPerPixel == 24) || (bitsPerPixel == 32) )
    {
        FILE *pFile = fopen( filename, "wb" );
        if( pFile )
        {
            const int TGA_HEADER_SIZE = 18;
            uint8_t header[ TGA_HEADER_SIZE ];
            memset( header, 0, TGA_HEADER_SIZE );

            // http://www.dca.fee.unicamp.br/~martino/disciplinas/ea978/tgaffs.pdf
            // Truevision TGA -- FILE FORMAT SPECIFICATION -- Version 2.0
            //[ 0] // image id
            //[ 1] // colormap present
            header[ 2] = 2; // uncompressed
            //[ 3] // colormap offset 16-bit
            //[ 5] // colormap entries 16-bit
            //[ 7] // colormap bits per pixel
            //[ 8] // x-origin 16-bit
            //[10] // y-origin 16-bit
            header[12] = width & 255;
            header[13] =(width >> 8) & 0xFF;
            header[14] = height & 255;
            header[15] =(height >> 8) & 0xFF;
            header[16] = bitsPerPixel;
            // 0 0 bottom left
            // 0 1 bottom right
            // 1 0 top left
            // 1 1 top right
            header[17] = 0x20; // bits 5 & 4 = direction for copy to screen

            if( fwrite( header, TGA_HEADER_SIZE, 1, pFile) )
                if( fwrite( texels, (width * height * bitsPerPixel) >> 3, 1, pFile) )
                {
                    fclose( pFile );
                    return true;
                }

            fclose( pFile );
        }
    }

    return false;
}

// calls Noise() WIDTH*HEIGHT number of times
void generate( OpenSimplexNoise & noise, float *values )
{
    for (int yi = 0; yi < HEIGHT; yi++)
    {
        float y = (-0.5f + yi / (float)(HEIGHT-1)) * (HEIGHT / FEATURE_SIZE);
        for (int xi = 0; xi < WIDTH; xi++)
        {
            float x = (-0.5f + xi / (float)(WIDTH-1)) * (WIDTH / FEATURE_SIZE);
            values[ xi+yi*WIDTH ] = noise.eval( x, y, 0.0f );
        }
    }

}

void quantize( float *values, uint8_t *texels )
{
    float   *pSrc = values;
    uint8_t *pDst = texels;

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            int8_t i =  (uint8_t) floor( ((*pSrc++ * 0.5f) + 0.5f) * 255.0f ); // BUGFIX: remove +0.5 bias so final 0 .. 255
            *pDst++ = i; // r
            *pDst++ = i; // g
            *pDst++ = i; // b
        }
    }
}

struct DataRate
{
    char     prefix ;
    uint64_t samples;
    uint64_t per_sec;
};

class Timer
{
    timeval start, end;
public:
    double   elapsed; // total seconds
    uint32_t mins;
    uint32_t secs;
    DataRate throughput;

    void Start() {
        gettimeofday( &start, NULL );
    }

    void Stop() {
        gettimeofday( &end, NULL );
        elapsed = (end.tv_sec - start.tv_sec);

        mins = (uint32_t)elapsed / 60;
        secs = (uint32_t)elapsed - (mins*60);
    }

    // size is number of bytes in a file, or number of iterations that you want to benchmark
    void Throughput( uint64_t size )
    {
        const int MAX_PREFIX = 4;
        DataRate datarate[MAX_PREFIX] = {
            {' '}, {'K'}, {'M'}, {'G'}
        };

        int best = 0;
        for( int units = 0; units < MAX_PREFIX; units++ ) {
            datarate[ units ].samples  = size >> (10*units);
            datarate[ units ].per_sec = (uint64_t) (datarate[units].samples / elapsed);
            if (datarate[units].per_sec > 0)
                best = units;
        }
        throughput = datarate[ best ];
    }
};


int main( int nArg, char *aArg[] )
{

    float   *values = new float [WIDTH * HEIGHT];
    uint8_t *texels = new uint8_t [ WIDTH * HEIGHT * 3 ]; // rgb

    // Default Seed
    {
        OpenSimplexNoise noise;
        generate( noise , values ); // generate float array
        quantize( values, texels ); // convert to 24-bit grayscale
        Targa_Save( "simplex_noise_default.tga", WIDTH, HEIGHT, texels, 24 );
    }

    // Open Simplex Seed specified
    bool bBenchmark = (nArg > 1); // TODO: -bench
    bool bSaveNoise = (nArg > 2); // TODO: -save

    if( bBenchmark )
    {
        Timer timer;
        timer.Start();
        uint64_t samples = 0;

        for( uint32_t seed = 0; seed < 256; seed++ )
        {
            if( bSaveNoise )
                printf( "Seed: %d\n", seed );
            OpenSimplexNoise noise( seed );
            generate( noise , values ); // generate float array
            quantize( values, texels ); // convert to 24-bit grayscale

            if( bSaveNoise )
            {
                char filename[ 64 ];
                sprintf( filename, "simplex_noise_seed_%d.tga", seed );
                Targa_Save( filename, WIDTH, HEIGHT, texels, 24 );
            }

            samples += (WIDTH * HEIGHT);
        }

        timer.Stop();
        timer.Throughput( samples );

        // Throughput = Total samples / Time
        printf( "Simplex Samples: %d,  %.f seconds = %d:%d, %d %cnoise/s\n"
            , (uint32_t)samples
            , timer.elapsed
            , timer.mins
            , timer.secs
            , (uint32_t)timer.throughput.per_sec
            , timer.throughput.prefix
        );
    }

    delete [] texels;
    delete [] values;

    return 0;
}
