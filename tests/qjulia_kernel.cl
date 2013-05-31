//
// File:       qjulia.c
//
// Abstract:   This example shows how to use OpenCL to raytrace a 4d Quaternion Julia-Set 
//             Fractal and intermix the results of a compute kernel with OpenGL for rendering.
//
// Version:    <1.0>
//
// Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc. ("Apple")
//             in consideration of your agreement to the following terms, and your use,
//             installation, modification or redistribution of this Apple software
//             constitutes acceptance of these terms.  If you do not agree with these
//             terms, please do not use, install, modify or redistribute this Apple
//             software.
//
//             In consideration of your agreement to abide by the following terms, and
//             subject to these terms, Apple grants you a personal, non - exclusive
//             license, under Apple's copyrights in this original Apple software ( the
//             "Apple Software" ), to use, reproduce, modify and redistribute the Apple
//             Software, with or without modifications, in source and / or binary forms;
//             provided that if you redistribute the Apple Software in its entirety and
//             without modifications, you must retain this notice and the following text
//             and disclaimers in all such redistributions of the Apple Software. Neither
//             the name, trademarks, service marks or logos of Apple Inc. may be used to
//             endorse or promote products derived from the Apple Software without specific
//             prior written permission from Apple.  Except as expressly stated in this
//             notice, no other rights or licenses, express or implied, are granted by
//             Apple herein, including but not limited to any patent rights that may be
//             infringed by your derivative works or by other works in which the Apple
//             Software may be incorporated.
//
//             The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
//             WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
//             WARRANTIES OF NON - INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
//             PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
//             ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//
//             IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
//             CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//             SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//             INTERRUPTION ) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION
//             AND / OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER
//             UNDER THEORY OF CONTRACT, TORT ( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR
//             OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright ( C ) 2008 Apple Inc. All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef WIDTH
#define WIDTH                       (512)
#endif
#ifndef HEIGHT
#define HEIGHT                      (512)
#endif
#define ASPECT                      ((float)WIDTH / (float)HEIGHT)
#define SQR(x)                      ((x)*(x))
#define BOUNDING_RADIUS             (2.0f)
#define BOUNDING_RADIUS_SQR         (SQR(BOUNDING_RADIUS))
#define ESCAPE_THRESHOLD            (BOUNDING_RADIUS * 1.5f)
#define DELTA                       (1e-5f)
#define ITERATIONS                  (10)
#define EPSILON                     (0.003f)
#define SHADOWS                     (0)


////////////////////////////////////////////////////////////////////////////////////////////////////


float4 qmult( float4 q1, float4 q2 )
{
    float4 r;
    float3 t;

    float3 q1yzw = (float3)(q1.y, q1.z, q1.w);
    float3 q2yzw = (float3)(q2.y, q2.z, q2.w);
    float3 c = cross( q1yzw, q2yzw );

    t = q2yzw * q1.x + q1yzw * q2.x + c;
    r.x = q1.x * q2.x - dot( q1yzw, q2yzw );
    r.yzw = t.xyz;

    return r;
}

