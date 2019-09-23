/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Ray tracing with compute shader, inspired by
 *
 *   smallpt by Kevin Beason (http://kevinbeason.com/smallpt/)
 *
 * and
 *
 *   SmallptGPU by David Bucciarelli (http://davibu.interfree.it/opencl/smallptgpu/smallptGPU.html)
 */

#define GLFW_INCLUDE_ES31
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

#if __EMSCRIPTEN__
#include <deque>
#include <emscripten.h>
#include <functional>
#endif

// https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html
#define STR(s) #s
#define XSTR(s) STR(s)

#ifndef WIDTH
#define WIDTH 320
#endif
#ifndef HEIGHT
#define HEIGHT 240
#endif

#ifndef LOCAL_SIZE_X
#define LOCAL_SIZE_X 1
#endif
#ifndef LOCAL_SIZE_Y
#define LOCAL_SIZE_Y 1
#endif

#ifndef MAX_REGION_SIZE
#define MAX_REGION_SIZE (WIDTH * HEIGHT * 4)
#endif

GLFWwindow *window;
bool quit = false;

GLuint compute_shader;
GLuint vertex_shader;
GLuint fragment_shader;
GLuint compute_program;
GLuint draw_program;
GLuint rand_buffer;
GLuint scene_buffer;
GLuint texture;
GLuint sampler;
GLuint vertex_array;
GLuint vertex_buffer;

#if __EMSCRIPTEN__
std::deque<std::function<void()>> queue;
#endif

void setup()
{
  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  window = glfwCreateWindow(WIDTH, HEIGHT, "SmallPT", nullptr, nullptr);
  if (!window) {
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);

  glClearColor(0.f, 0.f, 0.f, 1.f);

  {
    const GLchar *str =
        "#version 310 es"                                                                                            "\n"
        "precision highp int;"                                                                                       "\n"
        "precision highp float;"                                                                                     "\n"
        "precision mediump image2DArray;"                                                                            "\n"
        ""                                                                                                           "\n"
        "struct Ray {"                                                                                               "\n"
        "  vec3 origin;"                                                                                             "\n"
        "  vec3 direction;"                                                                                          "\n"
        "};"                                                                                                         "\n"
        "struct Sphere {"                                                                                            "\n"
        "  float radius;"                                                                                            "\n"
        "  vec3 position;"                                                                                           "\n"
        "  vec3 emission;"                                                                                           "\n"
        "  vec3 color;"                                                                                              "\n"
        "  int material;"                                                                                            "\n"
        "};"                                                                                                         "\n"
        ""                                                                                                           "\n"
        "const int DIFFUSE = 0;"                                                                                     "\n"
        "const int SPECULAR = DIFFUSE + 1;"                                                                          "\n"
        "const int REFRACTION = SPECULAR + 1;"                                                                       "\n"
        "layout(local_size_x = " XSTR(LOCAL_SIZE_X) ", local_size_y = " XSTR(LOCAL_SIZE_Y) ", local_size_z = 1) in;" "\n"
        "layout(binding = 0, std140) buffer Rands {"                                                                 "\n"
        "  uint rands[];"                                                                                            "\n"
        "};"                                                                                                         "\n"
        "layout(binding = 1, std140) buffer Scene {"                                                                 "\n"
        "  Ray camera;"                                                                                              "\n"
        "  Sphere spheres[];"                                                                                        "\n"
        "};"                                                                                                         "\n"
        "layout(binding = 0, r32f) uniform image2DArray uImage;"                                                     "\n"
        ""                                                                                                           "\n"
        "float rand()"                                                                                               "\n"
        "{"                                                                                                          "\n"
        "  // Marsaglia's multiply-with-carry pseudorandom number generator"                                         "\n"
        "  uint i = gl_GlobalInvocationID.y * uint(imageSize(uImage).x) + gl_GlobalInvocationID.x;"                  "\n"
        "  uint z = rands[i * 2u];"                                                                                  "\n"
        "  uint w = rands[i * 2u + 1u];"                                                                             "\n"
        "  uint mwc;"                                                                                                "\n"
        "  z = 36969u * (z & 65535u) + (z >> 16);"                                                                   "\n"
        "  w = 18000u * (w & 65535u) + (w >> 16);"                                                                   "\n"
        "  mwc = (z << 16) + w;"                                                                                     "\n"
        "  rands[i * 2u] = z;"                                                                                       "\n"
        "  rands[i * 2u + 1u] = w;"                                                                                  "\n"
        ""                                                                                                           "\n"
        "  mwc &= 0x007fffffu;"                                                                                      "\n"
        "  mwc |= 0x3f800000u;"                                                                                      "\n"
        "  return uintBitsToFloat(mwc) - 1.0;"                                                                       "\n"
        "}"                                                                                                          "\n"
        ""                                                                                                           "\n"
        "bool intersect(Ray ray, out vec3 hitPoint, out Sphere sphere)"                                              "\n"
        "{"                                                                                                          "\n"
        "  float infinity = 1.0e20;"                                                                                 "\n"
        "  float epsilon = 0.01;"                                                                                    "\n"
        "  float distance = infinity;"                                                                               "\n"
#if 0 // workaround https://anglebug.com/2972
        "  for (int i = 0; i < spheres.length(); i++) {"                                                             "\n"
#else
        "  int i = 0;"                                                                                               "\n"
        "  while (true) {"                                                                                           "\n"
        "    if (i >= spheres.length()) break;"                                                                      "\n"
#endif
        "    vec3 o = ray.origin;"                                                                                   "\n"
        "    vec3 d = ray.direction;"                                                                                "\n"
        "    float r = spheres[i].radius;"                                                                           "\n"
        "    vec3 p = spheres[i].position;"                                                                          "\n"
        "    float t;"                                                                                               "\n"
        ""                                                                                                           "\n"
        "    // Solve dot(d, d) * t * t + 2 * dot(d, o - p) * t + dot(o - p, o - p) - r * r = 0"                     "\n"
        "    float a = 1.0;"                                                                                         "\n"
        "    float b = 2.0 * dot(d, o - p);"                                                                         "\n"
        "    float c = dot(o - p, o - p) - r * r;"                                                                   "\n"
        "    float delta = b * b - 4.0 * a * c;"                                                                     "\n"
        "    if (delta < 0.0) {"                                                                                     "\n"
#if 1
        "      i++;"                                                                                                 "\n"
#endif
        "      continue;"                                                                                            "\n"
        "    }"                                                                                                      "\n"
        "    do {"                                                                                                   "\n"
        "      t = (-b - sqrt(delta)) / (2.0 * a);"                                                                  "\n"
        "      if (t > epsilon) break;"                                                                              "\n"
        "      t = (-b + sqrt(delta)) / (2.0 * a);"                                                                  "\n"
        "      if (t > epsilon) break;"                                                                              "\n"
        "    } while (false);"                                                                                       "\n"
        ""                                                                                                           "\n"
        "    if (t > epsilon && t < distance) {"                                                                     "\n"
        "      distance = t;"                                                                                        "\n"
        "      hitPoint = ray.origin + distance * ray.direction;"                                                    "\n"
#if 0 // workaround struct assignment from SSBO (https://anglebug.com/1951)
        "      sphere = spheres[i];"                                                                                 "\n"
#else
        "      sphere.radius = spheres[i].radius;"                                                                   "\n"
        "      sphere.position = spheres[i].position;"                                                               "\n"
        "      sphere.emission = spheres[i].emission;"                                                               "\n"
        "      sphere.color = spheres[i].color;"                                                                     "\n"
        "      sphere.material = spheres[i].material;"                                                               "\n"
#endif
        "    }"                                                                                                      "\n"
        ""                                                                                                           "\n"
#if 1
        "    i++;"                                                                                                   "\n"
#endif
        "  }"                                                                                                        "\n"
        "  return distance < infinity;"                                                                              "\n"
        "}"                                                                                                          "\n"
        ""                                                                                                           "\n"
        "vec3 radiance(Ray ray)"                                                                                     "\n"
        "{"                                                                                                          "\n"
        "  vec3 color = vec3(0.0); // accumulated color"                                                             "\n"
        "  vec3 throughput = vec3(1.0); // accumulated reflectance"                                                  "\n"
        "  for (int depth = 0; depth < 5; depth++) {"                                                                "\n"
        "    vec3 hitPoint;"                                                                                         "\n"
        "    Sphere sphere;"                                                                                         "\n"
        ""                                                                                                           "\n"
        "    if (!intersect(ray, hitPoint, sphere)) return color;"                                                   "\n"
        ""                                                                                                           "\n"
        "    color += throughput * sphere.emission;"                                                                 "\n"
        "    throughput *= sphere.color;"                                                                            "\n"
        ""                                                                                                           "\n"
        "    vec3 normal = normalize(hitPoint - sphere.position);"                                                   "\n"
        "    bool into = dot(normal, ray.direction) < 0.0;"                                                          "\n"
        "    vec3 direction;"                                                                                        "\n"
        "    switch (sphere.material) {"                                                                             "\n"
        "    case DIFFUSE: // Ideal DIFFUSE reflection"                                                              "\n"
        "      float rand1 = rand();"                                                                                "\n"
        "      float rand2 = rand();"                                                                                "\n"
        "      vec3 u, v, w;"                                                                                        "\n"
        "      w = into ? normal : -normal;"                                                                         "\n"
        "      if (abs(w.x) > 0.1) {"                                                                                "\n"
        "        u = normalize(cross(vec3(0.0, 1.0, 0.0), w));"                                                      "\n"
        "      } else {"                                                                                             "\n"
        "        u = normalize(cross(vec3(1.0, 0.0, 0.0), w));"                                                      "\n"
        "      }"                                                                                                    "\n"
        "      v = cross(w, u);"                                                                                     "\n"
        "      direction = u * cos(radians(360.0) * rand1) * sqrt(rand2) +"                                          "\n"
        "                  v * sin(radians(360.0) * rand1) * sqrt(rand2) +"                                          "\n"
        "                  w * sqrt(1.0 - rand2);"                                                                   "\n"
        "      ray.origin = hitPoint;"                                                                               "\n"
        "      ray.direction = normalize(direction);"                                                                "\n"
        "      break;"                                                                                               "\n"
        "    case SPECULAR: // Ideal SPECULAR reflection"                                                            "\n"
        "      ray.origin = hitPoint;"                                                                               "\n"
        "      ray.direction = reflect(ray.direction, normal);"                                                      "\n"
        "      break;"                                                                                               "\n"
        "    case REFRACTION: // Ideal dielectric REFRACTION"                                                        "\n"
        "      float n1 = 1.0;"                                                                                      "\n"
        "      float n2 = 1.5;"                                                                                      "\n"
        "      float reflection, transmission;"                                                                      "\n"
        ""                                                                                                           "\n"
        "      if (into) {"                                                                                          "\n"
        "        direction = refract(ray.direction, normal, n1 / n2);"                                               "\n"
        "      } else {"                                                                                             "\n"
        "        direction = refract(ray.direction, -normal, n2 / n1);"                                              "\n"
        "      }"                                                                                                    "\n"
        "      if (direction == vec3(0.0)) { // Total internal reflection"                                           "\n"
        "        ray.origin = hitPoint;"                                                                             "\n"
        "        ray.direction = reflect(ray.direction, normal);"                                                    "\n"
        "        break;"                                                                                             "\n"
        "      }"                                                                                                    "\n"
        ""                                                                                                           "\n"
        "      // Fresnel reflectance"                                                                               "\n"
        "      float a = n2 - n1;"                                                                                   "\n"
        "      float b = n2 + n1;"                                                                                   "\n"
        "      float c = 1.0 - (into ? dot(ray.direction, -normal) : dot(direction, normal));"                       "\n"
        "      float r0 = (a * a) / (b * b);"                                                                        "\n"
        "      reflection = r0 + (1.0 - r0) * c * c * c * c * c;"                                                    "\n"
        "      transmission = 1.0 - reflection;"                                                                     "\n"
        ""                                                                                                           "\n"
        "      // Russian roulette"                                                                                  "\n"
        "      float p = 0.25 + 0.5 * reflection;"                                                                   "\n"
        "      if (rand() < p) {"                                                                                    "\n"
        "        throughput = throughput * reflection / p;"                                                          "\n"
        "        ray.origin = hitPoint;"                                                                             "\n"
        "        ray.direction = reflect(ray.direction, normal);"                                                    "\n"
        "      } else {"                                                                                             "\n"
        "        throughput = throughput * transmission / (1.0 - p);"                                                "\n"
        "        ray.origin = hitPoint;"                                                                             "\n"
        "        ray.direction = direction;"                                                                         "\n"
        "      }"                                                                                                    "\n"
        "      break;"                                                                                               "\n"
        "    }"                                                                                                      "\n"
        "  }"                                                                                                        "\n"
        "  return color;"                                                                                            "\n"
        "}"                                                                                                          "\n"
        ""                                                                                                           "\n"
        "void main()"                                                                                                "\n"
        "{"                                                                                                          "\n"
        "  uint width = uint(imageSize(uImage).x);"                                                                  "\n"
        "  uint height = uint(imageSize(uImage).y);"                                                                 "\n"
        "  uint x = gl_GlobalInvocationID.x;"                                                                        "\n"
        "  uint y = gl_GlobalInvocationID.y;"                                                                        "\n"
        "  Ray ray;"                                                                                                 "\n"
        "  vec3 color;"                                                                                              "\n"
        ""                                                                                                           "\n"
        "  if (x >= width || y >= height) return;"                                                                   "\n"
        ""                                                                                                           "\n"
        "  vec3 cx = vec3(float(width) * 0.5135 / float(height), 0.0, 0.0);"                                         "\n"
        "  vec3 cy = normalize(cross(cx, camera.direction)) * 0.5135;"                                               "\n"
        "  float rand1 = rand();"                                                                                    "\n"
        "  float rand2 = rand();"                                                                                    "\n"
        "  vec3 direction = cx * ((float(x) + rand1) / float(width) - 0.5) +"                                        "\n"
        "                   cy * ((float(y) + rand2) / float(height) - 0.5) +"                                       "\n"
        "                   camera.direction;"                                                                       "\n"
        "  ray.origin = camera.origin + direction * 140.0;"                                                          "\n"
        "  ray.direction = normalize(direction);"                                                                    "\n"
        ""                                                                                                           "\n"
        "  color = radiance(ray);"                                                                                   "\n"
        ""                                                                                                           "\n"
        "  float r = imageLoad(uImage, ivec3(x, y, 0)).x;"                                                           "\n"
        "  float g = imageLoad(uImage, ivec3(x, y, 1)).x;"                                                           "\n"
        "  float b = imageLoad(uImage, ivec3(x, y, 2)).x;"                                                           "\n"
        "  imageStore(uImage, ivec3(x, y, 0), vec4(r + color.x, 0.0, 0.0, 1.0));"                                    "\n"
        "  imageStore(uImage, ivec3(x, y, 1), vec4(g + color.y, 0.0, 0.0, 1.0));"                                    "\n"
        "  imageStore(uImage, ivec3(x, y, 2), vec4(b + color.z, 0.0, 0.0, 1.0));"                                    "\n"
        "}"                                                                                                          "\n"
        "";
    GLint status;
    GLint length;
    std::vector<GLchar> log;
    compute_shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute_shader, 1, &str, nullptr);
    glCompileShader(compute_shader);
    glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
      glGetShaderiv(compute_shader, GL_INFO_LOG_LENGTH, &length);
      log.resize(length);
      glGetShaderInfoLog(compute_shader, log.size(), &length, log.data());
      std::cerr << std::string(log.data(), length * sizeof log[0]);
      exit(EXIT_FAILURE);
    }
  }

  {
    GLint status;
    GLint length;
    std::vector<GLchar> log;
    compute_program = glCreateProgram();
    glAttachShader(compute_program, compute_shader);
    glLinkProgram(compute_program);
    glGetProgramiv(compute_program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
      glGetProgramiv(compute_program, GL_INFO_LOG_LENGTH, &length);
      log.resize(length);
      glGetProgramInfoLog(compute_program, log.size(), &length, log.data());
      std::cerr << std::string(log.data(), length * sizeof log[0]);
      exit(EXIT_FAILURE);
    }
  }

  {
    const GLchar *str =
        "#version 310 es"                       "\n"
        "precision mediump float;"              "\n"
        ""                                      "\n"
        "in vec2 aPos;"                         "\n"
        "in vec2 aTexCoord;"                    "\n"
        "out vec2 vTexCoord;"                   "\n"
        ""                                      "\n"
        "void main()"                           "\n"
        "{"                                     "\n"
        "  gl_Position = vec4(aPos, 0.0, 1.0);" "\n"
        "  vTexCoord = aTexCoord;"              "\n"
        "}"                                     "\n"
        "";
    GLint status;
    GLint length;
    std::vector<GLchar> log;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &str, nullptr);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
      glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &length);
      log.resize(length);
      glGetShaderInfoLog(vertex_shader, log.size(), &length, log.data());
      std::cerr << std::string(log.data(), length * sizeof log[0]);
      exit(EXIT_FAILURE);
    }
  }

  {
    const GLchar *str =
        "#version 310 es"                                                    "\n"
        "precision mediump float;"                                           "\n"
        "precision mediump sampler2DArray;"                                  "\n"
        ""                                                                   "\n"
        "in vec2 vTexCoord;"                                                 "\n"
        "out vec4 color;"                                                    "\n"
        "uniform float uScale;"                                              "\n"
        "uniform sampler2DArray uSampler;"                                   "\n"
        ""                                                                   "\n"
        "void main()"                                                        "\n"
        "{"                                                                  "\n"
        "  color.x = texture(uSampler, vec3(vTexCoord, 0.0)).x;"             "\n"
        "  color.y = texture(uSampler, vec3(vTexCoord, 1.0)).x;"             "\n"
        "  color.z = texture(uSampler, vec3(vTexCoord, 2.0)).x;"             "\n"
        "  color.xyz = clamp(uScale * color.xyz, 0.0, 1.0);"                 "\n"
        "  color.xyz = pow(color.xyz, vec3(1.0 / 2.2)); // gamma correction" "\n"
        "  color.w = 1.0;"                                                   "\n"
        "}"                                                                  "\n"
        "";
    GLint status;
    GLint length;
    std::vector<GLchar> log;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &str, nullptr);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
      glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &length);
      log.resize(length);
      glGetShaderInfoLog(fragment_shader, log.size(), &length, log.data());
      std::cerr << std::string(log.data(), length * sizeof log[0]);
      exit(EXIT_FAILURE);
    }
  }

  {
    GLint status;
    GLint length;
    std::vector<GLchar> log;
    draw_program = glCreateProgram();
    glAttachShader(draw_program, vertex_shader);
    glAttachShader(draw_program, fragment_shader);
    glLinkProgram(draw_program);
    glGetProgramiv(draw_program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
      glGetProgramiv(draw_program, GL_INFO_LOG_LENGTH, &length);
      log.resize(length);
      glGetProgramInfoLog(draw_program, log.size(), &length, log.data());
      std::cerr << std::string(log.data(), length * sizeof log[0]);
      exit(EXIT_FAILURE);
    }
  }

  {
    std::vector<int> rands;
    std::mt19937 generator;
    std::uniform_int_distribution<int> distribution(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    int total_size = WIDTH * HEIGHT * 2 * 4 * sizeof rands[0];
    int region_size = MAX_REGION_SIZE / (4 * sizeof rands[0]) * (4 * sizeof rands[0]);
    rands.resize(region_size / sizeof rands[0], 0);
    glGenBuffers(1, &rand_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, rand_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, total_size, nullptr, GL_DYNAMIC_COPY);
    for (int region_offset = 0; region_offset < total_size; region_offset += region_size) {
      for (size_t i = 0; i < rands.size(); i += 4)
        rands[i] = distribution(generator);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, region_offset, std::min(total_size - region_offset, region_size), rands.data());
    }
  }

  {
    static struct {
      struct {
        float x;
        float y;
        float z;
      } origin;
      struct {
        float x;
        float y;
        float z;
      } direction;
    } camera;
    static struct {
      float radius;
      struct {
        float x;
        float y;
        float z;
      } position;
      struct {
        float r;
        float g;
        float b;
      } emission;
      struct {
        float r;
        float g;
        float b;
      } color;
      enum {
        DIFFUSE = 0,
        SPECULAR = DIFFUSE + 1,
        REFRACTION = SPECULAR + 1,
      } material;
    } spheres[9];

    float l;
    camera.origin.x = 50.f;
    camera.origin.y = 52.f;
    camera.origin.z = 295.6f;
    camera.direction.x = 0.f;
    camera.direction.y = -0.042612f;
    camera.direction.z = -1.f;
    l = camera.direction.x * camera.direction.x +
        camera.direction.y * camera.direction.y +
        camera.direction.z * camera.direction.z;
    l = sqrt(l);
    camera.direction.x /= l;
    camera.direction.y /= l;
    camera.direction.z /= l;

    // left
    spheres[0].radius = 10000.f;
    spheres[0].position.x = 10000.f + 1.f;
    spheres[0].position.y = 40.8f;
    spheres[0].position.z = 81.6f;
    spheres[0].emission.r = 0.f;
    spheres[0].emission.g = 0.f;
    spheres[0].emission.b = 0.f;
    spheres[0].color.r = 0.75f;
    spheres[0].color.g = 0.25f;
    spheres[0].color.b = 0.25f;
    spheres[0].material = spheres->DIFFUSE;

    // right
    spheres[1].radius = 10000.f;
    spheres[1].position.x = -10000.f + 99.f;
    spheres[1].position.y = 40.8f;
    spheres[1].position.z = 81.6f;
    spheres[1].emission.r = 0.f;
    spheres[1].emission.g = 0.f;
    spheres[1].emission.b = 0.f;
    spheres[1].color.r = 0.25f;
    spheres[1].color.g = 0.25f;
    spheres[1].color.b = 0.75f;
    spheres[1].material = spheres->DIFFUSE;

    // back
    spheres[2].radius = 10000.f;
    spheres[2].position.x = 50.f;
    spheres[2].position.y = 40.8f;
    spheres[2].position.z = 10000.f;
    spheres[2].emission.r = 0.f;
    spheres[2].emission.g = 0.f;
    spheres[2].emission.b = 0.f;
    spheres[2].color.r = 0.75f;
    spheres[2].color.g = 0.75f;
    spheres[2].color.b = 0.75f;
    spheres[2].material = spheres->DIFFUSE;

    // front
    spheres[3].radius = 10000.f;
    spheres[3].position.x = 50.f;
    spheres[3].position.y = 40.8f;
    spheres[3].position.z = -10000.f + 170.f;
    spheres[3].emission.r = 0.f;
    spheres[3].emission.g = 0.f;
    spheres[3].emission.b = 0.f;
    spheres[3].color.r = 0.f;
    spheres[3].color.g = 0.f;
    spheres[3].color.b = 0.f;
    spheres[3].material = spheres->DIFFUSE;

    // bottom
    spheres[4].radius = 10000.f;
    spheres[4].position.x = 50.f;
    spheres[4].position.y = 10000.f;
    spheres[4].position.z = 81.6f;
    spheres[4].emission.r = 0.f;
    spheres[4].emission.g = 0.f;
    spheres[4].emission.b = 0.f;
    spheres[4].color.r = 0.75f;
    spheres[4].color.g = 0.75f;
    spheres[4].color.b = 0.75f;
    spheres[4].material = spheres->DIFFUSE;

    // top
    spheres[5].radius = 10000.f;
    spheres[5].position.x = 50.f;
    spheres[5].position.y = -10000.f + 81.6f;
    spheres[5].position.z = 81.6f;
    spheres[5].emission.r = 0.f;
    spheres[5].emission.g = 0.f;
    spheres[5].emission.b = 0.f;
    spheres[5].color.r = 0.75f;
    spheres[5].color.g = 0.75f;
    spheres[5].color.b = 0.75f;
    spheres[5].material = spheres->DIFFUSE;

    // mirror
    spheres[6].radius = 16.5f;
    spheres[6].position.x = 27.f;
    spheres[6].position.y = 16.5f;
    spheres[6].position.z = 47.f;
    spheres[6].emission.r = 0.f;
    spheres[6].emission.g = 0.f;
    spheres[6].emission.b = 0.f;
    spheres[6].color.r = 0.999f;
    spheres[6].color.g = 0.999f;
    spheres[6].color.b = 0.999f;
    spheres[6].material = spheres->SPECULAR;

    // glass
    spheres[7].radius = 16.5f;
    spheres[7].position.x = 73.f;
    spheres[7].position.y = 16.5f;
    spheres[7].position.z = 78.f;
    spheres[7].emission.r = 0.f;
    spheres[7].emission.g = 0.f;
    spheres[7].emission.b = 0.f;
    spheres[7].color.r = 0.999f;
    spheres[7].color.g = 0.999f;
    spheres[7].color.b = 0.999f;
    spheres[7].material = spheres->REFRACTION;

    // light
    spheres[8].radius = 600.f;
    spheres[8].position.x = 50.f;
    spheres[8].position.y = 681.6f - 0.27f;
    spheres[8].position.z = 81.6f;
    spheres[8].emission.r = 12.f;
    spheres[8].emission.g = 12.f;
    spheres[8].emission.b = 12.f;
    spheres[8].color.r = 0.f;
    spheres[8].color.g = 0.f;
    spheres[8].color.b = 0.f;
    spheres[8].material = spheres->DIFFUSE;

    glGenBuffers(1, &scene_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, scene_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 32 + 64 * sizeof spheres / sizeof spheres[0], nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof camera.origin.x, &camera.origin.x);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 4, sizeof camera.origin.y, &camera.origin.y);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 8, sizeof camera.origin.z, &camera.origin.z);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 16, sizeof camera.direction.x, &camera.direction.x);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 20, sizeof camera.direction.y, &camera.direction.y);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 24, sizeof camera.direction.z, &camera.direction.z);
    for (size_t i = 0; i < sizeof spheres / sizeof spheres[0]; i++) {
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, 32 + 64 * i, sizeof spheres[i].radius, &spheres[i].radius);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, 32 + 64 * i + 16, sizeof spheres[i].position.x, &spheres[i].position.x);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, 32 + 64 * i + 20, sizeof spheres[i].position.y, &spheres[i].position.y);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, 32 + 64 * i + 24, sizeof spheres[i].position.z, &spheres[i].position.z);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, 32 + 64 * i + 32, sizeof spheres[i].emission.r, &spheres[i].emission.r);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, 32 + 64 * i + 36, sizeof spheres[i].emission.g, &spheres[i].emission.g);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, 32 + 64 * i + 40, sizeof spheres[i].emission.b, &spheres[i].emission.b);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, 32 + 64 * i + 48, sizeof spheres[i].color.r, &spheres[i].color.r);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, 32 + 64 * i + 52, sizeof spheres[i].color.g, &spheres[i].color.g);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, 32 + 64 * i + 56, sizeof spheres[i].color.b, &spheres[i].color.b);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, 32 + 64 * i + 60, sizeof spheres[i].material, &spheres[i].material);
    }
  }

  {
    GLuint framebuffer;
    GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
    GLfloat zero[] = { 0.f, 0.f, 0.f, 0.f };
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32F, WIDTH, HEIGHT, 3);
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
    glDrawBuffers(1, draw_buffers);
    for (int i = 0; i < 3; i++) {
      glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, draw_buffers[0], texture, 0, i);
      glClearBufferfv(GL_COLOR, 0, zero);
      glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, draw_buffers[0], 0, 0, 0);
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebuffer);
  }

  glGenSamplers(1, &sampler);
  glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glBindSampler(0, sampler);

  {
    static float vertices[] = {
      // x,    y,   u,   v,

      -1.f, -1.f, 0.f, 0.f,
       1.f, -1.f, 1.f, 0.f,
      -1.f,  1.f, 0.f, 1.f,

       1.f, -1.f, 1.f, 0.f,
       1.f,  1.f, 1.f, 1.f,
      -1.f,  1.f, 0.f, 1.f,
    };
    GLint location;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
#if 0 // not implemented
    glBindVertexBuffer(0, vertex_buffer, 0, 4 * sizeof vertices[0]);
#endif
    location = glGetProgramResourceLocation(draw_program, GL_PROGRAM_INPUT, "aPos");
#if 0 // not implemented
    glVertexAttribFormat(location, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(location, 0);
#else
    glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 4 * sizeof vertices[0], reinterpret_cast<GLvoid *>(0));
#endif
    glEnableVertexAttribArray(location);
    location = glGetProgramResourceLocation(draw_program, GL_PROGRAM_INPUT, "aTexCoord");
#if 0 // not implemented
    glVertexAttribFormat(location, 2, GL_FLOAT, GL_FALSE, 2 * sizeof vertices[0]);
    glVertexAttribBinding(location, 0);
#else
    glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 4 * sizeof vertices[0], reinterpret_cast<GLvoid *>(2 * sizeof vertices[0]));
