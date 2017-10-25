.. _Travis:

==============================
Building projects on Travis CI
==============================

`Travis CI <https://travis-ci.org/>`_ is a popular continuous integration service which offers free plans for open source projects. Thanks to a `Docker image by trzeci <https://hub.docker.com/r/trzeci/emscripten/>`_ installing emscripten in Travis CI is essentially a one line task.

A sample .travis.yml
====================

.. code-block:: yaml

    notifications:
      email: false

    language: node_js
    node_js:
      - node

    sudo: required

    services:
      - docker

    before_install:
      - docker run -dit --name emscripten -v $(pwd):/src trzeci/emscripten:sdk-incoming-64bit bash

    script:
      - docker exec -it emscripten make helloworld.js
      - make test

Let's break it down:

.. code-block:: yaml

    notifications:
      email: false

    language: node_js
    node_js:
      - node

    sudo: required

    services:
      - docker

These lines set up the basic settings for the Travis container. Most people do not want email notifications, but feel free to leave out those lines if you do.

``language: node_js`` and ``node_js: - node`` tell Travis we are a Node.js project, and that we want the latest stable Node release.

``sudo: required`` and ``services: - docker`` are required to enable Docker in the Travis container.

.. code-block:: yaml

    before_install:
      - docker run -dit --name emscripten -v $(pwd):/src trzeci/emscripten:sdk-incoming-64bit bash

In the before_install stage we download the Docker image, create a container with that image, and then give it the name ``emscripten``. The ``-dit`` options tell Docker that we want the container to run *bash* in the background.

This Docker image contains everything emscripten needs to run, as well as several additional build tools such as *make* and *cmake*. If you do not need them you can use the `emscripten-slim image <https://hub.docker.com/r/trzeci/emscripten-slim/>`_ instead, which excludes them and will be downloaded and installed slightly quicker. The emscripten versions available are listed at `the Docker Hub <https://hub.docker.com/r/trzeci/emscripten/tags/>`_.

.. code-block:: yaml

    script:
      - docker exec -it emscripten make helloworld.js
      - make test

In the script stage we can now run the commands we want, inside the Docker container we created earlier. In this sample we are using *make*, but you can call *emcc* directly if you prefer.

The Docker container is set up to use the same directories as Travis, so the second line uses the same *Makefile*, and can also depend on the output of the Docker command. If your test suite needs a later version of Node than what is installed by *emsdk* (Node v4), you will need to run the tests outside of Docker as a normal Travis command.

For an example of this setup in practice, see `the Travis page for emglken <https://travis-ci.org/curiousdannii/emglken>`_, which is also set up to use `Greenkeeper <https://greenkeeper.io/>`_ for automatic testing of dependency updates.
