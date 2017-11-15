# For travis
FROM buildpack-deps:xenial
SHELL ["/bin/bash", "-c"]
ENV DEBIAN_FRONTEND noninteractive
RUN mkdir -p /root/emscripten/
COPY . /root/emscripten/

RUN cd /root/ \
 && apt-get update \
 && apt-get install -y python python-pip cmake build-essential openjdk-9-jre-headless \
 && pip install --upgrade pip \
 && pip install lit \
 && wget https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz \
 && tar -xf emsdk-portable.tar.gz \
 && pushd emsdk-portable \
 && ./emsdk update \
 && ./emsdk install latest \
 && ./emsdk activate latest \
 && popd \
 && echo EMSCRIPTEN_ROOT="'/root/emscripten/'" >> .emscripten \
 && echo BINARYEN_ROOT="''" >> .emscripten

ARG TEST_TARGET
RUN export EMSCRIPTEN_BROWSER=0 \
 && python /root/emscripten/tests/runner.py $TEST_TARGET skip:ALL.test_sse1_full skip:ALL.test_sse2_full skip:ALL.test_sse3_full skip:ALL.test_ssse3_full skip:ALL.test_sse4_1_full skip:other.test_native_link_error_message skip:other.test_bad_triple
