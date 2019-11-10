#!/usr/bin/env bash

usage() {
    if [ "$*" ]; then
        echo "$*"
        echo
    fi
    echo "Usage: ${NAME} [--listen PORT] [--vnc VNC_HOST:PORT] [--cert CERT]"
    echo
    echo "Starts the WebSockets proxy and a mini-webserver and "
    echo "provides a cut-and-paste URL to go to."
    echo 
    echo "    --listen PORT         Port for proxy/webserver to listen on"
    echo "                          Default: 6080"
    echo "    --vnc VNC_HOST:PORT   VNC server host:port proxy target"
    echo "                          Default: localhost:5900"
    echo "    --cert CERT           Path to combined cert/key file"
    echo "                          Default: self.pem"
    echo "    --web WEB             Path to web files (e.g. vnc.html)"
    echo "                          Default: ./"
    exit 2
}

NAME="$(basename $0)"
HERE="$(cd "$(dirname "$0")" && pwd)"
PORT="6080"
VNC_DEST="localhost:5900"
CERT=""
WEB=""
proxy_pid=""

die() {
    echo "$*"
    exit 1
}

cleanup() {
    trap - TERM QUIT INT EXIT
    trap "true" CHLD   # Ignore cleanup messages
    echo
    if [ -n "${proxy_pid}" ]; then
        echo "Terminating WebSockets proxy (${proxy_pid})"
        kill ${proxy_pid}
    fi
}

# Process Arguments

# Arguments that only apply to chrooter itself
while [ "$*" ]; do
    param=$1; shift; OPTARG=$1
    case $param in
    --listen)  PORT="${OPTARG}"; shift            ;;
    --vnc)     VNC_DEST="${OPTARG}"; shift        ;;
    --cert)    CERT="${OPTARG}"; shift            ;;
    --web)     WEB="${OPTARG}"; shift            ;;
    -h|--help) usage                              ;;
    -*) usage "Unknown chrooter option: ${param}" ;;
    *) break                                      ;;
    esac
done

# Sanity checks
which netstat >/dev/null 2>&1 \
    || die "Must have netstat installed"

netstat -ltn | grep -qs "${PORT} .*LISTEN" \
    && die "Port ${PORT} in use. Try --listen PORT"

trap "cleanup" TERM QUIT INT EXIT

# Find vnc.html
if [ -n "${WEB}" ]; then
    if [ ! -e "${WEB}/vnc.html" ]; then
        die "Could not find ${WEB}/vnc.html"
    fi
elif [ -e "$(pwd)/vnc.html" ]; then
    WEB=$(pwd)
elif [ -e "${HERE}/../vnc.html" ]; then
    WEB=${HERE}/../
elif [ -e "${HERE}/vnc.html" ]; then
    WEB=${HERE}
elif [ -e "${HERE}/../share/novnc/vnc.html" ]; then
    WEB=${HERE}/../share/novnc/
else
    die "Could not find vnc.html"
fi

# Find self.pem
if [ -n "${CERT}" ]; then
    if [ ! -e "${CERT}" ]; then
        die "Could not find ${CERT}"
    fi
elif [ -e "$(pwd)/self.pem" ]; then
    CERT="$(pwd)/self.pem"
elif [ -e "${HERE}/../self.pem" ]; then
    CERT="${HERE}/../self.pem"
elif [ -e "${HERE}/self.pem" ]; then
    CERT="${HERE}/self.pem"
else
    echo "Warning: could not find self.pem"
fi

echo "Starting webserver and WebSockets proxy on port ${PORT}"
${HERE}/websockify --web ${WEB} ${CERT:+--cert ${CERT}} ${PORT} ${VNC_DEST} &
proxy_pid="$!"
sleep 1
if ! ps -p ${proxy_pid} >/dev/null; then
    proxy_pid=
    echo "Failed to start WebSockets proxy"
    exit 1
fi

echo -e "\n\nNavigate to this URL:\n"
echo -e "    http://$(hostname):${PORT}/vnc.html?host=$(hostname)&port=${PORT}\n"
echo -e "Press Ctrl-C to exit\n\n"

wait ${proxy_pid}
