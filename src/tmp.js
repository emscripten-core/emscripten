Object.assign(
  node,
  Object.fromEntries(
    Object.entries(attr).filter(([k, v]) => ['mode', 'atime', 'mtime', 'ctime'].contains(k) && v),
  ),
);
