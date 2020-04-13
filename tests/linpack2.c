/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

# include <stdlib.h>
# include <stdio.h>
# include <math.h>
# include <time.h>

double cpu_time ( );
void daxpy ( int n, double da, double dx[], int incx, double dy[], int incy );
double ddot ( int n, double dx[], int incx, double dy[], int incy );
int dgefa ( double a[], int lda, int n, int ipvt[] );
void dgesl ( double a[], int lda, int n, int ipvt[], double b[], int job );
void dscal ( int n, double sa, double x[], int incx );
int idamax ( int n, double dx[], int incx );
double r8_abs ( double x );
double r8_epsilon ( );
double r8_max ( double x, double y );
double r8_random ( int iseed[4] );
double *r8mat_gen ( int lda, int n );
void timestamp ( );

/******************************************************************************/

int main(int argc, char **argv) {

/******************************************************************************/
/*
  Purpose:

    MAIN is the main program for LINPACK_BENCH.

  Discussion:

    LINPACK_BENCH drives the double precision LINPACK benchmark program.

  Modified:

    25 July 2008

  Parameters:

    N is the problem size.
*/
# define N 2000
# define LDA ( N + 1 )

  double *a;
  double a_max;
  double *b;
  double b_max;
  double cray = 0.056;
  double eps;
  int i;
  int info;
  int *ipvt;
  int j;
  int job;
  double ops;
  double *resid;
  double resid_max;
  double residn;
  double *rhs;
  double t1;
  double t2;
  double time[6];
  double total;
  double *x;

  int arg = argc > 1 ? argv[1][0] - '0' : 3;
  if (arg == 0) return 0;

  timestamp ( );
  printf ( "\n" );
  printf ( "LINPACK_BENCH\n" );
  printf ( "  C version\n" );
  printf ( "\n" );
  printf ( "  The LINPACK benchmark.\n" );
  printf ( "  Language: C\n" );
  printf ( "  Datatype: Double precision real\n" );
  printf ( "  Matrix order N               = %d\n", N );
  printf ( "  Leading matrix dimension LDA = %d\n", LDA );

  ops = ( double ) ( 2.0 * N * N * N ) / 3.0 + 2.0 * ( double ) ( N * N );
/*
  Allocate space for arrays.
*/
  a = r8mat_gen ( LDA, N );
  b = ( double * ) malloc ( N * sizeof ( double ) );
  ipvt = ( int * ) malloc ( N * sizeof ( int ) );
  resid = ( double * ) malloc ( N * sizeof ( double ) );
  rhs = ( double * ) malloc ( N * sizeof ( double ) );
  x = ( double * ) malloc ( N * sizeof ( double ) );

  a_max = 0.0;
  for ( j = 0; j < N; j++ )
  {
    for ( i = 0; i < N; i++ )
    {
      a_max = r8_max ( a_max, a[i+j*LDA] );
    }
  }

  for ( i = 0; i < N; i++ )
  {
    x[i] = 1.0;
  }

  for ( i = 0; i < N; i++ )
  {
    b[i] = 0.0;
    for ( j = 0; j < N; j++ )
    {
      b[i] = b[i] + a[i+j*LDA] * x[j];
    }
  }
  t1 = cpu_time ( );

  info = dgefa ( a, LDA, N, ipvt );

  if ( info != 0 )
  {
    printf ( "\n" );
    printf ( "LINPACK_BENCH - Fatal error!\n" );
    printf ( "  The matrix A is apparently singular.\n" );
    printf ( "  Abnormal end of execution.\n" );
    return 1;
  }

  t2 = cpu_time ( );
  time[0] = t2 - t1;

  t1 = cpu_time ( );

  job = 0;
  dgesl ( a, LDA, N, ipvt, b, job );

  t2 = cpu_time ( );
  time[1] = t2 - t1;

  total = time[0] + time[1];

  free ( a );
/*
  Compute a residual to verify results.
*/
  a = r8mat_gen ( LDA, N );

  for ( i = 0; i < N; i++ )
  {
    x[i] = 1.0;
  }

  for ( i = 0; i < N; i++ )
  {
    rhs[i] = 0.0;
    for ( j = 0; j < N; j++ )
    {
      rhs[i] = rhs[i] + a[i+j*LDA] * x[j];
    }
  }

  for ( i = 0; i < N; i++ )
  {
    resid[i] = -rhs[i];
    for ( j = 0; j < N; j++ )
    {
      resid[i] = resid[i] + a[i+j*LDA] * b[j];
    }
  }

  resid_max = 0.0;
  for ( i = 0; i < N; i++ )
  {
    resid_max = r8_max ( resid_max, r8_abs ( resid[i] ) );
  }

  b_max = 0.0;
  for ( i = 0; i < N; i++ )
  {
    b_max = r8_max ( b_max, r8_abs ( b[i] ) );
  }

  eps = r8_epsilon ( );

  residn = resid_max / ( double ) N / a_max / b_max / eps;

  time[2] = total;
  if ( 0.0 < total )
  {
    time[3] = ops / ( 1.0E+06 * total );
  }
  else
  {
    time[3] = -1.0;
  }
  time[4] = 2.0 / time[3];
  time[5] = total / cray;

  printf ( "\n" );
  printf ( "     Norm. Resid      Resid           MACHEP         X[1]          X[N]\n" );
  printf ( "\n" );
  printf ( "  %14f  %14f  %14e  %14f  %14f\n", residn, resid_max, eps, b[0], b[N-1] );
  printf ( "\n" );
  printf ( "      Factor     Solve      Total            Unit      Cray-Ratio\n" );
  printf ( "\n" );
  printf ( "  %9f  %9f  %9f  %9f  %9f\n", 
    time[0], time[1], time[2], time[4], time[5] );
  printf ( "\n" );
  printf ( "Unrolled Double  Precision %9f Mflops\n", time[3]);
  printf ( "\n" );

  free ( a );
  free ( b );
  free ( ipvt );
  free ( resid );
  free ( rhs );
  free ( x );
/*
  Terminate.
*/
  printf ( "\n" );
  printf ( "LINPACK_BENCH\n" );
  printf ( "  Normal end of execution.\n" );

  printf ( "\n" );
  timestamp ( );

  return 0;
# undef LDA
# undef N
}
/******************************************************************************/

