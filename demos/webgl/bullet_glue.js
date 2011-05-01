/**
  Glue for Bullet demo.
 */

var Bullet = Module;

//print = function(text) {
//  throw 'fail: ' + text;
//}

function prepareBulletDemo() {
  __Z10zz_preparev();
}

function simulateBulletDemo(dt) {
  __ZN23btDiscreteDynamicsWorld14stepSimulationEfif(Bullet.IHEAP[_zz_dynamicsWorld], dt, 2, 1/60)
}

var tempVector3 = _malloc(40);
var tempQuaternion = _malloc(50);

var QUANTUM = 1;

function readBulletObject(i, pos, quat) {
  // TODO: Do not create new objects all the time
  __Z7zz_readiR9btVector3R12btQuaternion(i, tempVector3, tempQuaternion);
  pos[0] = Bullet.FHEAP[tempVector3+0*QUANTUM];
  pos[1] = Bullet.FHEAP[tempVector3+1*QUANTUM];
  pos[2] = Bullet.FHEAP[tempVector3+2*QUANTUM];
  quat.x = Bullet.FHEAP[tempQuaternion+0*QUANTUM];
  quat.y = Bullet.FHEAP[tempQuaternion+1*QUANTUM];
  quat.z = Bullet.FHEAP[tempQuaternion+2*QUANTUM];
  quat.w = Bullet.FHEAP[tempQuaternion+3*QUANTUM];
}

function restartBullet() {
  run([]);
  prepareBulletDemo();
}

// Startup

prepareBulletDemo();

