Name: bullet
Description: Bullet Continuous Collision Detection and Physics Library
Requires:
Version: @BULLET_VERSION@
Libs: -L@LIB_DESTINATION@ -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath
Cflags: @BULLET_DOUBLE_DEF@ -I@INCLUDE_INSTALL_DIR@