double cpu_time ( void )

/******************************************************************************/
/*
  Purpose:

    CPU_TIME returns the current reading on the CPU clock.

  Discussion:

    The CPU time measurements available through this routine are often
    not very accurate.  In some cases, the accuracy is no better than
    a hundredth of a second.  

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    06 June 2005

  Author:

    John Burkardt

  Parameters:

    Output, double CPU_TIME, the current reading of the CPU clock, in seconds.
*/
{
  double value;

  value = ( double ) clock ( ) 
        / ( double ) CLOCKS_PER_SEC;

  return value;
}
/******************************************************************************/

void daxpy ( int n, double da, double dx[], int incx, double dy[], int incy )

/******************************************************************************/
/*
  Purpose:

    DAXPY computes constant times a vector plus a vector.

  Discussion:

    This routine uses unrolled loops for increments equal to one.

  Modified:

    30 March 2007

  Author:

    FORTRAN77 original by Jack Dongarra, Cleve Moler, Jim Bunch, Pete Stewart.
    C version by John Burkardt

  Reference:

    Jack Dongarra, Cleve Moler, Jim Bunch, Pete Stewart,
    LINPACK User's Guide,
    SIAM, 1979.

    Charles Lawson, Richard Hanson, David Kincaid, Fred Krogh,
    Basic Linear Algebra Subprograms for Fortran Usage,
    Algorithm 539, 
    ACM Transactions on Mathematical Software, 
    Volume 5, Number 3, September 1979, pages 308-323.

  Parameters:

    Input, int N, the number of elements in DX and DY.

    Input, double DA, the multiplier of DX.

    Input, double DX[*], the first vector.

    Input, int INCX, the increment between successive entries of DX.

    Input/output, double DY[*], the second vector.
    On output, DY[*] has been replaced by DY[*] + DA * DX[*].

    Input, int INCY, the increment between successive entries of DY.
*/
{
  int i;
  int ix;
  int iy;
  int m;

  if ( n <= 0 )
  {
    return;
  }

  if ( da == 0.0 )
  {
    return;
  }
/*
  Code for unequal increments or equal increments
  not equal to 1.
*/
  if ( incx != 1 || incy != 1 )
  {
    if ( 0 <= incx )
    {
      ix = 0;
    }
    else
    {
      ix = ( - n + 1 ) * incx;
    }

    if ( 0 <= incy )
    {
      iy = 0;
    }
    else
    {
      iy = ( - n + 1 ) * incy;
    }

    for ( i = 0; i < n; i++ )
    {
      dy[iy] = dy[iy] + da * dx[ix];
      ix = ix + incx;
      iy = iy + incy;
    }
  }
/*
  Code for both increments equal to 1.
*/
  else
  {
    m = n % 4;

    for ( i = 0; i < m; i++ )
    {
      dy[i] = dy[i] + da * dx[i];
    }

    for ( i = m; i < n; i = i + 4 )
    {
      dy[i  ] = dy[i  ] + da * dx[i  ];
      dy[i+1] = dy[i+1] + da * dx[i+1];
      dy[i+2] = dy[i+2] + da * dx[i+2];
      dy[i+3] = dy[i+3] + da * dx[i+3];
    }
  }
  return;
}
/******************************************************************************/

