# For travis
FROM ubuntu:16.04
RUN apt-get update
RUN apt-get install -y python wget cmake
RUN wget https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz
RUN tar -xvf emsdk-portable.tar.gz
RUN cd emsdk-portable && ./emsdk update
RUN cd emsdk-portable && ./emsdk install latest || ls
RUN cat emsdk-portable/clang/tag-e1.37.9/build_tag-e1.37.9_64/CMakeFiles/CMakeOutput.log
RUN cat emsdk-portable/clang/tag-e1.37.9/build_tag-e1.37.9_64/CMakeFiles/CMakeError.log
RUN cd emsdk-portable && ./emsdk activate latest
RUN ls
RUN ./emcc # first use
RUN ./emcc -v # check s tuff
RUN python tests/runner.py test_hello_world

