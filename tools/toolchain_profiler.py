# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import atexit
import logging
import os
import sys
import subprocess
import tempfile
import time
from contextlib import ContextDecorator

logger = logging.getLogger('profiler')

from . import response_file

EMPROFILE = int(os.getenv('EMPROFILE', '0'))


class Logger(ContextDecorator):
  depth = 0

  def __init__(self, name):
    self.name = name

  def __call__(self, func):
    if self.name is None:
      self.name = func.__name__
    return super().__call__(func)

  def __enter__(self):
    if EMPROFILE == 2:
      indentation = '  ' * Logger.depth
      logger.info('%sstart block "%s"', indentation, self.name)
      Logger.depth += 1
    self.start = time.time()

  def __exit__(self, exc_type, value, traceback):
    # When a block ends debug log the total duration.
    now = time.time()
    duration = now - self.start
    if exc_type:
      msg = 'block "%s" raised an exception after %.3f seconds'
    else:
      msg = 'block "%s" took %.3f seconds'
    if EMPROFILE == 2:
      Logger.depth -= 1
      indentation = '  ' * Logger.depth
      logger.info(indentation + msg, self.name, duration)
    else:
      logger.debug(msg, self.name, duration)


if EMPROFILE == 1:
  original_sys_exit = sys.exit
  original_subprocess_call = subprocess.call
  original_subprocess_check_call = subprocess.check_call
  original_subprocess_check_output = subprocess.check_output
  original_Popen = subprocess.Popen
  process_returncode = None

  def profiled_sys_exit(returncode):
    # Stack the returncode which then gets used in the atexit handler
    global process_returncode
    process_returncode = returncode
    original_sys_exit(returncode)

  def profiled_call(cmd, *args, **kw):
    pid = ToolchainProfiler.imaginary_pid()
    ToolchainProfiler.record_subprocess_spawn(pid, cmd)
    try:
      returncode = original_subprocess_call(cmd, *args, **kw)
    except Exception:
      ToolchainProfiler.record_subprocess_finish(pid, 1)
      raise
    ToolchainProfiler.record_subprocess_finish(pid, returncode)
    return returncode

  def profiled_check_call(cmd, *args, **kw):
    pid = ToolchainProfiler.imaginary_pid()
    ToolchainProfiler.record_subprocess_spawn(pid, cmd)
    try:
      ret = original_subprocess_check_call(cmd, *args, **kw)
    except Exception as e:
      ToolchainProfiler.record_subprocess_finish(pid, e.returncode)
      raise
    ToolchainProfiler.record_subprocess_finish(pid, 0)
    return ret

  def profiled_check_output(cmd, *args, **kw):
    pid = ToolchainProfiler.imaginary_pid()
    ToolchainProfiler.record_subprocess_spawn(pid, cmd)
    try:
      ret = original_subprocess_check_output(cmd, *args, **kw)
    except Exception as e:
      ToolchainProfiler.record_subprocess_finish(pid, e.returncode)
      raise
    ToolchainProfiler.record_subprocess_finish(pid, 0)
    return ret

  class ProfiledPopen(original_Popen):
    def __init__(self, args, *otherargs, **kwargs):
      super().__init__(args, *otherargs, **kwargs)
      ToolchainProfiler.record_subprocess_spawn(self.pid, args)

    def communicate(self, *args, **kwargs):
      ToolchainProfiler.record_subprocess_wait(self.pid)
      output = super().communicate(*args, **kwargs)
      ToolchainProfiler.record_subprocess_finish(self.pid, self.returncode)
      return output

  sys.exit = profiled_sys_exit
  subprocess.call = profiled_call
  subprocess.check_call = profiled_check_call
  subprocess.check_output = profiled_check_output
  subprocess.Popen = ProfiledPopen

  class ToolchainProfiler:
    # Provide a running counter towards negative numbers for PIDs for which we
    # don't know what the actual process ID is
    imaginary_pid_ = 0
    profiler_logs_path = None # Log file not opened yet

    block_stack = []

    # Track if record_process_exit and record_process_start have been called
    # so we can assert they are only ever called once.
    process_start_recorded = False
    process_exit_recorded = False

    @staticmethod
    def timestamp():
      return '{0:.3f}'.format(time.time())

    @staticmethod
    def log_access():
      # Note: This function is called in two importantly different contexts: in
      # "main" process and in python subprocesses invoked via
      # subprocessing.Pool.map(). The subprocesses have their own PIDs, and
      # hence record their own data JSON files, but since the process pool is
      # maintained internally by python, the toolchain profiler does not track
      # the parent->child process spawns for the subprocessing pools. Therefore
      # any profiling events that the subprocess children generate are virtually
      # treated as if they were performed by the parent PID.
      return open(os.path.join(ToolchainProfiler.profiler_logs_path, 'toolchain_profiler.pid_' + str(os.getpid()) + '.json'), 'a')

    @staticmethod
    def escape_string(arg):
      return arg.replace('\\', '\\\\').replace('"', '\\"')

    @staticmethod
    def escape_args(args):
      return map(lambda arg: ToolchainProfiler.escape_string(arg), args)

    @staticmethod
    def record_process_start(write_log_entry=True):
      assert not ToolchainProfiler.process_start_recorded
      ToolchainProfiler.process_start_recorded = True
      atexit.register(ToolchainProfiler.record_process_exit)

      # For subprocessing.Pool.map() child processes, this points to the PID of
      # the parent process that spawned the subprocesses. This makes the
      # subprocesses look as if the parent had called the functions.
      ToolchainProfiler.mypid_str = str(os.getpid())
      ToolchainProfiler.profiler_logs_path = os.path.join(tempfile.gettempdir(), 'emscripten_toolchain_profiler_logs')
      os.makedirs(ToolchainProfiler.profiler_logs_path, exist_ok=True)

      ToolchainProfiler.block_stack = []

      if write_log_entry:
        with ToolchainProfiler.log_access() as f:
          f.write('[\n{"pid":' + ToolchainProfiler.mypid_str + ',"subprocessPid":' + str(os.getpid()) + ',"op":"start","time":' + ToolchainProfiler.timestamp() + ',"cmdLine":["' + '","'.join(ToolchainProfiler.escape_args(sys.argv)) + '"]}')

    @staticmethod
    def record_process_exit():
      assert not ToolchainProfiler.process_exit_recorded
      assert ToolchainProfiler.process_start_recorded
      ToolchainProfiler.process_exit_recorded = True

      ToolchainProfiler.exit_all_blocks()
      with ToolchainProfiler.log_access() as f:
        returncode = process_returncode
        if returncode is None:
          returncode = '"MISSING EXIT CODE"'
        f.write(',\n{"pid":' + ToolchainProfiler.mypid_str + ',"subprocessPid":' + str(os.getpid()) + ',"op":"exit","time":' + ToolchainProfiler.timestamp() + ',"returncode":' + str(returncode) + '}\n]\n')

    @staticmethod
    def record_subprocess_spawn(process_pid, process_cmdline):
      expanded_cmdline = response_file.substitute_response_files(process_cmdline)

      with ToolchainProfiler.log_access() as f:
        f.write(',\n{"pid":' + ToolchainProfiler.mypid_str + ',"subprocessPid":' + str(os.getpid()) + ',"op":"spawn","targetPid":' + str(process_pid) + ',"time":' + ToolchainProfiler.timestamp() + ',"cmdLine":["' + '","'.join(ToolchainProfiler.escape_args(expanded_cmdline)) + '"]}')

    @staticmethod
    def record_subprocess_wait(process_pid):
      with ToolchainProfiler.log_access() as f:
        f.write(',\n{"pid":' + ToolchainProfiler.mypid_str + ',"subprocessPid":' + str(os.getpid()) + ',"op":"wait","targetPid":' + str(process_pid) + ',"time":' + ToolchainProfiler.timestamp() + '}')

    @staticmethod
    def record_subprocess_finish(process_pid, returncode):
      with ToolchainProfiler.log_access() as f:
        f.write(',\n{"pid":' + ToolchainProfiler.mypid_str + ',"subprocessPid":' + str(os.getpid()) + ',"op":"finish","targetPid":' + str(process_pid) + ',"time":' + ToolchainProfiler.timestamp() + ',"returncode":' + str(returncode) + '}')

    @staticmethod
    def enter_block(block_name):
      with ToolchainProfiler.log_access() as f:
        f.write(',\n{"pid":' + ToolchainProfiler.mypid_str + ',"subprocessPid":' + str(os.getpid()) + ',"op":"enterBlock","name":"' + block_name + '","time":' + ToolchainProfiler.timestamp() + '}')

      ToolchainProfiler.block_stack.append(block_name)

    @staticmethod
    def remove_last_occurrence_if_exists(lst, item):
      for i in range(len(lst)):
        if lst[i] == item:
          lst.pop(i)
          return True
      return False

    @staticmethod
    def exit_block(block_name):
      if ToolchainProfiler.remove_last_occurrence_if_exists(ToolchainProfiler.block_stack, block_name):
        with ToolchainProfiler.log_access() as f:
          f.write(',\n{"pid":' + ToolchainProfiler.mypid_str + ',"subprocessPid":' + str(os.getpid()) + ',"op":"exitBlock","name":"' + block_name + '","time":' + ToolchainProfiler.timestamp() + '}')

    @staticmethod
    def exit_all_blocks():
      for b in ToolchainProfiler.block_stack[::-1]:
        ToolchainProfiler.exit_block(b)

    class ProfileBlock(Logger):
      def __init__(self, block_name):
        super().__init__(block_name)
        self.name = block_name

      def __enter__(self):
        ToolchainProfiler.enter_block(self.name)

      def __exit__(self, type, value, traceback):
        ToolchainProfiler.exit_block(self.name)

    @staticmethod
    def profile_block(block_name):
      return ToolchainProfiler.ProfileBlock(ToolchainProfiler.escape_string(block_name))

    @staticmethod
    def profile():
      return ToolchainProfiler.ProfileBlock(None)

    @staticmethod
    def imaginary_pid():
      ToolchainProfiler.imaginary_pid_ -= 1
      return ToolchainProfiler.imaginary_pid_

  ToolchainProfiler.record_process_start()
else:
  class ToolchainProfiler:
    @staticmethod
    def enter_block(block_name):
      pass

    @staticmethod
    def exit_block(block_name):
      pass

    @staticmethod
    def profile_block(block_name):
      return Logger(block_name)

    @staticmethod
    def profile():
      return Logger(None)
