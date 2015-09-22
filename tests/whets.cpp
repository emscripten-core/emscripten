/*  gcc whets.c cpuidc64.o cpuida64.o -m64 -lrt -lc -lm -o whet
*
*  XXX modified by emscripten to be slower, to not slow down test runner
*
*  Document:         Whets.c 
*  File Group:       Classic Benchmarks
*  Creation Date:    6 November 1996
*  Revision Date:    6 November 2010 Ubuntu Version for PCs
*
*  Title:            Whetstone Benchmark in C/C++
*  Keywords:         WHETSTONE BENCHMARK PERFORMANCE MIPS
*                    MWIPS MFLOPS
*
*  Abstract:         C or C++ version of Whetstone one of the
*                    Classic Numeric Benchmarks with example
*                    results on P3 to P6 based PCs.        
*
*  Contributor:      roy@roylongbottom.org.uk
*
************************************************************
*
*     C/C++ Whetstone Benchmark Single or Double Precision
*
*     Original concept        Brian Wichmann NPL      1960's
*     Original author         Harold Curnow  CCTA     1972
*     Self timing versions    Roy Longbottom CCTA     1978/87
*     Optimisation control    Bangor University       1987/90
*     C/C++ Version           Roy Longbottom          1996
*     Compatibility & timers  Al Aburto               1996
*
************************************************************
*
*              Official version approved by:
*
*         Harold Curnow  100421.1615@compuserve.com
*
*      Happy 25th birthday Whetstone, 21 November 1997
*
************************************************************
*
*     The program normally runs for about 100 seconds
*     (adjustable in main - variable duration). This time
*     is necessary because of poor PC clock resolution.
*     The original concept included such things as a given
*     number of subroutine calls and divides which may be
*     changed by optimisation. For comparison purposes the
*     compiler and level of optimisation should be identified.
*
*     This version is set to run for 10 seconds using high
*     resolution timer.
*       
************************************************************
*
*     The original benchmark had a single variable I which
*     controlled the running time. Constants with values up
*     to 899 were multiplied by I to control the number
*     passes for each loop. It was found that large values
*     of I could overflow index registers so an extra outer
*     loop with a second variable J was added.
*
*     Self timing versions were produced during the early
*     days. The 1978 changes supplied timings of individual
*     loops and these were used later to produce MFLOPS and
*     MOPS ratings.
*
*     1987 changes converted the benchmark to Fortran 77
*     standards and removed redundant IF statements and
*     loops to leave the 8 active loops N1 to N8. Procedure
*     P3 was changed to use global variables to avoid over-
*     optimisation with the first two statements changed from
*     X1=X and Y1=Y to X=Y and Y=Z. A self time calibrating
*     version for PCs was also produced, the facility being
*     incorporated in this version.
*
*     This version has changes to avoid worse than expected
*     speed ratings, due to underflow, and facilities to show
*     that consistent numeric output is produced with varying
*     optimisation levels or versions in different languages.
*
*     Some of the procedures produce ever decreasing numbers.
*     To avoid problems, variables T and T1 have been changed
*     from 0.499975 and 0.50025 to 0.49999975 and 0.50000025.
*
*     Each section now has its own double loop. Inner loops
*     are run 100 times the loop constants. Calibration
*     determines the number of outer loop passes. The
*     numeric results produced in the main output are for
*     one pass on the outer loop. As underflow problems were
*     still likely on a processor 100 times faster than a 100
*     MHz Pentium, three sections have T=1.0-T inserted in the
*     outer loop to avoid the problem. The two loops avoid
*     index register overflows.
*
*     The first section is run ten times longer than required
*     for accuracy in calculating MFLOPS. This time is divided
*     by ten for inclusion in the MWIPS calculations.
*
*     Early version has facilities for typing in details of
*     the particular run, appended to file whets.txt along
*     with the results. This version attemps to obtain these
*     automatically.
*
*     2010 Section 4 modified slightly to avoid over optimisation
*     by GCC compiler
*
*     Roy Longbottom  roy@roylongbottom.org.uk
*
************************************************************
*
*     Whetstone benchmark results, further details of the
*     benchmarks and history are available from:
*
*     http://www.roylongbottom.org.uk/whetstone%20results.htm
*     http://www.roylongbottom.org.uk/whetstone.htm
*
************************************************************
*
*     Source code is available in C/C++, Fortran, Basic and
*     Visual Basic in the same format as this version. Pre-
*     compiled versions for PCs are also available via C++.
*     These comprise optimised and non-optimised versions
*     for DOS, Windows and NT. See:
*
*     http://www.roylongbottom.org.uk/whetstone%20results.htm
*
************************************************************
*
* Example of initial calibration display (Pentium 100 MHz)
*
* Single Precision C/C++ Whetstone Benchmark
*
* Calibrate
*      0.17 Seconds          1   Passes (x 100)
*      0.77 Seconds          5   Passes (x 100)
*      3.70 Seconds         25   Passes (x 100)
*
* Use 676  passes (x 100)
*
* 676 passes are used for an approximate duration of 100
* seconds, providing an initial estimate of a speed rating
* of 67.6 MWIPS.
*
* This is followed by the table of results as below.

* Whetstone Single  Precision Benchmark in C/C++
*
* Loop content                 Result            MFLOPS     MOPS   Seconds
*
* N1 floating point    -1.12475025653839100      19.971              0.274
* N2 floating point    -1.12274754047393800      11.822              3.240
* N3 if then else       1.00000000000000000               11.659     2.530
* N4 fixed point       12.00000000000000000               13.962     6.430
* N5 sin,cos etc.       0.49904659390449520                2.097    11.310
* N6 floating point     0.99999988079071040       3.360             45.750
* N7 assignments        3.00000000000000000                2.415    21.810
* N8 exp,sqrt etc.      0.75110864639282230                1.206     8.790
*
* MWIPS                                          28.462            100.134
*
*  Note different numeric results to single precision. Slight variations
*  are normal with different compilers and sometimes optimisation levels. 
*
**************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1
#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif

#include <math.h>       /* for sin, exp etc.           */
#include <stdio.h>      /* standard I/O                */ 
#include <string.h>     /* for strcpy - 3 occurrences  */
#include <stdlib.h>     /* for exit   - 1 occurrence   */
#include <time.h> 