double ddot ( int n, double dx[], int incx, double dy[], int incy )

/******************************************************************************/
/*
  Purpose:

    DDOT forms the dot product of two vectors.

  Discussion:

    This routine uses unrolled loops for increments equal to one.

  Modified:

    30 March 2007

  Author:

    FORTRAN77 original by Jack Dongarra, Cleve Moler, Jim Bunch, Pete Stewart.
    C version by John Burkardt

  Reference:

    Jack Dongarra, Cleve Moler, Jim Bunch, Pete Stewart,
    LINPACK User's Guide,
    SIAM, 1979.

    Charles Lawson, Richard Hanson, David Kincaid, Fred Krogh,
    Basic Linear Algebra Subprograms for Fortran Usage,
    Algorithm 539, 
    ACM Transactions on Mathematical Software, 
    Volume 5, Number 3, September 1979, pages 308-323.

  Parameters:

    Input, int N, the number of entries in the vectors.

    Input, double DX[*], the first vector.

    Input, int INCX, the increment between successive entries in DX.

    Input, double DY[*], the second vector.

    Input, int INCY, the increment between successive entries in DY.

    Output, double DDOT, the sum of the product of the corresponding
    entries of DX and DY.
*/
{
  double dtemp;
  int i;
  int ix;
  int iy;
  int m;

  dtemp = 0.0;

  if ( n <= 0 )
  {
    return dtemp;
  }
/*
  Code for unequal increments or equal increments
  not equal to 1.
*/
  if ( incx != 1 || incy != 1 )
  {
    if ( 0 <= incx )
    {
      ix = 0;
    }
    else
    {
      ix = ( - n + 1 ) * incx;
    }

    if ( 0 <= incy )
    {
      iy = 0;
    }
    else
    {
      iy = ( - n + 1 ) * incy;
    }

    for ( i = 0; i < n; i++ )
    {
      dtemp = dtemp + dx[ix] * dy[iy];
      ix = ix + incx;
      iy = iy + incy;
    }
  }
/*
  Code for both increments equal to 1.
*/
  else
  {
    m = n % 5;

    for ( i = 0; i < m; i++ )
    {
      dtemp = dtemp + dx[i] * dy[i];
    }

    for ( i = m; i < n; i = i + 5 )
    {
      dtemp = dtemp + dx[i  ] * dy[i  ] 
                    + dx[i+1] * dy[i+1] 
                    + dx[i+2] * dy[i+2] 
                    + dx[i+3] * dy[i+3] 
                    + dx[i+4] * dy[i+4];
    }
  }
  return dtemp;
}
/******************************************************************************/

