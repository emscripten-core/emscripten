# For travis
FROM ubuntu:16.04
SHELL ["/bin/bash", "-c"]
ENV DEBIAN_FRONTEND noninteractive
RUN mkdir -p /root/emscripten/
COPY . /root/emscripten/

RUN cd /root/ \
 && apt-get update \
 && apt-get install -y python python-pip wget git cmake build-essential software-properties-common \
 && pip install --upgrade pip \
 && pip install lit \
 && add-apt-repository ppa:webupd8team/java \
 && apt-get update \
 && echo debconf shared/accepted-oracle-license-v1-1 select true | debconf-set-selections \
 && echo debconf shared/accepted-oracle-license-v1-1 seen true | debconf-set-selections \
 && apt-get install -y oracle-java9-installer \
 && wget https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz \
 && tar -xf emsdk-portable.tar.gz \
 && pushd emsdk-portable \
 && ./emsdk update \
 && ./emsdk install latest \
 && ./emsdk activate latest \
 && popd \
 && echo EMSCRIPTEN_ROOT="'/root/emscripten/'" >> .emscripten

ARG TEST_TARGET
RUN python /root/emscripten/tests/runner.py $TEST_TARGET skip:ALL.test_sse1_full skip:ALL.test_sse2_full skip:ALL.test_sse3_full skip:ALL.test_ssse3_full skip:ALL.test_sse4_1_full skip:other.test_native_link_error_message skip:other.test_bad_triple skip:ALL.test_binaryen skip:other.test_binaryen* skip:other.test_on_abort skip:other.test_sysconf_phys_pages skip:other.test_wasm_targets skip:other.test_symbol_map skip:other.test_legalize_js_ffi
