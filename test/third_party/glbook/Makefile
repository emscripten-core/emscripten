# Straight forward Makefile to compile all examples in a row

INCDIR=-I./Common
LIBS=-lGLESv2 -lEGL -lm -lX11

COMMONSRC=Common/esShader.c    \
          Common/esTransform.c \
          Common/esShapes.c    \
          Common/esUtil.c
COMMONHRD=esUtil.h

CH02SRC=Chapter_2/Hello_Triangle/Hello_Triangle.c
CH08SRC=Chapter_8/Simple_VertexShader/Simple_VertexShader.c
CH09SRC1=Chapter_9/Simple_Texture2D/Simple_Texture2D.c
CH09SRC2=Chapter_9/MipMap2D/MipMap2D.c
CH09SRC3=Chapter_9/Simple_TextureCubemap/Simple_TextureCubemap.c
CH09SRC4=Chapter_9/TextureWrap/TextureWrap.c
CH10SRC=Chapter_10/MultiTexture/MultiTexture.c
CH11SRC=Chapter_11/Multisample/Multisample.c
CH11SRC2=Chapter_11/Stencil_Test/Stencil_Test.c
CH13SRC2=Chapter_13/ParticleSystem/ParticleSystem.c

default: all

all: Chapter_2/Hello_Triangle/CH02_HelloTriangle.o \
     Chapter_8/Simple_VertexShader/CH08_SimpleVertexShader.o \
     Chapter_9/Simple_Texture2D/CH09_SimpleTexture2D.o \
     Chapter_9/MipMap2D/CH09_MipMap2D.o \
     Chapter_9/Simple_TextureCubemap/CH09_TextureCubemap.o \
     Chapter_9/TextureWrap/CH09_TextureWrap.o \
     Chapter_10/MultiTexture/CH10_MultiTexture.o \
     Chapter_13/ParticleSystem/CH13_ParticleSystem.o
#     Chapter_11/Multisample/CH11_Multisample.o \
#     Chapter_11/Stencil_Test/CH11_Stencil_Test.o \

clean:
	find . -name "CH??_*" | xargs rm -f

Chapter_2/Hello_Triangle/CH02_HelloTriangle.o: ${COMMONSRC} ${COMMONHDR} ${CH02SRC}
	$(CC) ${CFLAGS} ${COMMONSRC} ${CH02SRC} -r -o $@ ${INCDIR} ${LIBS}
Chapter_8/Simple_VertexShader/CH08_SimpleVertexShader.o: ${COMMONSRC} ${COMMONHDR} ${CH08SRC}
	$(CC) ${CFLAGS} ${COMMONSRC} ${CH08SRC} -r -o ./$@ ${INCDIR} ${LIBS}
Chapter_9/Simple_Texture2D/CH09_SimpleTexture2D.o: ${COMMONSRC} ${COMMONHDR} ${CH09SRC1}
	$(CC) ${CFLAGS} ${COMMONSRC} ${CH09SRC1} -r -o ./$@ ${INCDIR} ${LIBS}
Chapter_9/MipMap2D/CH09_MipMap2D.o: ${COMMONSRC} ${COMMONHDR} ${CH09SRC2}
	$(CC) ${CFLAGS} ${COMMONSRC} ${CH09SRC2} -r -o ./$@ ${INCDIR} ${LIBS}
Chapter_9/Simple_TextureCubemap/CH09_TextureCubemap.o: ${COMMONSRC} ${COMMONHDR} ${CH09SRC3}
	$(CC) ${CFLAGS} ${COMMONSRC} ${CH09SRC3} -r -o ./$@ ${INCDIR} ${LIBS}
Chapter_9/TextureWrap/CH09_TextureWrap.o: ${COMMONSRC} ${COMMONHDR} ${CH09SRC4}
	$(CC) ${CFLAGS} ${COMMONSRC} ${CH09SRC4} -r -o ./$@ ${INCDIR} ${LIBS}
Chapter_10/MultiTexture/CH10_MultiTexture.o: ${COMMONSRC} ${COMMONHDR} ${CH10SRC}
	$(CC) ${CFLAGS} ${COMMONSRC} ${CH10SRC} -r -o ./$@ ${INCDIR} ${LIBS}
Chapter_11/Multisample/CH11_Multisample.o: ${COMMONSRC} ${COMMONHDR} ${CH11SRC}
	$(CC) ${CFLAGS} ${COMMONSRC} ${CH11SRC} -r -o ./$@ ${INCDIR} ${LIBS}
Chapter_11/Stencil_Test/CH11_Stencil_Test.o: ${COMMONSRC} ${COMMONHDR} ${CH11SRC2}
	$(CC) ${CFLAGS} ${COMMONSRC} ${CH11SRC2} -r -o ./$@ ${INCDIR} ${LIBS}
Chapter_13/Noise3D/CH13_Noise3D.o: ${COMMONSRC} ${COMMONHDR} ${CH13SRC1}
	$(CC) ${CFLAGS} ${COMMONSRC} ${CH13SRC1} -r -o ./$@ ${INCDIR} ${LIBS}
Chapter_13/ParticleSystem/CH13_ParticleSystem.o: ${COMMONSRC} ${COMMONHDR} ${CH13SRC2}
	$(CC) ${CFLAGS} ${COMMONSRC} ${CH13SRC2} -r -o ./$@ ${INCDIR} ${LIBS}