int dgefa ( double a[], int lda, int n, int ipvt[] )

/******************************************************************************/
/*
  Purpose:

    DGEFA factors a real general matrix.

  Modified:

    16 May 2005

  Author:

    C version by John Burkardt.

  Reference:

    Jack Dongarra, Cleve Moler, Jim Bunch and Pete Stewart,
    LINPACK User's Guide,
    SIAM, (Society for Industrial and Applied Mathematics),
    3600 University City Science Center,
    Philadelphia, PA, 19104-2688.
    ISBN 0-89871-172-X

  Parameters:

    Input/output, double A[LDA*N].
    On intput, the matrix to be factored.
    On output, an upper triangular matrix and the multipliers used to obtain
    it.  The factorization can be written A=L*U, where L is a product of
    permutation and unit lower triangular matrices, and U is upper triangular.

    Input, int LDA, the leading dimension of A.

    Input, int N, the order of the matrix A.

    Output, int IPVT[N], the pivot indices.

    Output, int DGEFA, singularity indicator.
    0, normal value.
    K, if U(K,K) == 0.  This is not an error condition for this subroutine,
    but it does indicate that DGESL or DGEDI will divide by zero if called.
    Use RCOND in DGECO for a reliable indication of singularity.
*/
{
  int info;
  int j;
  int k;
  int l;
  double t;
/*
  Gaussian elimination with partial pivoting.
*/
  info = 0;

  for ( k = 1; k <= n-1; k++ )
  {
/*
  Find L = pivot index.
*/
    l = idamax ( n-k+1, a+(k-1)+(k-1)*lda, 1 ) + k - 1;
    ipvt[k-1] = l;
/*
  Zero pivot implies this column already triangularized.
*/
    if ( a[l-1+(k-1)*lda] == 0.0 )
    {
      info = k;
      continue;
    }
/*
  Interchange if necessary.
*/
    if ( l != k )
    {
      t = a[l-1+(k-1)*lda];
      a[l-1+(k-1)*lda] = a[k-1+(k-1)*lda];
      a[k-1+(k-1)*lda] = t;
    }
/*
  Compute multipliers.
*/
    t = -1.0 / a[k-1+(k-1)*lda];

    dscal ( n-k, t, a+k+(k-1)*lda, 1 );
/*
  Row elimination with column indexing.
*/
    for ( j = k+1; j <= n; j++ )
    {
      t = a[l-1+(j-1)*lda];
      if ( l != k )
      {
        a[l-1+(j-1)*lda] = a[k-1+(j-1)*lda];
        a[k-1+(j-1)*lda] = t;
      }
      daxpy ( n-k, t, a+k+(k-1)*lda, 1, a+k+(j-1)*lda, 1 );
    }

  }

  ipvt[n-1] = n;

  if ( a[n-1+(n-1)*lda] == 0.0 )
  {
    info = n;
  }

  return info;
}
/******************************************************************************/

void dgesl ( double a[], int lda, int n, int ipvt[], double b[], int job )