#endif
    glEnableVertexAttribArray(location);
  }
}

void render()
{
  static short counter = 0;
  GLuint index;
  GLuint binding;
  GLint location;
  GLint unit;
  int width, height;

  if (counter < std::numeric_limits<short>::max()) {
    GLenum props = GL_BUFFER_BINDING;
    GLint params;
    glUseProgram(compute_program);
    index = glGetProgramResourceIndex(compute_program, GL_SHADER_STORAGE_BLOCK, "Rands");
    glGetProgramResourceiv(compute_program, GL_SHADER_STORAGE_BLOCK, index, 1, &props, sizeof params, nullptr, &params);
    binding = params;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, rand_buffer);
    index = glGetProgramResourceIndex(compute_program, GL_SHADER_STORAGE_BLOCK, "Scene");
    glGetProgramResourceiv(compute_program, GL_SHADER_STORAGE_BLOCK, index, 1, &props, sizeof params, nullptr, &params);
    binding = params;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, scene_buffer);
    location = glGetProgramResourceLocation(compute_program, GL_UNIFORM, "uImage");
    glGetUniformiv(compute_program, location, &unit);
    glBindImageTexture(unit, texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);
    glDispatchCompute((WIDTH + LOCAL_SIZE_X - 1) / LOCAL_SIZE_X, (HEIGHT + LOCAL_SIZE_Y - 1) / LOCAL_SIZE_Y, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    counter++;
  }

  glfwGetFramebufferSize(window, &width, &height);
  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0, 0, width, height);
  glUseProgram(draw_program);
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(vertex_array);
  location = glGetProgramResourceLocation(draw_program, GL_UNIFORM, "uScale");
  glUniform1f(location, 1.f / counter);
  location = glGetProgramResourceLocation(draw_program, GL_UNIFORM, "uSampler");
  glUniform1i(location, 0);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glfwSwapBuffers(window);

  glfwPollEvents();
  if (glfwWindowShouldClose(window)) {
    quit = true;
  }
}

void shutdown()
{
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

#if __EMSCRIPTEN__

void next(void *)
{
  queue.front()();
  queue.pop_front();
}

void queuedSetup()
{
  setup();
  emscripten_async_call(next, nullptr, 0);
}

void queuedRender()
{
  render();
  if (!quit)
    return;
  emscripten_cancel_main_loop();
  emscripten_async_call(next, nullptr, 0);
}

void queuedShutdown()
{
  shutdown();
  emscripten_async_call(next, nullptr, 0);
}

int main()
{
  queue.push_back(std::bind(queuedSetup));
  queue.push_back(std::bind(emscripten_set_main_loop, queuedRender, 0, 0));
  queue.push_back(std::bind(queuedShutdown));
  emscripten_async_call(next, nullptr, 0);
  return EXIT_FAILURE;
}

#else

int main()
{
  setup();
  while (!quit)
    render();
  shutdown();
  return EXIT_FAILURE;
}

#endif
