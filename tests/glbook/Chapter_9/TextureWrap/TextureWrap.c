//
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com
//

// TextureWrap.c
//
//    This is an example that demonstrates the three texture
//    wrap modes available on 2D textures
//
#include <stdlib.h>
#include "esUtil.h"

typedef struct
{
   // Handle to a program object
   GLuint programObject;

   // Attribute locations
   GLint  positionLoc;
   GLint  texCoordLoc;

   // Sampler location
   GLint samplerLoc;

   // Offset location
   GLint offsetLoc;

   // Texture handle
   GLuint textureId;

   GLuint vertexObject, indexObject;

} UserData;

///
//  Generate an RGB8 checkerboard image
//
GLubyte* GenCheckImage( int width, int height, int checkSize )
{
   int x,
       y;
   GLubyte *pixels = malloc( width * height * 3 );
   
   if ( pixels == NULL )
      return NULL;

   for ( y = 0; y < height; y++ )
      for ( x = 0; x < width; x++ )
      {
         GLubyte rColor = 0;
         GLubyte bColor = 0;

         if ( ( x / checkSize ) % 2 == 0 )
         {
            rColor = 255 * ( ( y / checkSize ) % 2 );
            bColor = 255 * ( 1 - ( ( y / checkSize ) % 2 ) );
         }
         else
         {
            bColor = 255 * ( ( y / checkSize ) % 2 );
            rColor = 255 * ( 1 - ( ( y / checkSize ) % 2 ) );
         }

         pixels[(y * height + x) * 3] = rColor;
         pixels[(y * height + x) * 3 + 1] = 0;
         pixels[(y * height + x) * 3 + 2] = bColor; 
      } 

   return pixels;
}

///
// Create a mipmapped 2D texture image 
//
GLuint CreateTexture2D( )
{
   // Texture object handle
   GLuint textureId;
   int    width = 256,
          height = 256;
   GLubyte *pixels;
      
   pixels = GenCheckImage( width, height, 64 );
   if ( pixels == NULL )
      return 0;

   // Use tightly packed data
   glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );

   // Generate a texture object
   glGenTextures ( 1, &textureId );

   // Bind the texture object
   glBindTexture ( GL_TEXTURE_2D, textureId );

   // Load mipmap level 0
   glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, width, height, 
                  0, GL_RGB, GL_UNSIGNED_BYTE, pixels );
   
   // Set the filtering mode
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

   return textureId;

}


///
// Initialize the shader and program object
//
int Init ( ESContext *esContext )
{
   esContext->userData = malloc(sizeof(UserData));
	
   UserData *userData = esContext->userData;
   GLbyte vShaderStr[] =
      "uniform float u_offset;      \n"
      "attribute vec4 a_position;   \n"
      "attribute vec2 a_texCoord;   \n"
      "varying vec2 v_texCoord;     \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = a_position; \n"
      "   gl_Position.x += u_offset;\n"
      "   v_texCoord = a_texCoord;  \n"
      "}                            \n";
   
   GLbyte fShaderStr[] =  
      "precision mediump float;                            \n"
      "varying vec2 v_texCoord;                            \n"
      "uniform sampler2D s_texture;                        \n"
      "void main()                                         \n"
      "{                                                   \n"
      "  gl_FragColor = texture2D( s_texture, v_texCoord );\n"
      "}                                                   \n";

   // Load the shaders and get a linked program object
   userData->programObject = esLoadProgram ( vShaderStr, fShaderStr );

   // Get the attribute locations
   userData->positionLoc = glGetAttribLocation ( userData->programObject, "a_position" );
   userData->texCoordLoc = glGetAttribLocation ( userData->programObject, "a_texCoord" );
   
   // Get the sampler location
   userData->samplerLoc = glGetUniformLocation ( userData->programObject, "s_texture" );

   // Get the offset location
   userData->offsetLoc = glGetUniformLocation( userData->programObject, "u_offset" );

   // Load the texture
   userData->textureId = CreateTexture2D ();

   // Setup the vertex data
   GLfloat vVertices[] = { -0.3,  0.3, 0.0, 1.0,  // Position 0
                           -1.0,  -1.0,           // TexCoord 0
                           -0.3, -0.3, 0.0, 1.0,  // Position 1
                           -1.0,  2.0,            // TexCoord 1
                            0.3, -0.3, 0.0, 1.0,  // Position 2
                            2.0,  2.0,            // TexCoord 2
                            0.3,  0.3, 0.0, 1.0,  // Position 3
                            2.0,  -1.0            // TexCoord 3
                         };
   GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

   glGenBuffers(1, &userData->vertexObject);
   glBindBuffer ( GL_ARRAY_BUFFER, userData->vertexObject );
   glBufferData ( GL_ARRAY_BUFFER, 6 * 4 * 4, vVertices, GL_STATIC_DRAW );

   glGenBuffers(1, &userData->indexObject);
   glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, userData->indexObject );
   glBufferData ( GL_ELEMENT_ARRAY_BUFFER, 6 * 2, indices, GL_STATIC_DRAW );

   glClearColor ( 0.0, 0.0, 0.0, 1.0 );

   return GL_TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw ( ESContext *esContext )
{
   UserData *userData = esContext->userData;

   // Set the viewport
   glViewport ( 0, 0, esContext->width, esContext->height );
   
   // Clear the color buffer
   glClear ( GL_COLOR_BUFFER_BIT );

   // Use the program object
   glUseProgram ( userData->programObject );

   // Load the vertex position
   glBindBuffer ( GL_ARRAY_BUFFER, userData->vertexObject );
   glVertexAttribPointer ( userData->positionLoc, 4, GL_FLOAT, 
                           GL_FALSE, 6 * sizeof(GLfloat), 0 );
   // Load the texture coordinate
   glVertexAttribPointer ( userData->texCoordLoc, 2, GL_FLOAT,
                           GL_FALSE, 6 * sizeof(GLfloat), 4 * sizeof(GLfloat) );

   glEnableVertexAttribArray ( userData->positionLoc );
   glEnableVertexAttribArray ( userData->texCoordLoc );

   // Bind the texture
   glActiveTexture ( GL_TEXTURE0 );
   glBindTexture ( GL_TEXTURE_2D, userData->textureId );

   // Set the sampler texture unit to 0
   glUniform1i ( userData->samplerLoc, 0 );

   // Draw quad with repeat wrap mode
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
   glUniform1f ( userData->offsetLoc, -0.7f );   
   glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, userData->indexObject );
   glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0 );

   // Draw quad with clamp to edge wrap mode
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
   glUniform1f ( userData->offsetLoc, 0.0f );
   glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0 );

   // Draw quad with mirrored repeat
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
   glUniform1f ( userData->offsetLoc, 0.7f );
   glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0 );

}

///
// Cleanup
//
void ShutDown ( ESContext *esContext )
{
   UserData *userData = esContext->userData;

   // Delete texture object
   glDeleteTextures ( 1, &userData->textureId );

   // Delete program object
   glDeleteProgram ( userData->programObject );

   free ( esContext->userData);
}

int main ( int argc, char *argv[] )
{
   ESContext esContext;
   UserData  userData;

   esInitContext ( &esContext );
   esContext.userData = &userData;

   esCreateWindow ( &esContext, "MipMap 2D", 320, 240, ES_WINDOW_RGB );

   if ( !Init ( &esContext ) )
      return 0;

   esRegisterDrawFunc ( &esContext, Draw );

   esMainLoop ( &esContext );

   ShutDown ( &esContext );
}

