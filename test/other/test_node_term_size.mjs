import createModule from '../../out/test/out.mjs';
import * as fs from 'node:fs';
import * as tty from 'node:tty';

const outFd = fs.openSync('/proc/self/fd/1', 'w');

fs.closeSync(1);
fs.openSync("/dev/tty", 1);

globalThis.process = Object.assign({}, process, {stdout: new tty.WriteStream(1)});
process.stdout.fd = 1;
process.stdout.columns = 180;
process.stdout.rows = 50;

await createModule({ outFd });
