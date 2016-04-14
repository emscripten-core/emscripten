## websockify: WebSockets support for any application/server

websockify was formerly named wsproxy and was part of the
[noVNC](https://github.com/kanaka/noVNC) project.

At the most basic level, websockify just translates WebSockets traffic
to normal socket traffic. Websockify accepts the WebSockets handshake,
parses it, and then begins forwarding traffic between the client and
the target in both directions.

### News/help/contact

Notable commits, announcements and news are posted to
<a href="http://www.twitter.com/noVNC">@noVNC</a>

If you are a websockify developer/integrator/user (or want to be)
please join the <a
href="https://groups.google.com/forum/?fromgroups#!forum/novnc">noVNC/websockify
discussion group</a>

Bugs and feature requests can be submitted via [github
issues](https://github.com/kanaka/websockify/issues).

If you want to show appreciation for websockify you could donate to a great
non-profits such as: [Compassion
International](http://www.compassion.com/), [SIL](http://www.sil.org),
[Habitat for Humanity](http://www.habitat.org), [Electronic Frontier
Foundation](https://www.eff.org/), [Against Malaria
Foundation](http://www.againstmalaria.com/), [Nothing But
Nets](http://www.nothingbutnets.net/), etc. Please tweet <a
href="http://www.twitter.com/noVNC">@noVNC</a> if you do.

### WebSockets binary data

Starting with websockify 0.5.0, only the HyBi / IETF
6455 WebSocket protocol is supported.

Websockify negotiates whether to base64 encode traffic to and from the
client via the subprotocol header (Sec-WebSocket-Protocol). The valid
subprotocol values are 'binary' and 'base64' and if the client sends
both then the server (the python implementation) will prefer 'binary'.
The 'binary' subprotocol indicates that the data will be sent raw
using binary WebSocket frames. Some HyBi clients (such as the Flash
fallback and older Chrome and iOS versions) do not support binary data
which is why the negotiation is necessary.


### Encrypted WebSocket connections (wss://)

To encrypt the traffic using the WebSocket 'wss://' URI scheme you need to
generate a certificate and key for Websockify to load. By default, Websockify
loads a certificate file name `self.pem` but the `--cert=CERT` and `--key=KEY`
options can override the file name. You can generate a self-signed certificate
using openssl. When asked for the common name, use the hostname of the server
where the proxy will be running:

```
openssl req -new -x509 -days 365 -nodes -out self.pem -keyout self.pem
```

For a self-signed certificate to work, you need to make your client/browser
understand it. You can do this by installing it as accepted certificate, or by
using that same certificate for a HTTPS connection to which you navigate first
and approve. Browsers generally don't give you the "trust certificate?" prompt
by opening a WSS socket with invalid certificate, hence you need to have it
acccept it by either of those two methods.

If you have a commercial/valid SSL certificate with one ore more intermediate
certificates, concat them into one file, server certificate first, then the
intermediate(s) from the CA, etc. Point to this file with the `--cert` option
and then also to the key with `--key`. Finally, use `--ssl-only` as needed.


### Websock Javascript library


The `include/websock.js` Javascript library library provides a Websock
object that is similar to the standard WebSocket object but Websock
enables communication with raw TCP sockets (i.e. the binary stream)
via websockify. This is accomplished by base64 encoding the data
stream between Websock and websockify.

Websock has built-in receive queue buffering; the message event
does not contain actual data but is simply a notification that
there is new data available. Several rQ* methods are available to
read binary data off of the receive queue.

The Websock API is documented on the [websock.js API wiki page](https://github.com/kanaka/websockify/wiki/websock.js)

See the "Wrap a Program" section below for an example of using Websock
and websockify as a browser telnet client (`wstelnet.html`).


### Additional websockify features

These are not necessary for the basic operation.

* Daemonizing: When the `-D` option is specified, websockify runs
  in the background as a daemon process.

* SSL (the wss:// WebSockets URI): This is detected automatically by
  websockify by sniffing the first byte sent from the client and then
  wrapping the socket if the data starts with '\x16' or '\x80'
  (indicating SSL).

* Flash security policy: websockify detects flash security policy
  requests (again by sniffing the first packet) and answers with an
  appropriate flash security policy response (and then closes the
  port). This means no separate flash security policy server is needed
  for supporting the flash WebSockets fallback emulator.

* Session recording: This feature that allows recording of the traffic
  sent and received from the client to a file using the `--record`
  option.

* Mini-webserver: websockify can detect and respond to normal web
  requests on the same port as the WebSockets proxy and Flash security
  policy. This functionality is activated with the `--web DIR` option
  where DIR is the root of the web directory to serve.

* Wrap a program: see the "Wrap a Program" section below.

* Log files: websockify can save all logging information in a file.
  This functionality is activated with the `--log-file FILE` option
  where FILE is the file where the logs should be saved.

### Implementations of websockify

The primary implementation of websockify is in python. There are
several alternate implementations in other languages (C, Node.js,
Clojure, Ruby) in the `other/` subdirectory (with varying levels of
functionality).

In addition there are several other external projects that implement
the websockify "protocol". See the alternate implementation [Feature
Matrix](https://github.com/kanaka/websockify/wiki/Feature_Matrix) for
more information.


### Wrap a Program

In addition to proxying from a source address to a target address
(which may be on a different system), websockify has the ability to
launch a program on the local system and proxy WebSockets traffic to
a normal TCP port owned/bound by the program.

The is accomplished with a small LD_PRELOAD library (`rebind.so`)
which intercepts bind() system calls by the program. The specified
port is moved to a new localhost/loopback free high port. websockify
then proxies WebSockets traffic directed to the original port to the
new (moved) port of the program.

The program wrap mode is invoked by replacing the target with `--`
followed by the program command line to wrap.

    `./run 2023 -- PROGRAM ARGS`

The `--wrap-mode` option can be used to indicate what action to take
when the wrapped program exits or daemonizes.

Here is an example of using websockify to wrap the vncserver command
(which backgrounds itself) for use with
[noVNC](https://github.com/kanaka/noVNC):

    `./run 5901 --wrap-mode=ignore -- vncserver -geometry 1024x768 :1`

Here is an example of wrapping telnetd (from krb5-telnetd). telnetd
exits after the connection closes so the wrap mode is set to respawn
the command:

    `sudo ./run 2023 --wrap-mode=respawn -- telnetd -debug 2023`

The `wstelnet.html` page demonstrates a simple WebSockets based telnet
client (use 'localhost' and '2023' for the host and port
respectively).


### Building the Python ssl module (for python 2.5 and older)

* Install the build dependencies. On Ubuntu use this command:

    `sudo aptitude install python-dev bluetooth-dev`

* At the top level of the websockify repostory, download, build and
  symlink the ssl module:

    `wget --no-check-certificate http://pypi.python.org/packages/source/s/ssl/ssl-1.15.tar.gz`

    `tar xvzf ssl-1.15.tar.gz`

    `cd ssl-1.15`

    `make`

    `cd ../`

    `ln -sf ssl-1.15/build/lib.linux-*/ssl ssl`