/******************************************************************************/
/*
  Purpose:

    DGESL solves a real general linear system A * X = B.

  Discussion:

    DGESL can solve either of the systems A * X = B or A' * X = B.

    The system matrix must have been factored by DGECO or DGEFA.

    A division by zero will occur if the input factor contains a
    zero on the diagonal.  Technically this indicates singularity
    but it is often caused by improper arguments or improper
    setting of LDA.  It will not occur if the subroutines are
    called correctly and if DGECO has set 0.0 < RCOND
    or DGEFA has set INFO == 0.

  Modified:

    16 May 2005

  Author:

    C version by John Burkardt.

  Reference:

    Jack Dongarra, Cleve Moler, Jim Bunch and Pete Stewart,
    LINPACK User's Guide,
    SIAM, (Society for Industrial and Applied Mathematics),
    3600 University City Science Center,
    Philadelphia, PA, 19104-2688.
    ISBN 0-89871-172-X

  Parameters:

    Input, double A[LDA*N], the output from DGECO or DGEFA.

    Input, int LDA, the leading dimension of A.

    Input, int N, the order of the matrix A.

    Input, int IPVT[N], the pivot vector from DGECO or DGEFA.

    Input/output, double B[N].
    On input, the right hand side vector.
    On output, the solution vector.

    Input, int JOB.
    0, solve A * X = B;
    nonzero, solve A' * X = B.
*/
{
  int k;
  int l;
  double t;
/*
  Solve A * X = B.
*/
  if ( job == 0 )
  {
    for ( k = 1; k <= n-1; k++ )
    {
      l = ipvt[k-1];
      t = b[l-1];

      if ( l != k )
      {
        b[l-1] = b[k-1];
        b[k-1] = t;
      }

      daxpy ( n-k, t, a+k+(k-1)*lda, 1, b+k, 1 );

    }

    for ( k = n; 1 <= k; k-- )
    {
      b[k-1] = b[k-1] / a[k-1+(k-1)*lda];
      t = -b[k-1];
      daxpy ( k-1, t, a+0+(k-1)*lda, 1, b, 1 );
    }
  }
/*
  Solve A' * X = B.
*/
  else
  {
    for ( k = 1; k <= n; k++ )
    {
      t = ddot ( k-1, a+0+(k-1)*lda, 1, b, 1 );
      b[k-1] = ( b[k-1] - t ) / a[k-1+(k-1)*lda];
    }

    for ( k = n-1; 1 <= k; k-- )
    {
      b[k-1] = b[k-1] + ddot ( n-k, a+k+(k-1)*lda, 1, b+k, 1 );
      l = ipvt[k-1];

      if ( l != k )
      {
        t = b[l-1];
        b[l-1] = b[k-1];
        b[k-1] = t;
      }
    }
  }
  return;
}
/******************************************************************************/

void dscal ( int n, double sa, double x[], int incx )

/******************************************************************************/
/*
  Purpose:

    DSCAL scales a vector by a constant.

  Modified:

    30 March 2007

  Author:

    FORTRAN77 original by Jack Dongarra, Cleve Moler, Jim Bunch, Pete Stewart.
    C version by John Burkardt

  Reference:

    Jack Dongarra, Cleve Moler, Jim Bunch, Pete Stewart,
    LINPACK User's Guide,
    SIAM, 1979.

    Charles Lawson, Richard Hanson, David Kincaid, Fred Krogh,
    Basic Linear Algebra Subprograms for Fortran Usage,
    Algorithm 539,
    ACM Transactions on Mathematical Software,
    Volume 5, Number 3, September 1979, pages 308-323.

  Parameters:

    Input, int N, the number of entries in the vector.

    Input, double SA, the multiplier.

    Input/output, double X[*], the vector to be scaled.

    Input, int INCX, the increment between successive entries of X.
*/
{
  int i;
  int ix;
  int m;

  if ( n <= 0 )
  {
  }
  else if ( incx == 1 )
  {
    m = n % 5;

    for ( i = 0; i < m; i++ )
    {
      x[i] = sa * x[i];
    }

    for ( i = m; i < n; i = i + 5 )
    {
      x[i]   = sa * x[i];
      x[i+1] = sa * x[i+1];
      x[i+2] = sa * x[i+2];
      x[i+3] = sa * x[i+3];
      x[i+4] = sa * x[i+4];
    }
  }
  else
  {
    if ( 0 <= incx )
    {
      ix = 0;
    }
    else
    {
      ix = ( - n + 1 ) * incx;
    }

    for ( i = 0; i < n; i++ )
    {
      x[ix] = sa * x[ix];
      ix = ix + incx;
    }
  }
  return;
}
/******************************************************************************/

