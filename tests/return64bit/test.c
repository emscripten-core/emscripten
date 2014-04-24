
// This is just a trivial test function, the key bit of interest is that it returns a 64 bit long.
long long test() {
    long long x = ((long long)1234 << 32) + 5678;
    return x;
}
