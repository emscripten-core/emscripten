/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Small win32 application that is used to launcher emscripten via python.exe.
 * On non-windows platforms this is done via the run_pyton.sh shell script.
 *
 * The binary will look for a python script that matches its own name and run
 * that using python.exe.
 */

// Define _WIN32_WINNT to Windows 7 for max portability
#define _WIN32_WINNT 0x0601

#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

bool launcher_debug = false;

static int dbg(const char* format, ...) {
  if (launcher_debug) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
  }
}

static const wchar_t* get_python_executable() {
  const wchar_t* python_exe_w = _wgetenv(L"EMSDK_PYTHON");
  if (!python_exe_w) {
    return L"python.exe";
  }
  return python_exe_w;
}

// Get the name of the currently running executable (module)
static const wchar_t* get_module_path() {
  DWORD buffer_size = MAX_PATH;
  wchar_t* module_path_w = malloc(sizeof(wchar_t) * buffer_size);
  if (!module_path_w)
    abort();

  DWORD path_len = GetModuleFileNameW(NULL, module_path_w, buffer_size);
  while (path_len > 0 && path_len == buffer_size) {
    buffer_size *= 2;
    module_path_w = realloc(module_path_w, sizeof(wchar_t) * buffer_size);
    if (!module_path_w)
      abort();
    path_len = GetModuleFileNameW(NULL, module_path_w, buffer_size);
  }

  if (path_len == 0)
    abort();

  return module_path_w;
}

/**
 * A custom replacement for PathGetArgsW that is safe for command lines
 * longer than MAX_PATH.
 */
static const wchar_t* find_args(const wchar_t* command_line) {
  const wchar_t* p = command_line;

  // Skip past the executable name, which can be quoted.
  if (*p == L'"') {
    // The path is quoted, find the closing quote.
    p++;
    while (*p) {
      if (*p == L'"') {
        p++;
        break;
      }
      p++;
    }
  } else {
    // The path is not quoted, find the first space.
    while (*p && *p != L' ' && *p != L'\t') {
      p++;
    }
  }

  // Skip any whitespace between the executable and the first argument.
  while (*p && (*p == L' ' || *p == L'\t')) {
    p++;
  }

  return p;
}

/**
 * Create the script path by taking the launcher path and replacing the
 * extension with .py. For example `C:\path\to\emcc.exe` becomes
 * `C:\path\to\emcc.py`.
 *
 * If the corresponging .py file does not exist then also look it in the tools
 * subdirectory.  e.g. `C:\path\to\tools\emcc.py`
 */
static const wchar_t* get_script_path(const wchar_t* launcher_path) {
  wchar_t* script_path = wcsdup(launcher_path);
  if (!script_path)
    abort();
  PathRemoveExtensionW(script_path);
  size_t current_len = wcslen(script_path);
  // 4 for `.py` and the null terminator
  size_t new_size_in_chars = current_len + 4;
  script_path = realloc(script_path, new_size_in_chars * sizeof(wchar_t));
  if (!script_path)
    abort();
  wcscat_s(script_path, new_size_in_chars, L".py");
  if (PathFileExistsW(script_path)) {
    return script_path;
  }

  // Python file not found alongside launcher; try under tools
  // C:\path\to\emcc.py` => C:\path\to\tools\emcc.py`
  wchar_t* script_path_copy = wcsdup(script_path);
  wchar_t* basename = PathFindFileNameW(script_path_copy);
  // We need to add 6 more chars for 'tools\'.
  new_size_in_chars += 6;
  wchar_t* script_path_tools = malloc(new_size_in_chars * sizeof(wchar_t));
  wcscpy(script_path_tools, script_path);
  PathRemoveFileSpecW(script_path_tools);
  PathCombineW(script_path_tools, script_path_tools, L"tools");
  PathCombineW(script_path_tools, script_path_tools, basename);

  if (!PathFileExistsW(script_path_tools)) {
    fprintf(stderr, "pylauncher: target python file not found: %ls / %ls\n", script_path, script_path_tools);
    abort();
  }
  free(script_path_copy);
  free(script_path);

  return script_path_tools;
}

int main(int argc, char* argv[]) {
  // Setting EMCC_LAUNCHER_DEBUG enabled debug output for the launcher itself.
  launcher_debug = GetEnvironmentVariableW(L"EMCC_LAUNCHER_DEBUG", NULL, 0);

  dbg("pylauncher: main\n");

  const wchar_t* ccache = NULL;
  DWORD env_len = GetEnvironmentVariableW(L"_EMCC_CCACHE", NULL, 0);
  if (env_len) {
    dbg("pylauncher: running via ccache.exe\n");
    ccache = L"ccache.exe";
    SetEnvironmentVariableW(L"_EMCC_CCACHE", NULL);
  }

  const wchar_t* application_name = get_python_executable();
  const wchar_t* launcher_path_w = get_module_path();
  const wchar_t* script_path_w = get_script_path(launcher_path_w);
  size_t command_line_len = wcslen(application_name) + wcslen(script_path_w) + 9;
  if (ccache) {
    command_line_len += wcslen(ccache) + 1;
  }
  wchar_t* command_line = malloc(sizeof(wchar_t) * command_line_len);
  if (ccache) {
    swprintf(command_line, command_line_len, L"%ls \"%ls\" -E \"%ls\"", ccache, application_name, script_path_w);
  } else {
    swprintf(command_line, command_line_len, L"\"%ls\" -E \"%ls\"", application_name, script_path_w);
  }

  // -E will not ignore _PYTHON_SYSCONFIGDATA_NAME an internal
  // of cpython used in cross compilation via setup.py.
  SetEnvironmentVariableW(L"_PYTHON_SYSCONFIGDATA_NAME", NULL);

  // Build the final command line by appending the original arguments
  const wchar_t* all_args = find_args(GetCommandLineW());
  if (all_args && *all_args) {
    size_t current_len = wcslen(command_line);
    size_t args_len = wcslen(all_args);
    // +2 for the space and the null terminator
    command_line = realloc(command_line, (current_len + args_len + 2) * sizeof(wchar_t));
    if (!command_line)
      abort();
    wcscat_s(command_line, current_len + args_len + 2, L" ");
    wcscat_s(command_line, current_len + args_len + 2, all_args);
  }

  // Work around python bug 34780 by closing stdin, so that it is not inherited
  // by the python subprocess.
  env_len = GetEnvironmentVariableW(L"EM_WORKAROUND_PYTHON_BUG_34780", NULL, 0);
  if (env_len) {
    dbg("pylauncher: using EM_WORKAROUND_PYTHON_BUG_34780\n");
    CloseHandle(GetStdHandle(STD_INPUT_HANDLE));
  }

  STARTUPINFOW si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  dbg("pylauncher: running: %ls\n", command_line);
  if (!CreateProcessW(NULL, command_line, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
    fprintf(stderr, "pylauncher: CreateProcess failed (%d): %ls\n", GetLastError(), command_line);
    abort();
  }
  WaitForSingleObject(pi.hProcess, INFINITE);

  DWORD exit_code;
  GetExitCodeProcess(pi.hProcess, &exit_code);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  dbg("pylauncher: done: %d\n", exit_code);
  return exit_code;
}
