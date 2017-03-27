# For travis
FROM ubuntu:16.04
RUN apt-get update
RUN apt-get install -y python wget
RUN wget https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz
RUN tar -xvf emsdk-portable.tar.gz
RUN cd emsdk-portable && ./emsdk update
RUN cd emsdk-portable && ./emsdk install latest
RUN cd emsdk-portable && ./emsdk activate latest
RUN ls
RUN ./emcc # first use
RUN ./emcc -v # check s tuff
RUN python tests/runner.py test_hello_world

