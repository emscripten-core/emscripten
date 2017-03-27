# For travis
FROM ubuntu:16.04
RUN apt-get update
RUN apt-get install -y software-properties-common
RUN add-apt-repository -s -y ppa:fenics-packages/fenics
RUN apt-get update
RUN apt-get install -y python-dolfin python-numpy python-pip
RUN pip install matplotlib pytest pytest-cov

ADD . /root/
RUN cd /root/ && PYTHONPATH=$PWD:$PYTHONPATH MPLBACKEND=Agg pytest --cov maelstrom
RUN bash <(curl -s https://codecov.io/bash)


RUN wget https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz
RUN tar -xvf emsdk-portable.tar.gz
RUN cd emsdk-portable
RUN ./emsdk update
RUN ./emsdk install latest
RUN ./emsdk activate latest
RUN cd ..
#RUN source ./emsdk_env.sh
RUN ./emcc # first use
RUN ./emcc -v # check s tuff
RUN python tests/runner.py test_hello_world

