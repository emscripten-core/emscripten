.. _GitLab:

================================
Building and Deploying on GitLab
================================

`GitLab CI/CD <https://about.gitlab.com/product/continuous-integration/>`_ is a popular continuous integration service which offers free plans to everyone. Thanks to an `Alpine Linux package by Jakub Jirutka <https://pkgs.alpinelinux.org/packages?name=emscripten>`_ installing emscripten in GitLab CI/CD is literally a one line task.

A sample .gitlab-ci.yml
=======================

.. code-block:: yaml

    image: alpine:3.9

    before_script:
      - apk add emscripten make --repository=http://dl-cdn.alpinelinux.org/alpine/edge/testing

    pages:
      script:
      - make
      artifacts:
        paths:
        - public
      only:
        - main

Let's break it down:

.. code-block:: yaml

    before_script:
      - apk add emscripten make --repository=http://dl-cdn.alpinelinux.org/alpine/edge/testing

In the before_script stage we download the package from the Alpine Linux testing repository.

This step also contains the command to add an additional build tool *make*.

.. code-block:: yaml

    script:
      - make

In the script stage we can now run the commands we want. In this sample we are using *make*, but you can call *emcc* directly if you prefer.

For an example of this setup in practice, see `the Example Emscripten site using GitLab Pages <https://pages.gitlab.io/emscripten>`_.