/*  #include "cpuidh.h" */

/*PRECISION PRECISION PRECISION PRECISION PRECISION PRECISION PRECISION*/

/* #define DP */

#ifdef DP 
#define SPDP double
#define Precision "Double"
#else
#define SPDP float
#define Precision "Single"
#endif

#define opt "Opt 3 64 Bit"

void whetstones(long xtra, long x100, int calibrate);  
void pa(SPDP e[4], SPDP t, SPDP t2);
void po(SPDP e1[4], long j, long k, long l);
void p3(SPDP *x, SPDP *y, SPDP *z, SPDP t, SPDP t1, SPDP t2);
void pout(const char title[22], float ops, int type, SPDP checknum,
  	  SPDP time, int calibrate, int section);


static SPDP loop_time[9];
static SPDP loop_mops[9];
static SPDP loop_mflops[9];
static SPDP TimeUsed;
static SPDP mwips;
static char headings[9][18];
static SPDP Check;
static SPDP results[9];

/* this is truly rank, but it's minimally invasive, and lifted in part from the STREAM scores */

static double secs;

#ifndef WIN32

double mysecond()
{
        struct timeval tp;
        struct timezone tzp;
        int i;

        i = gettimeofday(&tp,&tzp);
        return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}
#else

double mysecond()
{
	static LARGE_INTEGER freq = {0};
	LARGE_INTEGER count = {0};
	if(freq.QuadPart == 0LL) {
		QueryPerformanceFrequency(&freq);
	}
	QueryPerformanceCounter(&count);
	return (double)count.QuadPart / (double)freq.QuadPart;
}

#endif

void start_time()
{
	secs = mysecond();
}

void end_time()
{
	secs = mysecond() - secs;
}

