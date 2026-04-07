import createModule from '../../out/test/out.mjs';
import * as fs from 'node:fs';
const outFd = fs.openSync('/proc/self/fd/1', 'w');

const arg = process.argv.at(-1);
if (arg[0] === "1") {
  fs.closeSync(0);
  fs.openSync("/dev/tty", 'r');
} else {
  fs.closeSync(0);
  fs.openSync("/dev/null", 'r');
}

if (arg[1] === "1") {
  fs.closeSync(1);
  fs.openSync("/dev/tty", 'w');
} else {
  fs.closeSync(1);
  fs.openSync("/dev/null", 'w');
}

if (arg[2] === "1") {
  fs.closeSync(2);
  fs.openSync("/dev/tty", 'w');
} else {
  fs.closeSync(2);
  fs.openSync("/dev/null", 'w');
}

await createModule({ outFd });
