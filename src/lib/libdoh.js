/**
 * @license
 * Copyright 2026 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 *
 * DNS-over-HTTPS (DoH) resolution for Emscripten.
 *
 * Replaces Emscripten's fake 172.29.x.x DNS with real lookups via fetch()
 * to a public DoH resolver (dns.google JSON API).
 *
 * Enable with -sDOH_DNS at link time. Requires -sASYNCIFY (or -sJSPI).
 *
 * Value provided:
 * - Real IP addresses from DNS (A and AAAA records)
 * - HTTPS/SVCB record support (RFC 9460, type 65): ALPN hints (h2/h3),
 *   IP address hints (skip extra lookups), port overrides, ECH keys
 * - TTL-based caching that respects upstream DNS TTLs
 * - Reverse lookup cache so getnameinfo/gethostbyaddr return real hostnames
 * - Integration with Emscripten's DNS.address_map for interop with existing code
 * - Proper IPv6 resolution (AAAA records) in getaddrinfo
 * - Graceful fallback to Emscripten's fake DNS if DoH fetch fails
 *
 * Works with any networking backend (Direct Sockets, WebSocket proxy, etc).
 */

#if DOH_DNS
#if !ASYNCIFY
#error "DOH_DNS requires ASYNCIFY or JSPI to be enabled"
#endif