int main(int argc, char *argv[])
{
	int count = 1, calibrate = 1;
	long xtra = 1;
	int section;
	long x100 = 1;
	int duration = 1;
	char compiler[80], options[256], general[10][80] = {" "};
	char endit[80];
	int i;

	printf("\n");
	printf("##########################################\n");
	{
		time_t t;
		char timeday[30];
		t = time(NULL);
		sprintf(timeday, "%s", asctime(localtime(&t)));

		printf("%s Precision C Whetstone Benchmark %s, %s\n", Precision, opt, timeday);
	}

	printf("Calibrate\n");
	do
	{
		TimeUsed=0;

		whetstones(xtra,x100,calibrate);

		printf("%11.2f Seconds %10.0lf   Passes (x 100)\n",
			TimeUsed,(SPDP)(xtra));
		calibrate++;
		count--;

		if (TimeUsed > 2.0)
		{
			count = 0;
		}
		else
		{
			xtra = xtra * 5;
		}
	}

	while (count > 0);

	if (TimeUsed > 0) xtra = (long)((SPDP)(duration * xtra) / TimeUsed);
	if (xtra < 1) xtra = 1;

	calibrate = 0;

	printf("\nUse %d  passes (x 100)\n", (int)xtra);

	printf("\n          %s Precision C/C++ Whetstone Benchmark",Precision);

#ifdef PRECOMP
	printf("\n          Compiler  %s", precompiler);
	printf("\n          Options   %s\n", preoptions);
#else
	printf("\n");
#endif

	printf("\nLoop content                  Result              MFLOPS "
		"     MOPS   Seconds\n\n");

	TimeUsed=0;
	whetstones(xtra,x100,calibrate);

	printf("\nMWIPS            ");
	if (TimeUsed>0)
	{
		mwips=(float)(xtra) * (float)(x100) / (10 * TimeUsed);
	}
	else
	{
		mwips = 0;
	}  

	printf("%39.3f%19.3f\n\n",mwips,TimeUsed);

	if (Check == 0) printf("Wrong answer  ");

	printf ("\n");
	printf ("A new results file, whets.txt,  will have been created in the same\n");
	printf ("directory as the .EXE files, if one did not already exist.\n\n");

	return 0;             
}

void whetstones(long xtra, long x100, int calibrate)
{

	long n1,n2,n3,n4,n5,n6,n7,n8,i,ix,n1mult;
	SPDP x,y,z;              
	long j,k,l;
	SPDP e1[4];

	SPDP t =  0.49999975;
	SPDP t0 = t;        
	SPDP t1 = 0.50000025;
	SPDP t2 = 2.0;

	Check=0.0;

	n1 = 1*x100;
	n2 = 1*x100;
	n3 = 3*x100;
	n4 = 2*x100;
	n5 = 3*x100;
	n6 = 8*x100;
	n7 = 6*x100;
	n8 = 9*x100;
	n1mult = 1;

	/* Section 1, Array elements */

	e1[0] = 1.0;
	e1[1] = -1.0;
	e1[2] = -1.0;
	e1[3] = -1.0;
	start_time();
	{
		for (ix=0; ix<xtra; ix++)
		{
			for(i=0; i<n1*n1mult; i++)
			{
				e1[0] = (e1[0] + e1[1] + e1[2] - e1[3]) * t;
				e1[1] = (e1[0] + e1[1] - e1[2] + e1[3]) * t;
				e1[2] = (e1[0] - e1[1] + e1[2] + e1[3]) * t;
				e1[3] = (-e1[0] + e1[1] + e1[2] + e1[3]) * t;
			}
			t = 1.0 - t;
		}
		t =  t0;                    
	}
	end_time();
	secs = secs/(SPDP)(n1mult);
	pout("N1 floating point\0",(float)(n1*16)*(float)(xtra),
		1,e1[3],secs,calibrate,1);

	/* Section 2, Array as parameter */

	start_time();
	{
		for (ix=0; ix<xtra; ix++)
		{ 
			for(i=0; i<n2; i++)
			{
				pa(e1,t,t2);
			}
			t = 1.0 - t;
		}
		t =  t0;
	}
	end_time();
	pout("N2 floating point\0",(float)(n2*96)*(float)(xtra),
		1,e1[3],secs,calibrate,2);

	/* Section 3, Conditional jumps */
	j = 1;
	start_time();
	{
		for (ix=0; ix<xtra; ix++)
		{
			for(i=0; i<n3; i++)
			{
				if(j==1)       j = 2;
				else           j = 3;
				if(j>2)        j = 0;
				else           j = 1;
				if(j<1)        j = 1;
				else           j = 0;
			}
		}
	}
	end_time();
	pout("N3 if then else  \0",(float)(n3*3)*(float)(xtra),
		2,(SPDP)(j),secs,calibrate,3);

	/* Section 4, Integer arithmetic */
	j = 1;
	k = 2;
	l = 3;
	e1[0] = 0.0;
	e1[1] = 0.0;
	start_time();
	{
		for (ix=0; ix<xtra; ix++)
		{
			for(i=0; i<n4; i++)
			{
				j = j *(k-j)*(l-k);
				k = l * k - (l-j) * k;
				l = (l-k) * (k+j);
				e1[l-2] = e1[l-2] + j + k + l;
				e1[k-2] = e1[k-2] + j * k * l;
				//  was          e1[l-2] = j + k + l; and  e1[k-2] = j * k * l;
			}
		}
	}
	end_time();
	x = (e1[0]+e1[1])/(SPDP)n4/(SPDP)xtra;   // was x = e1[0]+e1[1];
	pout("N4 fixed point   \0",(float)(n4*15)*(float)(xtra),
		2,x,secs,calibrate,4);

	/* Section 5, Trig functions */
	x = 0.5;
	y = 0.5;
	start_time();
	{
		for (ix=0; ix<xtra; ix++)
		{
			for(i=1; i<n5; i++)
			{
				x = t*atan(t2*sin(x)*cos(x)/(cos(x+y)+cos(x-y)-1.0));
				y = t*atan(t2*sin(y)*cos(y)/(cos(x+y)+cos(x-y)-1.0));
			}
			t = 1.0 - t;
		}
		t = t0;
	}
	end_time();
	pout("N5 sin,cos etc.  \0",(float)(n5*26)*(float)(xtra),
		2,y,secs,calibrate,5);


	/* Section 6, Procedure calls */
	x = 1.0;

	y = 1.0;
	z = 1.0;
	start_time();
	{
		for (ix=0; ix<xtra; ix++)
		{
			for(i=0; i<n6; i++)
			{
				p3(&x,&y,&z,t,t1,t2);
			}
		}
	}
	end_time();
	pout("N6 floating point\0",(float)(n6*6)*(float)(xtra),
		1,z,secs,calibrate,6);

	/* Section 7, Array refrences */
	j = 0;
	k = 1;
	l = 2;
	e1[0] = 1.0;
	e1[1] = 2.0;
	e1[2] = 3.0;
	start_time();
	{
		for (ix=0; ix<xtra; ix++)
		{
			for(i=0;i<n7;i++)
			{
				po(e1,j,k,l);
			}
		}
	}
	end_time();
	pout("N7 assignments   \0",(float)(n7*3)*(float)(xtra),
		2,e1[2],secs,calibrate,7);

	/* Section 8, Standard functions */
	x = 0.75;
	start_time();
	{
		for (ix=0; ix<xtra; ix++)
		{
			for(i=0; i<n8; i++)
			{
				x = sqrt(exp(log(x)/t1));
			}
		}
	}
	end_time();
	pout("N8 exp,sqrt etc. \0",(float)(n8*4)*(float)(xtra),
		2,x,secs,calibrate,8);

	return;
}


