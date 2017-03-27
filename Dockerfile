# For travis
FROM ubuntu:16.04
RUN apt-get update \
 && apt-get install -y python wget \
 && wget https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz \
 && tar -xvf emsdk-portable.tar.gz \
 && cd emsdk-portable \
 && ./emsdk update \
 && ./emsdk install latest \
 && ./emsdk activate latest \
 && cd .. \
 &&./emcc \
 && python tests/runner.py test_hello_world

