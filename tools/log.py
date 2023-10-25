import logging
import os

default_channel = None
log_levels_by_channel = {}

def str_to_log_level(s):
  level = s.lower()
  if level == 'mute' or level == 'critical': return logging.CRITICAL
  if level == 'error': return logging.ERROR
  if level == 'warning': return logging.WARNING
  if level == 'info': return logging.INFO
  if level == 'debug': return logging.DEBUG
  raise Exception(f'Invalid logging level "{s}"')

log_level = os.environ.get('EMCC_LOG_LEVEL')
if log_level:
  if ',' in log_level: # User passes multiple log levels for different channels?
    if ':' not in log_level:
      raise Exception('When specifying log levels for multiple channels, use the syntax "channel1:level1,channel2:level2".')
    channels = log_level.split(',')
    for c in channels:
      if ':' in c:
        name, level = c.split(':')
        log_levels_by_channel[name] = str_to_log_level(level)
      else:
        default_channel = str_to_log_level(c)
  else: # User passed a single "set EMCC_LOG_LEVEL=debug" directive to control level of all channels.
    default_channel = str_to_log_level(log_level)



def getLogger(name):
  logger = logging.getLogger(name)
  if name in log_levels_by_channel:
    logger.setLevel(log_levels_by_channel[name])
  elif default_channel:
    logger.setLevel(default_channel)
  return logger