void pa(SPDP e[4], SPDP t, SPDP t2)
{
	long j;
	for(j=0;j<6;j++)
	{
		e[0] = (e[0]+e[1]+e[2]-e[3])*t;
		e[1] = (e[0]+e[1]-e[2]+e[3])*t;
		e[2] = (e[0]-e[1]+e[2]+e[3])*t;
		e[3] = (-e[0]+e[1]+e[2]+e[3])/t2;
	}

	return;
}

void po(SPDP e1[4], long j, long k, long l)
{
	e1[j] = e1[k];
	e1[k] = e1[l];
	e1[l] = e1[j];
	return;
}

void p3(SPDP *x, SPDP *y, SPDP *z, SPDP t, SPDP t1, SPDP t2)
{
	*x = *y;
	*y = *z;
	*x = t * (*x + *y);
	*y = t1 * (*x + *y);
	*z = (*x + *y)/t2;
	return;
}


void pout(char title[18], float ops, int type, SPDP checknum,
		  SPDP time, int calibrate, int section)
{
	SPDP mops,mflops;

	Check = Check + checknum;
	loop_time[section] = time;
	strcpy (headings[section],title);
	TimeUsed =  TimeUsed + time;
	if (calibrate == 1)

	{
		results[section] = checknum;
	}
	if (calibrate == 0)
	{              
		printf("%s %24.17f    ",headings[section],results[section]);    

		if (type == 1)
		{
			if (time>0)
			{
				mflops = ops/(1000000L*time);
			}
			else
			{
				mflops = 0;
			}
			loop_mops[section] = 99999;
			loop_mflops[section] = mflops;
			printf(" %9.3f          %9.3f\n",
				loop_mflops[section], loop_time[section]);                
		}
		else
		{
			if (time>0)
			{
				mops = ops/(1000000L*time);
			}
			else
			{
				mops = 0;
			}
			loop_mops[section] = mops;
			loop_mflops[section] = 0;                 
			printf("           %9.3f%9.3f\n",
				loop_mops[section], loop_time[section]);
		}
	}

	return;
}

