#!/usr/bin/env python2
# -*- coding: utf-8 -*-

import sys, glob
from distutils.core import setup

DESCRIPTION = (
    'Parse C++ header files and generate a data structure '
    'representing the class'
    )


CLASSIFIERS = [
    'Operating System :: OS Independent',
    'Programming Language :: Python',
    'Programming Language :: Python :: 2',
    'Programming Language :: Python :: 3',
    'Programming Language :: C++',
    'License :: OSI Approved :: BSD License',
    'Development Status :: 5 - Production/Stable',
    'Intended Audience :: Developers',
    'Topic :: Software Development',
    'Topic :: Software Development :: Code Generators',
    'Topic :: Software Development :: Compilers',
    'Topic :: Software Development :: Disassemblers'
    ]

setup(
    name = 'CppHeaderParser',
    version = '1.9',
    author = 'Jashua Cloutier',
    author_email = 'jashuac@bellsouth.net',
    url = 'http://sourceforge.net/projects/cppheaderparser/',
    description = DESCRIPTION,
    long_description = open('README.txt').read(),
    license = 'BSD',
    platforms = 'Platform Independent',
    packages = ['CppHeaderParser'],
    keywords = 'c++ header parser ply',
    classifiers = CLASSIFIERS,
    requires = ['ply'],
    package_data = { 'CppHeaderParser': ['README', 'README.html', 'doc/*.*', 'examples/*.*'], },
    )