float4 qsqr( float4 q )
{
    float4 r;
    float3 t;
    
    float3 qyzw = (float3)(q.y, q.z, q.w);

    t     = 2.0f * q.x * qyzw;
    r.x   = q.x * q.x - dot( qyzw, qyzw );
    r.yzw = t.xyz;

    return r;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float3 
EstimateNormalQJulia(
    float3 p,
    float4 c,
    int iterations )
{
    float4 qp = (float4)( p.x, p.y, p.z, 0.0f );
    float4 gx1 = qp - (float4)( DELTA, 0.0f, 0.0f, 0.0f );
    float4 gx2 = qp + (float4)( DELTA, 0.0f, 0.0f, 0.0f );
    float4 gy1 = qp - (float4)( 0.0f, DELTA, 0.0f, 0.0f );
    float4 gy2 = qp + (float4)( 0.0f, DELTA, 0.0f, 0.0f );
    float4 gz1 = qp - (float4)( 0.0f, 0.0f, DELTA, 0.0f );
    float4 gz2 = qp + (float4)( 0.0f, 0.0f, DELTA, 0.0f );

    for ( int i = 0; i < iterations; i++ )
    {
        gx1 = qsqr( gx1 ) + c;
        gx2 = qsqr( gx2 ) + c;
        gy1 = qsqr( gy1 ) + c;
        gy2 = qsqr( gy2 ) + c;
        gz1 = qsqr( gz1 ) + c;
        gz2 = qsqr( gz2 ) + c;
    }

    float nx = fast_length(gx2) - fast_length(gx1);
    float ny = fast_length(gy2) - fast_length(gy1);
    float nz = fast_length(gz2) - fast_length(gz1);

    float3 normal = fast_normalize((float3)( nx, ny, nz ));

    return normal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float4 
IntersectQJulia(
    float3 rO,
    float3 rD,
    float4 c,
    float epsilon,
    float escape)
{
    float rd = 0.0f;
    float dist = epsilon;
    while ( dist >= epsilon && rd < escape)
    {
        float4 z = (float4)( rO.x, rO.y, rO.z, 0.0f );
        float4 zp = (float4)( 1.0f, 0.0f, 0.0f, 0.0f );
        float zd = 0.0f;
        uint count = 0;
        while(zd < escape && count < ITERATIONS)
        {
            zp = 2.0f * qmult(z, zp);
            z = qsqr(z) + c;
            zd = dot(z, z);
            count++;
        }

        float normZ = fast_length( z );
        dist = 0.5f * normZ * half_log( normZ ) / fast_length( zp );
        rO += rD * dist;
        rd = dot(rO, rO);
    }

    return (float4)(rO.x, rO.y, rO.z, dist);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float3
Phong(
    float3 light,
    float3 eye,
    float3 pt,
    float3 normal,
    float3 base)
{
    const float SpecularExponent = 10.0f; 
    const float Specularity = 0.45f;

    float3 light_dir = fast_normalize( light - pt );
    float3 eye_dir = fast_normalize( eye - pt );
    float NdotL = dot( normal, light_dir );
    float3 reflect_dir = light_dir - 2.0f * NdotL * normal;

    base += fabs(normal) * 0.5f;
    float3 diffuse = base * fmax(NdotL, 0.0f);
    float3 specular = Specularity * half_powr( fmax( dot(eye_dir, reflect_dir), 0.0f), SpecularExponent );
    return diffuse + specular;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float
IntersectSphere(
    float3 rO,
    float3 rD,
    float radius )
{
    float fB = 2.0f * dot( rO, rD );
    float fB2 = fB * fB;
    float fC = dot( rO, rO ) - radius;
    float fT = (fB2 - 4.0f * fC);
    if (fT <= 0.0f)
        return 0.0f;
    float fD = half_sqrt( fT );
    float fT0 = ( -fB + fD ) * 0.5f;
    float fT1 = ( -fB - fD ) * 0.5f;
    fT = fmin(fT0, fT1);
    return fT;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float4 
RaytraceQJulia(
    float3 rO,
    float3 rD,
    float4 mu,
    float epsilon,
    float3 eye,
    float3 light,
    float3 diffuse,
    float radius,
    bool shadows,
    int iterations )
{
    const float4 background = (float4)( 0.15f, 0.15f, 0.15f, 0.0f );
    float4 color = background;

    rD = fast_normalize( rD );
    float t = IntersectSphere( rO, rD, radius );
    if ( t <= 0.0f )
        return color;

    rO += rD * t;
    float4 hit = IntersectQJulia( rO, rD, mu, epsilon, ESCAPE_THRESHOLD );
    float dist = hit.w;
    if (dist >= epsilon)
        return color;

    rO.xyz = hit.xyz;
    float3 normal = EstimateNormalQJulia( rO, mu, iterations );

    float3 rgb = Phong( light, rD, rO, normal, diffuse );
    color.xyz = rgb.xyz;
    color.w = 1.0f;

    if (SHADOWS)
    {
        float3 light_dir = fast_normalize( light - rO );
        rO += normal * epsilon * 2.0f;
        hit = IntersectQJulia( rO, light_dir, mu, epsilon, ESCAPE_THRESHOLD );
        dist = hit.w;
        color.xyz *= (dist < epsilon) ? (0.4f) : (1.0f);
    }

    return color;
}

///////////////////////////////////////////////////////////////////////////////////////////

float4 
QJulia(
    float4 coord,
    float4 mu,
    float4 diffuse,
    float epsilon,
    float iterations,
    int shadows,
    uint width,
    uint height)
{
    float zoom = BOUNDING_RADIUS_SQR;
    float radius = BOUNDING_RADIUS_SQR;

    float2 size = (float2)((float)width, (float)height);
    float scale = max(size.x, size.y);
    float2 h = (float2)(0.5f, 0.5f); // half
    float2 position = (coord.xy - h * size) / scale;
    float2 frame = (position) * zoom;

    float3 light = (float3)(1.5f, 0.5f, 4.0f);
    float3 eye = (float3)(0.0f, 0.0f, 4.0f);
    float3 ray = (float3)(frame.x, frame.y, 0.0f);
    float3 base = (float3)(diffuse.x, diffuse.y, diffuse.z);    

    float3 rO = eye;
    float3 rD = (ray - rO);
    
    float4 color = RaytraceQJulia( rO, rD, mu, epsilon, eye, light, base, radius, shadows, iterations);

    return color;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

__kernel void
QJuliaKernel(
    __global uchar4 *result,
    const float4 mu,
    const float4 diffuse,
    const float epsilon)
{
    int tx = get_global_id(0);
    int ty = get_global_id(1);
    int sx = get_global_size(0);
    int sy = get_global_size(1);
    int index = ty * WIDTH + tx;
    bool valid = (tx < WIDTH) && (ty < HEIGHT);

    float4 coord = (float4)((float)tx, (float)ty, 0.0f, 0.0f);
    
    if(valid)
    {
        float4 color = QJulia(coord, mu, diffuse, epsilon, ITERATIONS, SHADOWS, WIDTH, HEIGHT);
        uchar4 output = convert_uchar4_sat_rte(color * 255.0f);
        result[index] = output;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
