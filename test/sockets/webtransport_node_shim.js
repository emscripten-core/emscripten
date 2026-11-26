// A stand-in for the browser's WebTransport API under node, for testing
// -sWEBTRANSPORT_SOCKETS (node has no WebTransport). Each session is a plain TCP
// connection to the host and port in the URL, and its bidirectional stream is
// that connection as a pair of Web Streams, with backpressure in both
// directions.
if (typeof process == 'object' && !globalThis.WebTransport) {
  const net = require('node:net');

  // Like a QUIC stream, each direction ends independently: the readable side
  // ends at the peer's FIN, even while our side is still open. (Duplex.toWeb
  // only ends it once both directions are done.)
  const toStreams = (socket) => ({
    readable: new ReadableStream({
      start(controller) {
        socket.on('data', (chunk) => {
          controller.enqueue(new Uint8Array(chunk));
          if (controller.desiredSize <= 0) socket.pause(); // until pulled again
        });
        socket.on('end', () => controller.close());
        socket.on('error', (e) => controller.error(e));
      },
      pull() {
        socket.resume();
      },
      cancel() {
        socket.destroy();
      },
    }, { highWaterMark: 65536, size: (chunk) => chunk.byteLength }),
    writable: new WritableStream({
      // Resolves once node has handed the chunk to the OS, so a peer that
      // doesn't read holds up later writes.
      write(chunk) {
        return new Promise((resolve, reject) => socket.write(chunk, (e) => e ? reject(e) : resolve()));
      },
      close() {
        return new Promise((resolve) => socket.end(resolve));
      },
      abort() {
        socket.destroy();
      },
    }),
  });

  globalThis.WebTransport = class {
    constructor(url) {
      const { hostname, port } = new URL(url);
      this.socket = net.connect({ host: hostname, port: Number(port), allowHalfOpen: true });
      this.ready = new Promise((resolve, reject) => {
        this.socket.once('connect', resolve);
        this.socket.once('error', reject);
      });
      this.closed = new Promise((resolve, reject) => {
        this.socket.once('close', (hadError) => hadError ? reject(new Error('connection failed')) : resolve({}));
      });
    }

    async createBidirectionalStream() {
      return toStreams(this.socket);
    }

    close() {
      this.socket.destroy();
    }
  };
}
