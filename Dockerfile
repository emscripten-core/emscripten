# For travis
FROM buildpack-deps:xenial
SHELL ["/bin/bash", "-c"]
ENV DEBIAN_FRONTEND=noninteractive LANG=C.UTF-8
RUN mkdir -p /root/emscripten/
COPY . /root/emscripten/

RUN cd /root/ \
 && apt-get update \
 && apt-get install -y python cmake build-essential openjdk-9-jre-headless \
 && wget https://github.com/juj/emsdk/archive/master.tar.gz \
 && tar -xf master.tar.gz \
 && pushd emsdk-master \
 && ./emsdk update-tags \
 && ./emsdk install latest \
 && ./emsdk activate latest \
 && popd \
 && echo BINARYEN_ROOT="''" >> .emscripten

ARG TEST_TARGET
RUN export EMTEST_BROWSER=0 \
 && python /root/emscripten/tests/runner.py $TEST_TARGET skip:other.test_native_link_error_message skip:other.test_emcc_v skip:other.test_single_file skip:other.test_vorbis skip:other.test_eval_ctors

# skip:other.test_emcc_v to prevent tool version differences from breaking CI
