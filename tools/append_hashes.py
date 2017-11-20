'''
Recurrent response file traverser.

Extends each file name from a file list with it's full path hash and copies original file to hashed file.
Response files (i.e. whose name start with @ and who contain a list of other files) are processed recursively.

Modifies response files content with hashed file names.
Also returns list of copied files.

Ar archivers (ar, llvm-ar) include only basename of their members whithout a full path,
which leads to duplicate entries in an archive. When linking with *.a file containing duplicate entries,
only the last one is taken into account, leading to unresolved external symbols.
Appending full path hash to file name helps to avoid duplicate entries in archives, which takes place
when we archive files with same basenames from different directories.
'''


import hashlib, os, shutil

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

# Append hashes to all files referenced in file rsp_filename.
# Original files are always copied to hashed files, as they could be altered since last copying.
# Recursively processes other rsp files referenced in rsp_filename.
def append_hash_to_rsp_file(rsp_filename):
  with open(rsp_filename, 'r') as f:
    rsp_content = f.read().replace('\n', '')
  (hashed_names, copied_names) = append_hash_to_file_list(rsp_content_as_list(rsp_content))
  with open(rsp_filename, 'w') as f:
    f.write(' '.join(hashed_names))
  return copied_names

# parse response file content according to @file section from man ar(1)
def rsp_content_as_list(rsp_content):
  result = []
  current = ''
  verbatim = False
  skip = False
  for i in range(len(rsp_content)):
    if skip:
      skip = False
      continue
    if rsp_content[i] == '\\':
      current += rsp_content[i + 1]
      skip = True
      continue
    if rsp_content[i] == '"':
      if verbatim:
        result.append(current)
        current = ''
      verbatim = not verbatim
      continue
    if (rsp_content[i] == ' ') and (not verbatim):
      if len(current) > 0:
        result.append(current)
        current = ''
      continue
    current += rsp_content[i]
  if len(current) > 0:
    result.append(current)
  return result

# Append hash to each file name from file_list.
# Recursively call append_hash_to_rsp_file on each rsp file from file_list.
def append_hash_to_file_list(file_list):
  hashed_names = []
  copied_names = []
  for file_name in file_list:
    if file_name.startswith('@'):
      tobehashed_file_name = file_name[1:]
    else:
      tobehashed_file_name = file_name
    # prevent any existing files from overwriting
    hashed_name = find_unused_filename(append_hash(tobehashed_file_name))
    full_hashed_name = os.path.abspath(hashed_name)
    if file_name.startswith('@'): # prepend @ back to hashed file name
      hashed_name = '@%s' % (hashed_name)
    try: # it is ok to fail here, we just don't get hashing
      shutil.copyfile(tobehashed_file_name, full_hashed_name)
      hashed_names.append(hashed_name)
      copied_names.append(full_hashed_name) # delete copied files only
    except:
      hashed_names.append(file_name)
    if file_name.startswith('@'):
      copied_names.extend(append_hash_to_rsp_file(hashed_names[-1][1:]))
  hashed_names = map(lambda fname: '"%s"' % (fname) if ' ' in fname else fname, hashed_names)
  return (hashed_names, copied_names)
