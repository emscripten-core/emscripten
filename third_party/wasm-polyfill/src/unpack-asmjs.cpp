// vim: set ts=2 sw=2 tw=99 et:

#include "unpack.h"

#include <stdexcept>
#include <iostream>
#include <fstream>

using namespace std;

int
main(int argc, char** argv)
try
{
  if (argc < 3 || argc > 4) {
    cerr <<  "Usage: unpack-asmjs in.wasm out.js [name]" << endl;
    return -1;
  }

  const char* in_file_name = argv[1];
  const char* out_file_name = argv[2];
  const char* name = argc > 3 ? argv[3] : nullptr;

  // Read in packed .asm file bytes.
  vector<uint8_t> in_bytes;
  ifstream in_stream(in_file_name, ios::binary | ios::ate);
  in_stream.exceptions(ios::failbit | ios::badbit);
  in_bytes.resize(in_stream.tellg());
  in_stream.seekg(0);
  in_stream.read((char*)in_bytes.data(), in_bytes.size());
  in_stream.close();

  if (!asmjs::has_magic_number(in_bytes.data())) {
    cerr << in_file_name << " isn't a packed asm.js file" << endl;
    return -1;
  }

  // Unpack .asm file into utf8 chars.
  uint32_t unpacked_size = asmjs::unpacked_size(in_bytes.data(), name);
  vector<uint8_t> out_bytes(unpacked_size);
  asmjs::unpack(in_bytes.data(), name, out_bytes.size(), out_bytes.data());

  // Write the utf8 chars out to a .js file.
  ofstream out_stream(out_file_name, ios::binary);
  out_stream.exceptions(ios::failbit | ios::badbit);
  out_stream.write((char*)out_bytes.data(), out_bytes.size());
  out_stream.close();

  return 0;
}
catch (const ios::failure& err)
{
  cerr << "Failed with iostream error: " << err.what() << endl;
  return -1;
}
catch(const runtime_error& err)
{
  cerr << "Failed with runtime error: " << err.what() << endl;
  return -1;
}