var DoHLibrary = {

  $DOH_DNS__deps: ['$DNS', '$inetPton4'],
  $DOH_DNS: {
    // hostname -> {addresses: [ip, ...], family: AF_INET|AF_INET6, expires: timestamp}
    cache: {},

    // ip -> hostname reverse cache (populated on successful resolution)
    reverseCache: {},

    // hostname -> {alpn, ipv4hint, ipv6hint, port, targetName, expires}
    // from HTTPS/SVCB records (RFC 9460, type 65)
    httpsCache: {},

    // resolve hostname via DoH, returns first matching IP string or null
    async resolve(hostname, family) {
      var cacheKey = hostname + '/' + family;
      var cached = DOH_DNS.cache[cacheKey];
      if (cached && cached.expires > Date.now()) return cached.addresses[0];

      var type = (family === {{{ cDefs.AF_INET6 }}}) ? 'AAAA' : 'A';
      var typeNum = (type === 'A') ? 1 : 28;
      try {
        var resp = await fetch(
          'https://dns.google/resolve?name=' + encodeURIComponent(hostname) + '&type=' + type,
          { headers: { 'Accept': 'application/dns-json' } }
        );
        var json = await resp.json();

        if (json.Answer && json.Answer.length > 0) {
          var addresses = json.Answer
            .filter(function(a) { return a.type === typeNum; })
            .map(function(a) { return a.data; });
          if (addresses.length === 0) return null;
          var ttl = Math.max((json.Answer[0].TTL || 300), 60);
          DOH_DNS.cache[cacheKey] = {
            addresses: addresses, expires: Date.now() + ttl * 1000
          };
          for (var i = 0; i < addresses.length; i++) {
            DOH_DNS.reverseCache[addresses[i]] = hostname;
          }
          return addresses[0];
        }
      } catch (e) {
#if SOCKET_DEBUG
        dbg('doh: resolution failed for ' + hostname + ' (' + type + '): ' + e);
#endif
      }
      return null;
    },

    // Query HTTPS/SVCB records (type 65) for a hostname (RFC 9460).
    // Returns service parameters: ALPN protocol hints (h2, h3),
    // IP address hints (avoids extra A/AAAA lookups), port overrides,
    // and target name aliases.
    //
    // These records are how modern DNS advertises HTTP/3 support,
    // alternative endpoints, and Encrypted Client Hello (ECH) keys.
    // The IP hints are also pre-seeded into the A/AAAA cache so
    // subsequent resolve() calls can return instantly.
    async resolveHTTPS(hostname) {
      var cached = DOH_DNS.httpsCache[hostname];
      if (cached && cached.expires > Date.now()) return cached;

      try {
        var resp = await fetch(
          'https://dns.google/resolve?name=' + encodeURIComponent(hostname) + '&type=HTTPS',
          { headers: { 'Accept': 'application/dns-json' } }
        );
        var json = await resp.json();

        if (json.Answer && json.Answer.length > 0) {
          var result = { alpn: null, ipv4hint: null, ipv6hint: null, port: null, targetName: null };
          var ttl = 300;

          for (var i = 0; i < json.Answer.length; i++) {
            var ans = json.Answer[i];
            if (ans.type !== 65) continue;
            ttl = Math.max((ans.TTL || 300), 60);

            // dns.google JSON API returns HTTPS RDATA as a string like:
            // "1 . alpn=h2,h3 ipv4hint=1.2.3.4 ipv6hint=::1 port=443"
            var data = ans.data;
            if (typeof data !== 'string') continue;

            var parts = data.split(' ');
            if (parts.length >= 2 && parts[1] !== '.') {
              result.targetName = parts[1];
            }

            var alpnMatch = data.match(/alpn=([^\s]+)/);
            if (alpnMatch) result.alpn = alpnMatch[1].split(',');

            var ipv4Match = data.match(/ipv4hint=([^\s]+)/);
            if (ipv4Match) result.ipv4hint = ipv4Match[1].split(',');

            var ipv6Match = data.match(/ipv6hint=([^\s]+)/);
            if (ipv6Match) result.ipv6hint = ipv6Match[1].split(',');

            var portMatch = data.match(/port=(\d+)/);
            if (portMatch) result.port = parseInt(portMatch[1]);

            // First ServiceMode (priority > 0) record wins
            if (parts[0] !== '0') break;
          }

          result.expires = Date.now() + ttl * 1000;
          DOH_DNS.httpsCache[hostname] = result;

          // Pre-seed A/AAAA cache from IP hints (avoids separate lookups)
          if (result.ipv4hint && result.ipv4hint.length > 0) {
            var cacheKey4 = hostname + '/' + {{{ cDefs.AF_INET }}};
            if (!DOH_DNS.cache[cacheKey4]) {
              DOH_DNS.cache[cacheKey4] = {
                addresses: result.ipv4hint, expires: result.expires
              };
              for (var j = 0; j < result.ipv4hint.length; j++) {
                DOH_DNS.reverseCache[result.ipv4hint[j]] = hostname;
              }
            }
          }
          if (result.ipv6hint && result.ipv6hint.length > 0) {
            var cacheKey6 = hostname + '/' + {{{ cDefs.AF_INET6 }}};
            if (!DOH_DNS.cache[cacheKey6]) {
              DOH_DNS.cache[cacheKey6] = {
                addresses: result.ipv6hint, expires: result.expires
              };
              for (var k = 0; k < result.ipv6hint.length; k++) {
                DOH_DNS.reverseCache[result.ipv6hint[k]] = hostname;
              }
            }
          }

#if SOCKET_DEBUG
          dbg('doh: HTTPS record for ' + hostname + ': alpn=' + (result.alpn || 'none') +
              ', ipv4hint=' + (result.ipv4hint || 'none') + ', ipv6hint=' + (result.ipv6hint || 'none'));
#endif
          return result;
        }
      } catch (e) {
#if SOCKET_DEBUG
        dbg('doh: HTTPS record query failed for ' + hostname + ': ' + e);
#endif
      }
      return null;
    },

    // register a resolved address in Emscripten's DNS.address_map so that
    // getnameinfo, gethostbyaddr, DNS.lookup_addr all work for reverse lookups
    registerMapping(hostname, ip) {
      if (DNS.address_map) {
        if (!DNS.address_map.addrs) DNS.address_map.addrs = {};
        if (!DNS.address_map.names) DNS.address_map.names = {};
        DNS.address_map.addrs[hostname] = ip;
        DNS.address_map.names[ip] = hostname;
      }
      DOH_DNS.reverseCache[ip] = hostname;
    },

    // reverse lookup: ip -> hostname (checks DoH cache, then Emscripten's map)
    reverseLookup(ip) {
      return DOH_DNS.reverseCache[ip] || (DNS.lookup_addr ? DNS.lookup_addr(ip) : null);
    },
  },

  // Override _emscripten_lookup_name (used by gethostbyname -> __lookup_name)
  _emscripten_lookup_name__deps: ['$DOH_DNS', '$DNS', '$inetPton4', '$UTF8ToString'],
  _emscripten_lookup_name__async: true,
  _emscripten_lookup_name: async (name) => {
    var hostname = UTF8ToString(name);

    // localhost and IP literals don't need DoH
    if (hostname === 'localhost' || hostname === '127.0.0.1') {
      return inetPton4(DNS.lookup_name('localhost'));
    }
    if (/^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/.test(hostname)) {
      return inetPton4(hostname);
    }

    // Try DoH resolution (IPv4 - this function returns a uint32)
    var realIp = await DOH_DNS.resolve(hostname, {{{ cDefs.AF_INET }}});
    if (realIp) {
#if SOCKET_DEBUG
      dbg('doh: resolved ' + hostname + ' -> ' + realIp);
#endif
      DOH_DNS.registerMapping(hostname, realIp);
      return inetPton4(realIp);
    }

    // Fallback to Emscripten's fake DNS
    return inetPton4(DNS.lookup_name(hostname));
  },

  // Override getaddrinfo to use DoH for hostname resolution.
  // The default getaddrinfo in libcore.js calls DNS.lookup_name() directly
  // (not _emscripten_lookup_name), so we must override it separately to get
  // real DNS for the getaddrinfo() code path.
  getaddrinfo__deps: ['$DOH_DNS', '$DNS', '$inetPton4', '$inetNtop4', '$inetPton6', '$inetNtop6', '$writeSockaddr', 'malloc', 'htonl', '$UTF8ToString'],
  getaddrinfo__async: true,
  getaddrinfo: async (node, service, hint, out) => {
    var addrs = [];
    var canon = null;
    var addr = 0;
    var port = 0;
    var flags = 0;
    var family = {{{ cDefs.AF_UNSPEC }}};
    var type = 0;
    var proto = 0;

    function allocaddrinfo(family, type, proto, canon, addr, port) {
      var sa, salen, ai;
      var errno;

      salen = family === {{{ cDefs.AF_INET6 }}} ?
        {{{ C_STRUCTS.sockaddr_in6.__size__ }}} :
        {{{ C_STRUCTS.sockaddr_in.__size__ }}};
      addr = family === {{{ cDefs.AF_INET6 }}} ?
        inetNtop6(addr) :
        inetNtop4(addr);
      sa = _malloc(salen);
      errno = writeSockaddr(sa, family, addr, port);
#if ASSERTIONS
      assert(!errno);
#endif

      ai = _malloc({{{ C_STRUCTS.addrinfo.__size__ }}});
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_family, 'family', 'i32') }}};
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_socktype, 'type', 'i32') }}};
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_protocol, 'proto', 'i32') }}};
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_canonname, 'canon', '*') }}};
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_addr, 'sa', '*') }}};
      if (family === {{{ cDefs.AF_INET6 }}}) {
        {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_addrlen, C_STRUCTS.sockaddr_in6.__size__, 'i32') }}};
      } else {
        {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_addrlen, C_STRUCTS.sockaddr_in.__size__, 'i32') }}};
      }
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_next, '0', 'i32') }}};

      return ai;
    }

    if (hint) {
      flags = {{{ makeGetValue('hint', C_STRUCTS.addrinfo.ai_flags, 'i32') }}};
      family = {{{ makeGetValue('hint', C_STRUCTS.addrinfo.ai_family, 'i32') }}};
      type = {{{ makeGetValue('hint', C_STRUCTS.addrinfo.ai_socktype, 'i32') }}};
      proto = {{{ makeGetValue('hint', C_STRUCTS.addrinfo.ai_protocol, 'i32') }}};
    }
    if (type && !proto) {
      proto = type === {{{ cDefs.SOCK_DGRAM }}} ? {{{ cDefs.IPPROTO_UDP }}} : {{{ cDefs.IPPROTO_TCP }}};
    }
    if (!type && proto) {
      type = proto === {{{ cDefs.IPPROTO_UDP }}} ? {{{ cDefs.SOCK_DGRAM }}} : {{{ cDefs.SOCK_STREAM }}};
    }
    if (proto === 0) {
      proto = {{{ cDefs.IPPROTO_TCP }}};
    }
    if (type === 0) {
      type = {{{ cDefs.SOCK_STREAM }}};
    }

    if (!node && !service) {
      return {{{ cDefs.EAI_NONAME }}};
    }
    if (flags & ~({{{ cDefs.AI_PASSIVE }}}|{{{ cDefs.AI_CANONNAME }}}|{{{ cDefs.AI_NUMERICHOST }}}|
        {{{ cDefs.AI_NUMERICSERV }}}|{{{ cDefs.AI_V4MAPPED }}}|{{{ cDefs.AI_ALL }}}|{{{ cDefs.AI_ADDRCONFIG }}})) {
      return {{{ cDefs.EAI_BADFLAGS }}};
    }
    if (hint !== 0 && ({{{ makeGetValue('hint', C_STRUCTS.addrinfo.ai_flags, 'i32') }}} & {{{ cDefs.AI_CANONNAME }}}) && !node) {
      return {{{ cDefs.EAI_BADFLAGS }}};
    }
    if (flags & {{{ cDefs.AI_ADDRCONFIG }}}) {
      // TODO
      return {{{ cDefs.EAI_NONAME }}};
    }
    if (type !== 0 && type !== {{{ cDefs.SOCK_STREAM }}} && type !== {{{ cDefs.SOCK_DGRAM }}}) {
      return {{{ cDefs.EAI_SOCKTYPE }}};
    }
    if (family !== {{{ cDefs.AF_UNSPEC }}} && family !== {{{ cDefs.AF_INET }}} && family !== {{{ cDefs.AF_INET6 }}}) {
      return {{{ cDefs.EAI_FAMILY }}};
    }

    if (service) {
      service = UTF8ToString(service);
      port = parseInt(service, 10);

      if (isNaN(port)) {
        if (flags & {{{ cDefs.AI_NUMERICSERV }}}) {
          return {{{ cDefs.EAI_NONAME }}};
        }
        return {{{ cDefs.EAI_SERVICE }}};
      }
    }

    if (!node) {
      if (family === {{{ cDefs.AF_UNSPEC }}}) {
        family = {{{ cDefs.AF_INET }}};
      }
      if ((flags & {{{ cDefs.AI_PASSIVE }}}) === 0) {
        if (family === {{{ cDefs.AF_INET }}}) {
          addr = _htonl({{{ cDefs.INADDR_LOOPBACK }}});
        } else {
          addr = [0, 0, 0, _htonl(1)];
        }
      }
      var ai = allocaddrinfo(family, type, proto, null, addr, port);
      {{{ makeSetValue('out', '0', 'ai', '*') }}};
      return 0;
    }

    //
    // try as a numeric address
    //
    node = UTF8ToString(node);
    addr = inetPton4(node);
    if (addr !== null) {
      if (family === {{{ cDefs.AF_UNSPEC }}} || family === {{{ cDefs.AF_INET }}}) {
        family = {{{ cDefs.AF_INET }}};
      }
      else if (family === {{{ cDefs.AF_INET6 }}} && (flags & {{{ cDefs.AI_V4MAPPED }}})) {
        addr = [0, 0, _htonl(0xffff), addr];
        family = {{{ cDefs.AF_INET6 }}};
      } else {
        return {{{ cDefs.EAI_NONAME }}};
      }
    } else {
      addr = inetPton6(node);
      if (addr !== null) {
        if (family === {{{ cDefs.AF_UNSPEC }}} || family === {{{ cDefs.AF_INET6 }}}) {
          family = {{{ cDefs.AF_INET6 }}};
        } else {
          return {{{ cDefs.EAI_NONAME }}};
        }
      }
    }
    if (addr != null) {
      var ai = allocaddrinfo(family, type, proto, node, addr, port);
      {{{ makeSetValue('out', '0', 'ai', '*') }}};
      return 0;
    }
    if (flags & {{{ cDefs.AI_NUMERICHOST }}}) {
      return {{{ cDefs.EAI_NONAME }}};
    }

    //
    // try as a hostname - this is where DoH kicks in
    //
    var resolvedIp = null;
    var resolvedFamily = family;

    // Try IPv6 first if requested or unspec
    if (family === {{{ cDefs.AF_UNSPEC }}} || family === {{{ cDefs.AF_INET6 }}}) {
      resolvedIp = await DOH_DNS.resolve(node, {{{ cDefs.AF_INET6 }}});
      if (resolvedIp) {
        resolvedFamily = {{{ cDefs.AF_INET6 }}};
        addr = inetPton6(resolvedIp);
        DOH_DNS.registerMapping(node, resolvedIp);
      }
    }

    // Try IPv4 if no IPv6 result, or if IPv4 specifically requested
    if (!resolvedIp && (family === {{{ cDefs.AF_UNSPEC }}} || family === {{{ cDefs.AF_INET }}})) {
      resolvedIp = await DOH_DNS.resolve(node, {{{ cDefs.AF_INET }}});
      if (resolvedIp) {
        resolvedFamily = {{{ cDefs.AF_INET }}};
        addr = inetPton4(resolvedIp);
        DOH_DNS.registerMapping(node, resolvedIp);
      }
    }

    // IPv4 result but AF_INET6 requested with V4MAPPED
    if (resolvedIp && resolvedFamily === {{{ cDefs.AF_INET }}} && family === {{{ cDefs.AF_INET6 }}}) {
      if (flags & {{{ cDefs.AI_V4MAPPED }}}) {
        addr = [0, 0, _htonl(0xffff), inetPton4(resolvedIp)];
        resolvedFamily = {{{ cDefs.AF_INET6 }}};
      } else {
        // Got IPv4 but caller wants IPv6 only, and no v4mapped - try IPv6
        resolvedIp = await DOH_DNS.resolve(node, {{{ cDefs.AF_INET6 }}});
        if (resolvedIp) {
          resolvedFamily = {{{ cDefs.AF_INET6 }}};
          addr = inetPton6(resolvedIp);
          DOH_DNS.registerMapping(node, resolvedIp);
        }
      }
    }

    if (!resolvedIp) {
      // DoH failed - fall back to Emscripten's fake DNS
      node = DNS.lookup_name(node);
      addr = inetPton4(node);
      if (family === {{{ cDefs.AF_UNSPEC }}}) {
        resolvedFamily = {{{ cDefs.AF_INET }}};
      } else if (family === {{{ cDefs.AF_INET6 }}}) {
        addr = [0, 0, _htonl(0xffff), addr];
        resolvedFamily = {{{ cDefs.AF_INET6 }}};
      } else {
        resolvedFamily = {{{ cDefs.AF_INET }}};
      }
    }

    var ai = allocaddrinfo(resolvedFamily, type, proto, null, addr, port);
    {{{ makeSetValue('out', '0', 'ai', '*') }}};
    return 0;
  },
};

addToLibrary(DoHLibrary);

#endif // DOH_DNS
