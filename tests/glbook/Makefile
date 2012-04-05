# Straight forward Makefile to compile all examples in a row

INCDIR=-I./Common
LIBS=-lGLESv2 -lEGL -lm -lX11

COMMONSRC=./Common/esShader.c    \
          ./Common/esTransform.c \
          ./Common/esShapes.c    \
          ./Common/esUtil.c
COMMONHRD=esUtil.h

CH02SRC=./Chapter_2/Hello_Triangle/Hello_Triangle.c
CH08SRC=./Chapter_8/Simple_VertexShader/Simple_VertexShader.c
CH09SRC1=./Chapter_9/Simple_Texture2D/Simple_Texture2D.c
CH09SRC2=./Chapter_9/MipMap2D/MipMap2D.c
CH09SRC3=./Chapter_9/Simple_TextureCubemap/Simple_TextureCubemap.c
CH09SRC4=./Chapter_9/TextureWrap/TextureWrap.c
CH10SRC=./Chapter_10/MultiTexture/MultiTexture.c
CH11SRC=./Chapter_11/Multisample/Multisample.c
CH11SRC2=./Chapter_11/Stencil_Test/Stencil_Test.c
CH13SRC2=./Chapter_13/ParticleSystem/ParticleSystem.c

default: all

all: ./Chapter_2/Hello_Triangle/CH02_HelloTriangle.bc \
     ./Chapter_8/Simple_VertexShader/CH08_SimpleVertexShader.bc \
     ./Chapter_9/Simple_Texture2D/CH09_SimpleTexture2D.bc \
     ./Chapter_9/MipMap2D/CH09_MipMap2D.bc \
     ./Chapter_9/Simple_TextureCubemap/CH09_TextureCubemap.bc \
     ./Chapter_9/TextureWrap/CH09_TextureWrap.bc \
     ./Chapter_10/MultiTexture/CH10_MultiTexture.bc \
     ./Chapter_13/ParticleSystem/CH13_ParticleSystem.bc
#     ./Chapter_11/Multisample/CH11_Multisample.bc \
#     ./Chapter_11/Stencil_Test/CH11_Stencil_Test.bc \

clean:
	find . -name "CH??_*" | xargs rm -f

./Chapter_2/Hello_Triangle/CH02_HelloTriangle.bc: ${COMMONSRC} ${COMMONHDR} ${CH02SRC}
	$(CC) ${COMMONSRC} ${CH02SRC} -o $@ ${INCDIR} ${LIBS}
./Chapter_8/Simple_VertexShader/CH08_SimpleVertexShader.bc: ${COMMONSRC} ${COMMONHDR} ${CH08SRC}
	$(CC) ${COMMONSRC} ${CH08SRC} -o ./$@ ${INCDIR} ${LIBS}
./Chapter_9/Simple_Texture2D/CH09_SimpleTexture2D.bc: ${COMMONSRC} ${COMMONHDR} ${CH09SRC1}
	$(CC) ${COMMONSRC} ${CH09SRC1} -o ./$@ ${INCDIR} ${LIBS}
./Chapter_9/MipMap2D/CH09_MipMap2D.bc: ${COMMONSRC} ${COMMONHDR} ${CH09SRC2}
	$(CC) ${COMMONSRC} ${CH09SRC2} -o ./$@ ${INCDIR} ${LIBS}
./Chapter_9/Simple_TextureCubemap/CH09_TextureCubemap.bc: ${COMMONSRC} ${COMMONHDR} ${CH09SRC3}
	$(CC) ${COMMONSRC} ${CH09SRC3} -o ./$@ ${INCDIR} ${LIBS}
./Chapter_9/TextureWrap/CH09_TextureWrap.bc: ${COMMONSRC} ${COMMONHDR} ${CH09SRC4}
	$(CC) ${COMMONSRC} ${CH09SRC4} -o ./$@ ${INCDIR} ${LIBS}
./Chapter_10/MultiTexture/CH10_MultiTexture.bc: ${COMMONSRC} ${COMMONHDR} ${CH10SRC}
	$(CC) ${COMMONSRC} ${CH10SRC} -o ./$@ ${INCDIR} ${LIBS}
./Chapter_11/Multisample/CH11_Multisample.bc: ${COMMONSRC} ${COMMONHDR} ${CH11SRC}
	$(CC) ${COMMONSRC} ${CH11SRC} -o ./$@ ${INCDIR} ${LIBS}
./Chapter_11/Stencil_Test/CH11_Stencil_Test.bc: ${COMMONSRC} ${COMMONHDR} ${CH11SRC2}
	$(CC) ${COMMONSRC} ${CH11SRC2} -o ./$@ ${INCDIR} ${LIBS}
./Chapter_13/Noise3D/CH13_Noise3D.bc: ${COMMONSRC} ${COMMONHDR} ${CH13SRC1}
	$(CC) ${COMMONSRC} ${CH13SRC1} -o ./$@ ${INCDIR} ${LIBS}
./Chapter_13/ParticleSystem/CH13_ParticleSystem.bc: ${COMMONSRC} ${COMMONHDR} ${CH13SRC2}
	$(CC) ${COMMONSRC} ${CH13SRC2} -o ./$@ ${INCDIR} ${LIBS}
