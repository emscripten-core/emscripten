/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <process.h> // For _spawnvp
#include <stdio.h>
#include <stdlib.h> // For _splitpath_s, _makepath_s, GetEnvironmentVariable
#include <string.h>
#include <windows.h>

#if 1
#define dbg printf
#else
#define dbg(...)
#endif

void findPythonExePath(char* pythonExePath, size_t bufferSize) {
  char envPythonPath[MAX_PATH]; // Temporary buffer for environment variable

  // Attempt to retrieve the value of the "PYTHON" environment variable.
  DWORD result = GetEnvironmentVariable("PYTHON", envPythonPath, MAX_PATH);

  if (result > 0 && result < MAX_PATH) {
    // If the "PYTHON" environment variable is set and its value fits within the
    // buffer, use this explicit path. strncpy_s is used for safe string
    // copying.
    strncpy_s(pythonExePath, bufferSize, envPythonPath, _TRUNCATE);
    dbg("Launcher: Using PYTHON environment variable: %s\n", pythonExePath);
    return;
  }

  // If "PYTHON" environment variable is not set or its value is too long,
  // default to "python.exe". The _spawnvp function in main() will then
  // search for "python.exe" in the system's PATH.
  strcpy(pythonExePath, "python.exe");
  dbg("Launcher: PYTHON environment variable not found or invalid, defaulting "
      "to 'python.exe' (will use PATH search).\n");
}

int main() {
  char exePath[MAX_PATH];
  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];
  char fname[_MAX_FNAME];
  char ext[_MAX_EXT];
  char pythonScriptPath[MAX_PATH];
  char pythonExeToLaunch[MAX_PATH]; // Buffer to store the determined Python
                                    // executable path

  // Call the subroutine to determine the Python executable path and its
  // discovery method. The return value (discoveryMethod) is no longer used for
  // adding -Xutf8.
  findPythonExePath(pythonExeToLaunch, MAX_PATH);

  // 1. Get the full path of the current executable.
  if (GetModuleFileName(NULL, exePath, MAX_PATH) == 0) {
    fprintf(stderr, "Error: Could not get executable path (Error code: %lu)\n", GetLastError());
    return 1;
  }

  // 2. Split the executable path into its components (drive, directory, file
  // name, extension).
  _splitpath_s(exePath, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);

  // 3. Construct the Python script path. It uses the same base name as the
  // launcher but with a ".py" extension, residing in the same directory.
  _makepath_s(pythonScriptPath, MAX_PATH, drive, dir, fname, ".py");

  dbg("Launcher: Executable path: %s\n", exePath);
  dbg("Launcher: Python script to run: %s\n", pythonScriptPath);

  // 4. Prepare arguments for the Python executable.
  // The arguments are now always 3: python_exe, script_path, and NULL
  // terminator.
  char* argv[] = {
    pythonExeToLaunch, // Use the path determined by findPythonExePath
    pythonScriptPath,
    NULL};

  // Check for EM_WORKAROUND_PYTHON_BUG_34780 environment variable
  if (GetEnvironmentVariable("EM_WORKAROUND_PYTHON_BUG_34780", NULL, 0) > 0) {
    dbg("Launcher: EM_WORKAROUND_PYTHON_BUG_34780 is set so closing stdin\n");
    fclose(stdin);
  }

  // 5. Launch the Python executable and wait for it to finish.
  // _spawnvp searches for the program specified in argv[0] in the system's PATH
  // (if argv[0] is not a full path) and executes it. _P_WAIT ensures the
  // launcher waits for the Python process to complete before continuing.
  intptr_t result = _spawnvp(_P_WAIT, argv[0], argv);

  if (result == -1) {
    // If _spawnvp returns -1, an error occurred during the launch of the child
    // process.
    perror("Launcher Error: Failed to launch python.exe");
    return 1; // Set a specific exit code to indicate launch failure.
  }

  // If successful, the return value of _spawnvp is the exit code of the spawned
  // process.
  int exitCode = (int)result;
  dbg("Launcher: python.exe exited with code: %d\n", exitCode);
  return exitCode;
}
