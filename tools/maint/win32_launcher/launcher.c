/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
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
#include <stdlib.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

wchar_t* get_python_executable() {
  wchar_t* python_exe_w = _wgetenv(L"EMSDK_PYTHON");
  if (!python_exe_w) {
    return L"python";
  }
  return python_exe_w;
}

wchar_t* get_long_path(const wchar_t* path) {
  DWORD path_len = GetFullPathNameW(path, 0, NULL, NULL);
  if (path_len == 0)
    abort();

  wchar_t* full_path = malloc(sizeof(wchar_t) * (path_len + 4));
  if (!full_path)
    abort();

  wcscpy_s(full_path, path_len + 4, L"\\\\?\\");
  if (GetFullPathNameW(path, path_len, full_path + 4, NULL) == 0)
    abort();
  return full_path;
}

// Get the name of the currently running executable (module)
wchar_t* get_module_path() {
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

// Handle the _EMCC_CCACHE environment variable which essentially re-executes
// the lauccher command with `cacche.exe` prefixed.
BOOL handle_ccache(wchar_t** application_name, wchar_t** command_line, const wchar_t* launcher_path_w) {
  const wchar_t* ccache_env = _wgetenv(L"_EMCC_CCACHE");
  if (!ccache_env || !*ccache_env) {
    return FALSE;
  }

  _wputenv_s(L"_EMCC_CCACHE", L"");
  *application_name = L"ccache.exe";

  size_t command_line_len = wcslen(*application_name) + wcslen(launcher_path_w) + 5;
  *command_line = malloc(sizeof(wchar_t) * command_line_len);
  if (!*command_line)
    abort();

  swprintf(*command_line, command_line_len, L"\"%s\" \"%s\"", *application_name, launcher_path_w);
  return TRUE;
}

// A custom replacement for PathGetArgsW that is safe for command lines
// longer than MAX_PATH.
const wchar_t* find_args(const wchar_t* command_line) {
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

int wmain(int argc, wchar_t** argv) {
  wchar_t* launcher_path_w = get_module_path();

  wchar_t* launcher_dir = wcsdup(launcher_path_w);
  PathRemoveFileSpecW(launcher_dir);

  size_t path_len = wcslen(launcher_path_w);
  wchar_t* launcher_name_w = malloc((path_len + 1) * sizeof(wchar_t));
  if (!launcher_name_w)
    abort();
  wcscpy_s(launcher_name_w, path_len + 1, launcher_path_w);
  PathStripPathW(launcher_name_w);
  PathRemoveExtensionW(launcher_name_w);

  size_t script_path_len = wcslen(launcher_dir) + wcslen(launcher_name_w) + 6;
  wchar_t* script_path_w = malloc(sizeof(wchar_t) * script_path_len);
  swprintf(script_path_w, script_path_len, L"%s\%s.py", launcher_dir, launcher_name_w);

  wchar_t* application_name = NULL;
  wchar_t* command_line = NULL;

  if (!handle_ccache(&application_name, &command_line, launcher_path_w)) {
    application_name = get_python_executable();
    wchar_t* long_script_path = get_long_path(script_path_w);
    size_t command_line_len = wcslen(application_name) + wcslen(long_script_path) + 10;
    command_line = malloc(sizeof(wchar_t) * command_line_len);
    swprintf(command_line, command_line_len, L"\"%s\" -E \"%s\"", application_name, long_script_path);
    free(long_script_path);
    // -E will not ignore _PYTHON_SYSCONFIGDATA_NAME an internal
    // of cpython used in cross compilation via setup.py.
    _wputenv_s(L"_PYTHON_SYSCONFIGDATA_NAME", L"");
  }

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
  wchar_t* workaround_env = _wgetenv(L"EM_WORKAROUND_PYTHON_BUG_34780");
  if (workaround_env) {
    CloseHandle(GetStdHandle(STD_INPUT_HANDLE));
  }

  STARTUPINFOW si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  if (!CreateProcessW(application_name, command_line, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
    wprintf(L"CreateProcess failed (%d).\n", GetLastError());
    abort();
  }

  WaitForSingleObject(pi.hProcess, INFINITE);

  DWORD exit_code;
  GetExitCodeProcess(pi.hProcess, &exit_code);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return exit_code;
}
