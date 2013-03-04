//========================================================================
// Multithreading benchmark program, based on the GLFW multi threading
// support.
//
// This program can be used to get an idea of what to expect in terms of
// multithreading granularity performance.
//
// As a "bonus", this program demonstrates how to create a signal
// primitive using the GLFW mutex and condition variable primitives.
//
// Here are some benchmark results:
// (Note: these are not exact measurments, since they are subject to
// varying CPU-loads etc. Some tested systems are multi-user systems
// which were running under anything but optimal conditions)
//
// +------------+-------+-------------+-------------------+------------+
// | Processor  |  CPUs |     OS      | Context switches  | Mean sleep |
// |            |       |             |    per second     | time (ms)  |
// +------------+-------+-------------+-------------------+------------+
// |Athlon      |   1   | Linux       |       161942      |   20.000   |
// |710 MHz     |       | 2.4.3       |                   |            |
// +------------+-------+-------------+-------------------+------------+
// |Athlon      |   1   | MS Win2k    |       525230      |   10.014   |
// |710 MHz     |       |             |                   |            |
// +------------+-------+-------------+-------------------+------------+
// |Athlon      |   1   | MS Win 98   |        23564      |    4.947   |
// |710 MHz     |       |             |                   |            |
// +------------+-------+-------------+-------------------+------------+
// |Pentium III |   1   | MS NT 4.0   |       304694      |   10.014   |
// |500 MHz     |       |             |                   |            |
// +------------+-------+-------------+-------------------+------------+
// |UltraSPARC2 |   6   | SunOS 5.6   |       120867      |   19.355   |
// |400 MHz     |       |             |                   |            |
// +------------+-------+-------------+-------------------+------------+
// |Alpha 21264 |   1   | OSF1        |       131993      |    3.097   |
// |500 MHz     |       |             |                   |            |
// +------------+-------+-------------+-------------------+------------+
// |Alpha 21264 |   2   | OSF1        |        40836      |    1.397   |
// |500 MHz     |       |             |                   |            |
// +------------+-------+-------------+-------------------+------------+
// |68020 (emu) |   1   | AmigaOS 3.1 |        50425      |   40.060   |
// |~200 MHz    |       | (WinUAE)    |                   |            |
// +------------+-------+-------------+-------------------+------------+
//
//========================================================================

#include <stdio.h>
#include <GL/glfw.h>


typedef struct {
    GLFWcond  cond;
    GLFWmutex mutex;
    int       flag;
} signal_t;


signal_t gotoA, gotoB;

GLFWcond   threadDone;
GLFWmutex  doneMutex;
int        doneCount;
int        gotoACount, gotoBCount;

#define MAX_COUNT 10000


//------------------------------------------------------------------------
// InitSignal()
//------------------------------------------------------------------------

void InitSignal( signal_t *s )
{
    s->cond  = glfwCreateCond();
    s->mutex = glfwCreateMutex();
    s->flag  = 0;
}


//------------------------------------------------------------------------
// KillSignal()
//------------------------------------------------------------------------

void KillSignal( signal_t *s )
{
    glfwDestroyCond( s->cond );
    glfwDestroyMutex( s->mutex );
    s->flag  = 0;
}


//------------------------------------------------------------------------
// WaitSignal()
//------------------------------------------------------------------------

void WaitSignal( signal_t *s )
{
    glfwLockMutex( s->mutex );
    while( !s->flag )
    {
        glfwWaitCond( s->cond, s->mutex, GLFW_INFINITY );
    }
    s->flag = 0;
    glfwUnlockMutex( s->mutex );
}


//------------------------------------------------------------------------
// SetSignal()
//------------------------------------------------------------------------

void SetSignal( signal_t *s )
{
    glfwLockMutex( s->mutex );
    s->flag = 1;
    glfwUnlockMutex( s->mutex );
    glfwSignalCond( s->cond );
}


//------------------------------------------------------------------------
// threadAfun()
//------------------------------------------------------------------------

void GLFWCALL threadAfun( void * arg )
{
    int done;

    do
    {
        done = (gotoACount >= MAX_COUNT);
        if( !done )
        {
            gotoACount ++;
            SetSignal( &gotoB );
            WaitSignal( &gotoA );
        }
    }
    while( !done );

    SetSignal( &gotoB );

    glfwLockMutex( doneMutex );
    doneCount ++;
    glfwUnlockMutex( doneMutex );
    glfwSignalCond( threadDone );
}


