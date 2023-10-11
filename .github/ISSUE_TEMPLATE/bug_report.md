---
name: Bug report
about: Create a report to help us improve
title: ''
labels: ''
assignees: ''

---
Please include the following in your bug report:

**Version of emscripten/emsdk:**
Please include the output `emcc -v` here

**Failing command line in full:**
If this is compile or link-time failure please include the full failing command
along with its entire output.

**Full link command and output with `-v` appended:**
Even for runtime issues it helps a lot if you can include the full link command.
Adding `-v` to the link command will show all of the sub-commands run which
can help us diagnose your issue.

Note:  Where possible, please avoid attaching screen shots of code or console
logs.  Instead, please include them as text so that they may be copied /
searched.  To make code blocks more readable and syntax highlighted, please
escape them with three backticks before and after, like this:

```cpp
int x = 20;
```
