#### Visit Bullet library ibmsdk dirs and build  code

DIRS :=				\
../BulletCollision/ibmsdk	\
../BulletDynamics/ibmsdk	\
../LinearMath/ibmsdk

IBM_CELLSDK_VERSION := $(shell if [ -d /opt/cell ]; then echo "3.0"; fi)

ifeq ("$(IBM_CELLSDK_VERSION)","3.0")
        CELL_TOP ?= /opt/cell/sdk
        include $(CELL_TOP)/buildutils/make.footer
else
        CELL_TOP ?= /opt/ibm/cell-sdk/prototype
        include $(CELL_TOP)/make.footer
endif