//------------------------------------------------------------------------
// threadBfun()
//------------------------------------------------------------------------

void GLFWCALL threadBfun( void * arg )
{
    int done;

    do
    {
        done = (gotoBCount >= MAX_COUNT);
        if( !done )
        {
            gotoBCount ++;
            SetSignal( &gotoA );
            WaitSignal( &gotoB );
        }
    }
    while( !done );

    SetSignal( &gotoA );

    glfwLockMutex( doneMutex );
    doneCount ++;
    glfwUnlockMutex( doneMutex );
    glfwSignalCond( threadDone );
}



//------------------------------------------------------------------------
// main()
//------------------------------------------------------------------------

int main( void )
{
    GLFWthread threadA, threadB;
    double     t1, t2, csps;
    int        done, count, i;

    gotoACount = gotoBCount = doneCount = 0;

    // Initialize GLFW
    if( !glfwInit() )
    {
        return 0;
    }

    // Print some program information
    printf( "\nMultithreading benchmarking program\n" );
    printf( "-----------------------------------\n\n" );
    printf( "This program consists of two tests. In the first test " );
    printf( "two threads are created,\n" );
    printf( "which continously signal/wait each other. This forces " );
    printf( "the execution to\n" );
    printf( "alternate between the two threads, and gives a measure " );
    printf( "of the thread\n" );
    printf( "synchronization granularity. In the second test, the " );
    printf( "main thread is repeatedly\n" );
    printf( "put to sleep for a very short interval using glfwSleep. " );
    printf( "The average sleep time\n" );
    printf( "is measured, which tells the minimum supported sleep " );
    printf( "interval.\n\n" );
    printf( "Results:\n" );
    printf( "--------\n\n" );
    printf( "Number of CPUs: %d\n\n", glfwGetNumberOfProcessors() );
    fflush( stdout );


//------------------------------------------------------------------------
// 1) Benchmark thread synchronization granularity
//------------------------------------------------------------------------

    // Init mutexes and conditions
    doneMutex  = glfwCreateMutex();
    threadDone = glfwCreateCond();
    InitSignal( &gotoA );
    InitSignal( &gotoB );

    // Create threads A & B
    threadA = glfwCreateThread( threadAfun, NULL );
    threadB = glfwCreateThread( threadBfun, NULL );
    if( threadA == -1 || threadB == -1 )
    {
        glfwLockMutex( doneMutex );
        doneCount = 2;
        glfwUnlockMutex( doneMutex );
    }

    // Wait for both threads to be done
    t1 = glfwGetTime();
    glfwLockMutex( doneMutex );
    do
    {
        done = (doneCount == 2);
        if( !done )
        {
            glfwWaitCond( threadDone, doneMutex, GLFW_INFINITY );
        }
    }
    while( !done );
    glfwUnlockMutex( doneMutex );
    t2 = glfwGetTime();

    // Display results
    count = gotoACount + gotoBCount;
    csps = (double)count / (t2-t1);
    printf( "Test 1:  %.0f context switches / second (%.3f us/switch)\n",
            csps, 1e6/csps );
    fflush( stdout );

    // Wait for threads to die
    glfwWaitThread( threadA, GLFW_WAIT );
    glfwWaitThread( threadB, GLFW_WAIT );

    // Destroy mutexes and conditions
    glfwDestroyMutex( doneMutex );
    glfwDestroyCond( threadDone );
    KillSignal( &gotoA );
    KillSignal( &gotoB );


//------------------------------------------------------------------------
// 2) Benchmark thread sleep granularity
//------------------------------------------------------------------------

    // Find an initial estimate
    t1 = glfwGetTime();
    for( i = 0; i < 10; i ++ )
    {
        glfwSleep( 0.0001 );
    }
    t2 = glfwGetTime();

    // Sleep for roughly 1 s
    count = (int)(1.0 / ((t2-t1)/10.0));
    t1 = glfwGetTime();
    for( i = 0; i < count; i ++ )
    {
        glfwSleep( 0.0001 );
    }
    t2 = glfwGetTime();

    // Display results
    printf( "Test 2:  %.3f ms / sleep (mean)\n\n",
            1000.0 * (t2-t1) / (double)count );

    // Terminate GLFW
    glfwTerminate();

    return 0;
}
