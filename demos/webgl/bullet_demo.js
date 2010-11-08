print = alert;

function prepareBulletDemo() {
    Bullet.__Z10zz_preparev();
}

function simulateBulletDemo(dt) {
    Bullet.__Z11zz_simulatef(dt);
}

var tempVector3 = Bullet._.btVector3.__new__();
var tempQuaternion = Bullet._malloc(50);

function readBulletObject(i) {
  Bullet.__Z7zz_readiR9btVector3R12btQuaternion(i, tempVector3, tempQuaternion);
  return [ Bullet.FHEAP[Bullet._.btVector3.getX(tempVector3)],
           Bullet.FHEAP[Bullet._.btVector3.getY(tempVector3)],
           Bullet.FHEAP[Bullet._.btVector3.getZ(tempVector3)],
           Bullet.FHEAP[Bullet.__ZNK10btQuadWord4getXEv(tempQuaternion)],
           Bullet.FHEAP[Bullet.__ZNK10btQuadWord4getYEv(tempQuaternion)],
           Bullet.FHEAP[Bullet.__ZNK10btQuadWord4getZEv(tempQuaternion)],
           Bullet.FHEAP[Bullet.__ZNK12btQuaternion4getWEv(tempQuaternion)] ];
//print('pSIm: ' + Bullet.FHEAP[Bullet._.btVector3.getX(_t)] + ',' + Bullet.FHEAP[Bullet._.btVector3.getY(_t)] + ',' + Bullet.FHEAP[Bullet._.btVector3.getZ(_t)]);
}

function applyBulletObject(glgeObject, bulletObject) {
//alert(bulletObject);
  glgeObject.setLoc.apply(glgeObject, bulletObject.slice(0,3));
  glgeObject.setQuat.apply(glgeObject, bulletObject.slice(3,7));
}

