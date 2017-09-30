# For travis
FROM ubuntu:16.04
SHELL ["/bin/bash", "-c"]

COPY . /

RUN apt-get update \
 && apt-get install -y python wget git \
 && wget https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz \
 && tar -xf emsdk-portable.tar.gz \
 && cd emsdk-portable \
 && ./emsdk update \
 && ./emsdk install latest \
 && ./emsdk activate latest \
 && source ./emsdk_env.sh \
 && cd .. \
 && ./emcc --version \
 && python tests/runner.py test_hello_world