int idamax ( int n, double dx[], int incx )

/******************************************************************************/
/*
  Purpose:

    IDAMAX finds the index of the vector element of maximum absolute value.

  Discussion:

    WARNING: This index is a 1-based index, not a 0-based index!

  Modified:

    30 March 2007

  Author:

    FORTRAN77 original by Jack Dongarra, Cleve Moler, Jim Bunch, Pete Stewart.
    C version by John Burkardt

  Reference:

    Jack Dongarra, Cleve Moler, Jim Bunch, Pete Stewart,
    LINPACK User's Guide,
    SIAM, 1979.

    Charles Lawson, Richard Hanson, David Kincaid, Fred Krogh,
    Basic Linear Algebra Subprograms for Fortran Usage,
    Algorithm 539,
    ACM Transactions on Mathematical Software,
    Volume 5, Number 3, September 1979, pages 308-323.

  Parameters:

    Input, int N, the number of entries in the vector.

    Input, double X[*], the vector to be examined.

    Input, int INCX, the increment between successive entries of SX.

    Output, int IDAMAX, the index of the element of maximum
    absolute value.
*/
{
  double dmax;
  int i;
  int ix;
  int value;

  value = 0;

  if ( n < 1 || incx <= 0 )
  {
    return value;
  }

  value = 1;

  if ( n == 1 )
  {
    return value;
  }

  if ( incx == 1 )
  {
    dmax = r8_abs ( dx[0] );

    for ( i = 1; i < n; i++ )
    {
      if ( dmax < r8_abs ( dx[i] ) )
      {
        value = i + 1;
        dmax = r8_abs ( dx[i] );
      }
    }
  }
  else
  {
    ix = 0;
    dmax = r8_abs ( dx[0] );
    ix = ix + incx;

    for ( i = 1; i < n; i++ )
    {
      if ( dmax < r8_abs ( dx[ix] ) )
      {
        value = i + 1;
        dmax = r8_abs ( dx[ix] );
      }
      ix = ix + incx;
    }
  }

  return value;
}
/******************************************************************************/

double r8_abs ( double x )

/******************************************************************************/
/*
  Purpose:

    R8_ABS returns the absolute value of a R8.

  Modified:

    02 April 2005

  Author:

    John Burkardt

  Parameters:

    Input, double X, the quantity whose absolute value is desired.

    Output, double R8_ABS, the absolute value of X.
*/
{
  double value;

  if ( 0.0 <= x )
  {
    value = x;
  } 
  else
  {
    value = -x;
  }
  return value;
}
/******************************************************************************/

double r8_epsilon ( )

/******************************************************************************/
/*
  Purpose:

    R8_EPSILON returns the R8 round off unit.

  Discussion:

    R8_EPSILON is a number R which is a power of 2 with the property that,
    to the precision of the computer's arithmetic,
      1 < 1 + R
    but
      1 = ( 1 + R / 2 )

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    01 September 2012

  Author:

    John Burkardt

  Parameters:

    Output, double R8_EPSILON, the R8 round-off unit.
*/
{
  const double value = 2.220446049250313E-016;

  return value;
}
/******************************************************************************/

double r8_max ( double x, double y )

/******************************************************************************/
/*
  Purpose:

    R8_MAX returns the maximum of two R8's.

  Modified:

    18 August 2004

  Author:

    John Burkardt

  Parameters:

    Input, double X, Y, the quantities to compare.

    Output, double R8_MAX, the maximum of X and Y.
*/
{
  double value;

  if ( y < x )
  {
    value = x;
  } 
  else
  {
    value = y;
  }
  return value;
}
/******************************************************************************/

