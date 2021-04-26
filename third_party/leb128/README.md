# LEB128

LEB128 or Little Endian Base 128 is a form of variable-length code compression used to store an arbitrarily large integer in a small number of bytes. LEB128 is used in the DWARF debug file format and the WebAssembly binary encoding for all integer literals.

```sh
$ pip3 install leb128
```

`leb128` is used in [pywasm](https://github.com/mohanson/pywasm), which the WebAssembly virtual machine.

# Example

```py
import io
import leb128

# unsigned leb128
assert leb128.u.encode(624485) == bytearray([0xe5, 0x8e, 0x26])
assert leb128.u.decode(bytearray([0xe5, 0x8e, 0x26])) == 624485
assert leb128.u.decode_reader(io.BytesIO(bytearray([0xe5, 0x8e, 0x26]))) == (624485, 3)

# signed leb128
assert leb128.i.encode(-123456) == bytearray([0xc0, 0xbb, 0x78])
assert leb128.i.decode(bytearray([0xc0, 0xbb, 0x78])) == -123456
assert leb128.i.decode_reader(io.BytesIO(bytearray([0xc0, 0xbb, 0x78]))) == (-123456, 3)
```

# Performance

Since I used the most optimized algorithm, it is likely to be the fastest among all pure Python implementations of leb128. The detailed results can refer to the table, which is the result of using a very low-performance CPU.

|          Case          | Duration |
| ---------------------- | -------- |
| U encode 1000000 times | 0.865 s  |
| U decode 1000000 times | 0.808 s  |
| I encode 1000000 times | 0.762 s  |
| I decode 1000000 times | 0.835 s  |

# License

MIT
