'''
Ar archivers (ar, llvm-ar) include only basename of their members whithout a full path,
which leads to duplicate entries in an archive. When linking with *.a file containing duplicate entries,
only the last one is taken into account, leading to unresolved external symbols.
Appending full path hash to file name helps to avoid duplicate entries in archives, which takes place
when we archive files with same basenames from different directories.
'''


import hashlib, os, shutil
from response_file import create_response_file, expand_response_files

# appends absolute path hash to file_name
def append_hash(file_name):
  full_name = os.path.abspath(file_name)
  full_name_hash = hashlib.md5(full_name.encode('utf-8')).hexdigest()[:8]
  base_name = os.path.basename(file_name)
  dir_name = os.path.dirname(file_name)
  parts = base_name.split('.')
  parts[0] = '%s_%s' % (parts[0], full_name_hash)
  hashed_base_name = '.'.join(parts)
  return os.path.join(dir_name, hashed_base_name)

def find_unused_filename(file_name):
  nonce = 0
  while os.path.exists(file_name):
    base_name = os.path.basename(file_name)
    dir_name = os.path.dirname(file_name)
    parts = base_name.split('.')
    parts[0] = '%s_%d' % (parts[0], nonce)
    nonced_base_name = '.'.join(parts)
    file_name = os.path.join(dir_name, nonced_base_name)
    nonce += 1
  return file_name

# Recursively expands response files from argument list into a flattened list of files.
# Appends full path hashes to file names from flattened list and copies original files to hashed ones.
# All hashed files are added to temp file list and will be automatically deleted after process finishes.
# Writes a temporary response file with hashed file names. Returns temporary response file name.
def make_hashed_response_file(args, directory):
  from . import shared
  rsp_content = expand_response_files(args)
  hashed_rsp_content = map(lambda fn:  append_hash(fn), rsp_content)
  dst_filenames = []
  for p in zip(rsp_content, hashed_rsp_content):
    dst_name = find_unused_filename(p[1])
    shutil.copyfile(p[0], dst_name)
    dst_filenames.append(dst_name)
    shared.configuration.get_temp_files().note(dst_name)
  return create_response_file(dst_filenames, directory)