double r8_random ( int iseed[4] )

/******************************************************************************/
/*
  Purpose:

    R8_RANDOM returns a uniformly distributed random number between 0 and 1.

  Discussion:

    This routine uses a multiplicative congruential method with modulus
    2**48 and multiplier 33952834046453 (see G.S.Fishman,
    'Multiplicative congruential random number generators with modulus
    2**b: an exhaustive analysis for b = 32 and a partial analysis for
    b = 48', Math. Comp. 189, pp 331-344, 1990).

    48-bit integers are stored in 4 integer array elements with 12 bits
    per element. Hence the routine is portable across machines with
    integers of 32 bits or more.

  Parameters:

    Input/output, integer ISEED(4).
    On entry, the seed of the random number generator; the array
    elements must be between 0 and 4095, and ISEED(4) must be odd.
    On exit, the seed is updated.

    Output, double R8_RANDOM, the next pseudorandom number.
*/
{
  int ipw2 = 4096;
  int it1;
  int it2;
  int it3;
  int it4;
  int m1 = 494;
  int m2 = 322;
  int m3 = 2508;
  int m4 = 2549;
  double one = 1.0;
  double r = 1.0 / 4096.0;
  double value;
/*
  Multiply the seed by the multiplier modulo 2**48.
*/
  it4 = iseed[3] * m4;
  it3 = it4 / ipw2;
  it4 = it4 - ipw2 * it3;
  it3 = it3 + iseed[2] * m4 + iseed[3] * m3;
  it2 = it3 / ipw2;
  it3 = it3 - ipw2 * it2;
  it2 = it2 + iseed[1] * m4 + iseed[2] * m3 + iseed[3] * m2;
  it1 = it2 / ipw2;
  it2 = it2 - ipw2 * it1;
  it1 = it1 + iseed[0] * m4 + iseed[1] * m3 + iseed[2] * m2 + iseed[3] * m1;
  it1 = ( it1 % ipw2 );
/*
  Return updated seed
*/
  iseed[0] = it1;
  iseed[1] = it2;
  iseed[2] = it3;
  iseed[3] = it4;
/*
  Convert 48-bit integer to a real number in the interval (0,1)
*/
  value = 
      r * ( ( double ) ( it1 ) 
    + r * ( ( double ) ( it2 ) 
    + r * ( ( double ) ( it3 ) 
    + r * ( ( double ) ( it4 ) ) ) ) );

  return value;
}
/******************************************************************************/

double *r8mat_gen ( int lda, int n )

/******************************************************************************/
/*
  Purpose:

    R8MAT_GEN generates a random R8MAT.

  Modified:

    06 June 2005

  Parameters:

    Input, integer LDA, the leading dimension of the matrix.

    Input, integer N, the order of the matrix.

    Output, double R8MAT_GEN[LDA*N], the N by N matrix.
*/
{
  double *a;
  int i;
  int init[4] = { 1, 2, 3, 1325 };
  int j;

  a = ( double * ) malloc ( lda * n * sizeof ( double ) );

  for ( j = 1; j <= n; j++ )
  {
    for ( i = 1; i <= n; i++ )
    {
      a[i-1+(j-1)*lda] = r8_random ( init ) - 0.5;
    }
  }

  return a;
}
/******************************************************************************/

void timestamp ( void )

/******************************************************************************/
/*
  Purpose:

    TIMESTAMP prints the current YMDHMS date as a time stamp.

  Example:

    31 May 2001 09:45:54 AM

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    24 September 2003

  Author:

    John Burkardt

  Parameters:

    None
*/
{
# define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct tm *tm;
  size_t len;
  time_t now;

  now = time ( NULL );
  tm = localtime ( &now );

  len = strftime ( time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm );

  printf ( "%s\n", time_buffer );

  return;
# undef TIME_SIZE
}

