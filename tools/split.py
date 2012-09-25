import sys
    
def split_javascript_file(input_filename, output_filename_prefix, max_part_size_in_bytes):
 
    try:
        # Contains the entire Emscripten generated Javascript code
        input_file = open(input_filename,'r')
    
        # Output main file. This needs to be loaded last (!)
        output_main_filename = output_filename_prefix + ".js"
        output_main_file = open(output_main_filename,'w')

        # File with script tags to be included in HTML later on
        output_html_include_file = open(output_filename_prefix + ".include.html",'w')
    
        # Number of the current part
        output_part_file_counter = 1
        output_part_file = None

        # Flag to distinguish if we are in a function block or not
        is_function = False

        for line in input_file:
            
            if line == "//FUNCTION_BEGIN_MARKER\n":
                is_function = True
                # Open new part file if necessary
                if output_part_file is None:
                    output_part_filename = output_filename_prefix + ".part" + str(output_part_file_counter).zfill(5) + ".js"
                    output_part_file = open(output_part_filename,'w')
                    output_html_include_file.write("<script type=\"text/javascript\" src=\"" + output_part_filename + "\"></script>")
                    
            elif line == "//FUNCTION_END_MARKER\n":
                is_function = False
                # Close current part file if max size exceeded
                if output_part_file is not None and output_part_file.tell() > max_part_size_in_bytes:
                    output_part_file.close()
                    output_part_file = None
                    output_part_file_counter += 1
            else:
                if is_function: output_part_file.write(line)
                else: output_main_file.write(line)

        output_html_include_file.write("<script type=\"text/javascript\" src=\"" + output_main_filename + "\"></script>")

    except Exception, e:
      print >> sys.stderr, 'error: Splitting of Emscripten generated Javascript failed: %s' % str(e)

    finally:
         if input_file is not None: input_file.close()
         if output_main_file is not None: output_main_file.close()
         if output_part_file is not None: output_part_file.close()
         if output_html_include_file is not None: output_html_include_file.close()