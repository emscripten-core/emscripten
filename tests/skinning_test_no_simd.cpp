// From https://github.com/chadaustin/Web-Benchmarks/blob/master/skinning_test_no_simd.cpp
// Modifications:
//   1. Run for a fixed # of iterations, so the total runtime is the benchmark
//   2. Not have so much stuff on the stack in main()
//
// compiled in cygwin with:
// g++ -Wall -O2 -o skinning_test_no_simd skinning_test_no_simd.cpp

#include <vector>
#include <set>
#include <map>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct CalBase4 {
  float x, y, z, w;

  void set(float _x, float _y, float _z, float _w) {
    x = _x;
    y = _y;
    z = _z;
    w = _w;
  }
};

struct CalVector4 : CalBase4 {
  CalVector4() {
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    w = 0.0f;
  }

  CalVector4(float x, float y, float z, float w = 0.0f) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
  }

  void setAsVector(float x, float y, float z) {
    this->x = x;
    this->y = y;
    this->z = z;
    w = 0.0f;
  }
};

struct CalPoint4 : CalBase4 {
  CalPoint4() {
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    w = 1.0f;
  }

  CalPoint4(float x, float y, float z, float w = 1.0f) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
  }

  void setAsPoint(float x, float y, float z) {
    x = this->x;
    y = this->y;
    z = this->z;
    w = 1.0f;
  }
};

// 3x3 transform matrix plus a translation 3-vector (stored in the w components
// of the rows.
struct BoneTransform {
  CalVector4 rowx;
  CalVector4 rowy;
  CalVector4 rowz;
};

struct Influence
{
  Influence() {
    boneId = -1;
    weight = 0.0f;
    lastInfluenceForThisVertex = 0;
  }
  
  Influence(unsigned b, float w, bool last) {
    boneId = b;
    weight = w;
    lastInfluenceForThisVertex = last ? 1 : 0;
  }

  unsigned boneId;
  float weight;
  unsigned lastInfluenceForThisVertex;
};

struct Vertex
{
  CalPoint4 position;
  CalVector4 normal;
};

inline void ScaleMatrix(BoneTransform& result, const BoneTransform& mat, const float s) {
  result.rowx.x = s * mat.rowx.x;
  result.rowx.y = s * mat.rowx.y;
  result.rowx.z = s * mat.rowx.z;
  result.rowx.w = s * mat.rowx.w;
  result.rowy.x = s * mat.rowy.x;
  result.rowy.y = s * mat.rowy.y;
  result.rowy.z = s * mat.rowy.z;
  result.rowy.w = s * mat.rowy.w;
  result.rowz.x = s * mat.rowz.x;
  result.rowz.y = s * mat.rowz.y;
  result.rowz.z = s * mat.rowz.z;
  result.rowz.w = s * mat.rowz.w;
}
inline void AddScaledMatrix(BoneTransform& result, const BoneTransform& mat, const float s) {
  result.rowx.x += s * mat.rowx.x;
  result.rowx.y += s * mat.rowx.y;
  result.rowx.z += s * mat.rowx.z;
  result.rowx.w += s * mat.rowx.w;
  result.rowy.x += s * mat.rowy.x;
  result.rowy.y += s * mat.rowy.y;
  result.rowy.z += s * mat.rowy.z;
  result.rowy.w += s * mat.rowy.w;
  result.rowz.x += s * mat.rowz.x;
  result.rowz.y += s * mat.rowz.y;
  result.rowz.z += s * mat.rowz.z;
  result.rowz.w += s * mat.rowz.w;
}
inline void TransformPoint(CalVector4& result, const BoneTransform& m, const CalBase4& v) {
  result.x = m.rowx.x * v.x + m.rowx.y * v.y + m.rowx.z * v.z + m.rowx.w;
  result.y = m.rowy.x * v.x + m.rowy.y * v.y + m.rowy.z * v.z + m.rowy.w;
  result.z = m.rowz.x * v.x + m.rowz.y * v.y + m.rowz.z * v.z + m.rowz.w;
}
inline void TransformVector(CalVector4& result, const BoneTransform& m, const CalBase4& v) {
  result.x = m.rowx.x * v.x + m.rowx.y * v.y + m.rowx.z * v.z;
  result.y = m.rowy.x * v.x + m.rowy.y * v.y + m.rowy.z * v.z;
  result.z = m.rowz.x * v.x + m.rowz.y * v.y + m.rowz.z * v.z;
}

void calculateVerticesAndNormals_x87(
  const BoneTransform* boneTransforms,
  int vertexCount,
  const Vertex* vertices,
  const Influence* influences,
  CalVector4* output_vertex
) {

  BoneTransform total_transform;

  // calculate all submesh vertices
  while (vertexCount--) {
    ScaleMatrix(total_transform, boneTransforms[influences->boneId], influences->weight);

    while (!influences++->lastInfluenceForThisVertex) {
      AddScaledMatrix(total_transform, boneTransforms[influences->boneId], influences->weight);
    }

    TransformPoint(output_vertex[0], total_transform, vertices->position);
    TransformVector(output_vertex[1], total_transform, vertices->normal);
    ++vertices;
    output_vertex += 2;
  }
}

int main (int argc, char*argv[])
{
  int N, M;
  int arg = argc > 1 ? argv[1][0] - '0' : 3;
  switch(arg) {
    case 0: return 0; break;
    case 1: N = 2000; M = 1700; break;
    case 2: N = 6600; M = 6500; break;
    case 3: N = 9500; M = 10000; break;
    case 4: N = 2*11000; M = 2*12000; break;
    case 5: N = 3*10000; M = 3*10800; break;
    default: printf("error: %d\\n", arg); return -1;
  }

  Vertex *v = new Vertex[N];
  Influence *i = new Influence[N];
  for (int k = 0; k < N; ++k) {
    v[k].position.setAsPoint(1.0f, 2.0f, 3.0f);
    v[k].normal.setAsVector(0.0f, 0.0f, 1.0f);
    i[k].boneId = 0;
    i[k].weight = 1.0f;
    i[k].lastInfluenceForThisVertex = true;
  }

  BoneTransform bt;
  memset(&bt, 0, sizeof(bt));

  CalVector4 *output = new CalVector4[N * 2];

  for (unsigned j = 0; j < M; j++)
    calculateVerticesAndNormals_x87(&bt, N, v, i, output);

  float sum = 0;
  for (unsigned j = 0; j < N * 2; ++j) {
    sum += (output[j].x + output[j].y + output[j].z + output[j].w);
  }

  printf("blah=%f\n", sum);
}

