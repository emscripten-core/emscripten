import sys
import os
from sets import Set

def split_javascript_file(input_filename, output_filename_prefix, max_part_size_in_bytes):
 
  try:
    # Javascript main file. On execution, this file needs to be loaded at last (!)
    output_main_filename = output_filename_prefix + ".js"
    output_main_file = open(output_main_filename,'w')
    # File with HTML script tags to load the Javascript files in HTML later on
    output_html_include_file = open(output_filename_prefix + ".include.html",'w')

    # Variable will contain the source of a Javascript function if we find one during parsing
    js_function = None
    
    # Dictionary with lower case source file as key and a tupel of case sensitive source file name (first encountered case wins)
    # and an array of functions associated to that source file as value
    function_buckets = {};
    
    output_part_file = None

    # Locate names of all the source files (.c/.cpp) that produced output to the .js file.
    source_files = Set()
    for line in open(input_filename,'r'):
      if line.startswith("//FUNCTION_END_MARKER_OF_SOURCE_FILE_"):
        associated_source_file_base = line[len("//FUNCTION_END_MARKER_OF_SOURCE_FILE_"):len(line)-1]
        if not associated_source_file_base == "NO_SOURCE":
          source_files.add(os.path.dirname(os.path.abspath(os.path.realpath(associated_source_file_base))))

    common_source_file_prefix = os.path.commonprefix(list(source_files))

    # Iterate over Javascript source; write main file; parse function declarations.
    input_file = open(input_filename,'r')
    for line in input_file:
      if line == "//FUNCTION_BEGIN_MARKER\n":
        js_function = "//Func\n"
      elif line.startswith("//FUNCTION_END_MARKER_OF_SOURCE_FILE_"):
        # At the end of the function marker we get the source file that is associated to that function.
        associated_source_file_base = line[len("//FUNCTION_END_MARKER_OF_SOURCE_FILE_"):len(line)-1]
        
        if associated_source_file_base == "NO_SOURCE":
          # Functions without associated source file are stored in a file in the base directory
          associated_source_file_base = output_filename_prefix + "_functions";
        else:
          # Functions with a known associated source file are stored in a file in the directory `output_filename_prefix`
          associated_source_file_base = os.path.join(output_filename_prefix, os.path.relpath(os.path.abspath(os.path.realpath(associated_source_file_base)), common_source_file_prefix))

        associated_source_file_base_lower = associated_source_file_base.lower()
        
        # Add the function to its respective file
        if associated_source_file_base_lower not in function_buckets:
          function_buckets[associated_source_file_base_lower] = [associated_source_file_base, []]
        function_buckets[associated_source_file_base_lower][1] += [js_function]
        
        # Clear the function read cache
        js_function = None
      else:
        if js_function is None:
          output_main_file.write(line)
        else:
          js_function += line
          
    # Iterate over all function buckets and write their functions to the associated files
    # An associated file is split into chunks of `max_part_size_in_bytes`
    for associated_source_file_base in function_buckets:
      # At first we try to name the Javascript source file to match the assoicated source file + `.js`
      js_source_file = function_buckets[associated_source_file_base][0] + ".js"
    
      # Check if the directory of the Javascript source file exists
      js_source_dir = os.path.dirname(js_source_file)
      if len(js_source_dir) > 0 and not os.path.exists(js_source_dir):
        os.makedirs(js_source_dir)
    
      output_part_file_counter = 0
      output_part_file = None
      for js_function in function_buckets[associated_source_file_base][1]:
        if output_part_file is None:
          output_html_include_file.write("<script type=\"text/javascript\" src=\"" + js_source_file.replace('\\', '/') + "\"></script>")
          output_part_file = open(js_source_file,'w')
        
        output_part_file.write(js_function)
        
        if output_part_file is not None and output_part_file.tell() > max_part_size_in_bytes:
          output_part_file.close()
          output_part_file = None
          output_part_file_counter += 1
          js_source_file = function_buckets[associated_source_file_base][0] + ".part" + str(output_part_file_counter) + ".js"
    
      if output_part_file is not None:
        output_part_file.close()
        output_part_file = None
      
    # Write the main Javascript file at last to the HTML includes because this file contains the code to start
    # the execution of the generated Emscripten application and requires all the extracted functions.
    output_html_include_file.write("<script type=\"text/javascript\" src=\"" + output_main_filename.replace('\\', '/') + "\"></script>")

  except Exception, e:
    print >> sys.stderr, 'error: Splitting of Emscripten generated Javascript failed: %s' % str(e)
  
  finally:
    if input_file is not None: input_file.close()
    if output_main_file is not None: output_main_file.close()
    if output_part_file is not None: output_part_file.close()
    if output_html_include_file is not None: output_html_include_file.close()