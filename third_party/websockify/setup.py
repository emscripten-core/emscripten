from setuptools import setup, find_packages

version = '0.2.1'
name = 'websockify'
long_description = open("README.md").read() + "\n" + \
    open("CHANGES.txt").read() + "\n"

setup(name=name,
      version=version,
      description="Websockify.",
      long_description=long_description,
      classifiers=[
          "Programming Language :: Python",
        ],
      keywords='noVNC websockify',
      license='LGPLv3',
      url="https://github.com/kanaka/websockify",
      author="Joel Martin",
      author_email="github@martintribe.org",

      packages=['websockify'],
      include_package_data=True,
      install_requires=['numpy'],
      zip_safe=False,
      entry_points={
        'console_scripts': [
            'websockify = websockify.websocketproxy:websockify_init',
        ]
      },
    )
