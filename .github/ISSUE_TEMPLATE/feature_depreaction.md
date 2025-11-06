---
name: Feature deprecation
about: Use this template for removal of features from emscripten.
title: 'Deprecation proposal for <FEATURE>'
labels: 'deprecation'
assignees: ''

---

Tracking issue for the deprecation and removal of <FEATURE>.

The process for deprecating and removing features from emscripten is laid out out in https://github.com/emscripten-core/emscripten/blob/main/docs/process.md#deprecating-settings-and-features.

At any point in the process we may decide to delay or abort the deprecation and close this issue.

Deprecation checklist:

- [ ] Open an issue (this)
- [ ] Perform global search for usage in public repos
- [ ] Send message to emscripten-discuss
- [ ] Add "deprecation" warning to emcc
- [ ] Wait at least 4 releases or 2 months
- [ ] Remove feature from codebase
