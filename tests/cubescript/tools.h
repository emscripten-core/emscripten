// generic useful stuff for any C++ program

#ifndef _TOOLS_H
#define _TOOLS_H

#ifdef NULL
#undef NULL
#endif
#define NULL 0

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

#ifdef _DEBUG
#ifdef __GNUC__
#define ASSERT(c) if(!(c)) { asm("int $3"); }
#else
#define ASSERT(c) if(!(c)) { __asm int 3 }
#endif
#else
#define ASSERT(c) if(c) {}
#endif

#if defined(__GNUC__) || (defined(_MSC_VER) && _MSC_VER >= 1400)
#define RESTRICT __restrict
#else
#define RESTRICT
#endif

#ifdef swap
#undef swap
#endif
template<class T>
static inline void swap(T &a, T &b)
{
    T t = a;
    a = b;
    b = t;
}
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
template<class T>
static inline T max(T a, T b)
{
    return a > b ? a : b;
}
template<class T>
static inline T min(T a, T b)
{
    return a < b ? a : b;
}

#define clamp(a,b,c) (max(b, min(a, c)))
#define rnd(x) ((int)(randomMT()&0xFFFFFF)%(x))
#define rndscale(x) (float((randomMT()&0xFFFFFF)*double(x)/double(0xFFFFFF)))
#define detrnd(s, x) ((int)(((((uint)(s))*1103515245+12345)>>16)%(x)))

#define loop(v,m) for(int v = 0; v<int(m); v++)
#define loopi(m) loop(i,m)
#define loopj(m) loop(j,m)
#define loopk(m) loop(k,m)
#define loopl(m) loop(l,m)


#define DELETEP(p) if(p) { delete   p; p = 0; }
#define DELETEA(p) if(p) { delete[] p; p = 0; }

#define PI  (3.1415927f)
#define PI2 (2*PI)
#define SQRT2 (1.4142136f)
#define SQRT3 (1.7320508f)
#define RAD (PI / 180.0f)

#ifdef WIN32
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_LN2
#define M_LN2 0.693147180559945309417
#endif

#ifndef __GNUC__
#pragma warning (3: 4189)       // local variable is initialized but not referenced
#pragma warning (disable: 4244) // conversion from 'int' to 'float', possible loss of data
#pragma warning (disable: 4267) // conversion from 'size_t' to 'int', possible loss of data
#pragma warning (disable: 4355) // 'this' : used in base member initializer list
#pragma warning (disable: 4996) // 'strncpy' was declared deprecated
#endif

#define strcasecmp _stricmp
#define PATHDIV '\\'
#else
#define __cdecl
#define _vsnprintf vsnprintf
#define PATHDIV '/'
#endif

// easy safe strings

#define MAXSTRLEN 260
typedef char string[MAXSTRLEN];

inline void vformatstring(char *d, const char *fmt, va_list v, int len = MAXSTRLEN) { _vsnprintf(d, len, fmt, v); d[len-1] = 0; }
inline char *copystring(char *d, const char *s, size_t len = MAXSTRLEN) { strncpy(d, s, len); d[len-1] = 0; return d; }
inline char *concatstring(char *d, const char *s, size_t len = MAXSTRLEN) { size_t used = strlen(d); return used < len ? copystring(d+used, s, len-used) : d; }

struct stringformatter
{
    char *buf;
    stringformatter(char *buf): buf((char *)buf) {}
    void operator()(const char *fmt, ...)
    {
        va_list v;
        va_start(v, fmt);
        vformatstring(buf, fmt, v);
        va_end(v);
    }
};

#define formatstring(d) stringformatter((char *)d)
#define defformatstring(d) string d; formatstring(d)
#define defvformatstring(d,last,fmt) string d; { va_list ap; va_start(ap, last); vformatstring(d, fmt, ap); va_end(ap); }

#define loopv(v)    for(int i = 0; i<(v).length(); i++)
#define loopvj(v)   for(int j = 0; j<(v).length(); j++)
#define loopvk(v)   for(int k = 0; k<(v).length(); k++)
#define loopvrev(v) for(int i = (v).length()-1; i>=0; i--)

template <class T>
struct databuf
{
    enum
    {
        OVERREAD  = 1<<0,
        OVERWROTE = 1<<1
    };

    T *buf;
    int len, maxlen;
    uchar flags;

    databuf() : buf(NULL), len(0), maxlen(0), flags(0) {}

    template<class U> 
    databuf(T *buf, U maxlen) : buf(buf), len(0), maxlen((int)maxlen), flags(0) {}

    const T &get()
    {
        static T overreadval;
        if(len<maxlen) return buf[len++];
        flags |= OVERREAD;
        return overreadval;
    }

    databuf subbuf(int sz)
    {
        sz = min(sz, maxlen-len);
        len += sz;
        return databuf(&buf[len-sz], sz);
    }

    void put(const T &val)
    {
        if(len<maxlen) buf[len++] = val;
        else flags |= OVERWROTE;
    }

    void put(const T *vals, int numvals)
    {
        if(maxlen-len<numvals) flags |= OVERWROTE;
        memcpy(&buf[len], vals, min(maxlen-len, numvals)*sizeof(T));
        len += min(maxlen-len, numvals);
    }

    int get(T *vals, int numvals)
    {
        int read = min(maxlen-len, numvals);
        if(read<numvals) flags |= OVERREAD;
        memcpy(vals, &buf[len], read*sizeof(T));
        len += read;
        return read;
    }

    int length() const { return len; }
    int remaining() const { return maxlen-len; }
    bool overread() const { return (flags&OVERREAD)!=0; }
    bool overwrote() const { return (flags&OVERWROTE)!=0; }

    void forceoverread()
    {
        len = maxlen;
        flags |= OVERREAD;
    }
};

typedef databuf<char> charbuf;
typedef databuf<uchar> ucharbuf;

template<class T>
static inline float heapscore(const T &n) { return n; }

template<class T, class U>
static inline void quicksort(T *buf, int n, int (__cdecl *func)(U *, U *))
{
    qsort(buf, n, sizeof(T), (int (__cdecl *)(const void *,const void *))func);
}

template <class T> struct vector
{
    static const int MINSIZE = 8;

    T *buf;
    int alen, ulen;

    vector() : buf(NULL), alen(0), ulen(0)
    {
    }

    vector(const vector &v) : buf(NULL), alen(0), ulen(0)
    {
        *this = v;
    }

    ~vector() { shrink(0); if(buf) delete[] (uchar *)buf; }

    vector<T> &operator=(const vector<T> &v)
    {
        shrink(0);
        if(v.length() > alen) growbuf(v.length());
        loopv(v) add(v[i]);
        return *this;
    }

    T &add(const T &x)
    {
        if(ulen==alen) growbuf(ulen+1);
        new (&buf[ulen]) T(x);
        return buf[ulen++];
    }

    T &add()
    {
        if(ulen==alen) growbuf(ulen+1);
        new (&buf[ulen]) T;
        return buf[ulen++];
    }

    T &dup()
    {
        if(ulen==alen) growbuf(ulen+1);
        new (&buf[ulen]) T(buf[ulen-1]);
        return buf[ulen++];
    }

    void move(vector<T> &v)
    {
        if(!ulen)
        {
            swap(buf, v.buf);
            swap(ulen, v.ulen);
            swap(alen, v.alen);
        }
        else
        {
            growbuf(ulen+v.ulen);
            if(v.ulen) memcpy(&buf[ulen], v.buf, v.ulen*sizeof(T));
            ulen += v.ulen;
            v.ulen = 0;
        }
    }

    bool inrange(size_t i) const { return i<size_t(ulen); }
    bool inrange(int i) const { return i>=0 && i<ulen; }

    T &pop() { return buf[--ulen]; }
    T &last() { return buf[ulen-1]; }
    void drop() { ulen--; buf[ulen].~T(); }
    bool empty() const { return ulen==0; }

    int capacity() const { return alen; }
    int length() const { return ulen; }
    T &operator[](int i) { ASSERT(i>=0 && i<ulen); return buf[i]; }
    const T &operator[](int i) const { ASSERT(i >= 0 && i<ulen); return buf[i]; }
    
    void shrink(int i)         { ASSERT(i<=ulen); while(ulen>i) drop(); }
    void setsize(int i) { ASSERT(i<=ulen); ulen = i; }
    
    void deletecontents() { while(!empty()) delete   pop(); }
    void deletearrays() { while(!empty()) delete[] pop(); }
    
    T *getbuf() { return buf; }
    const T *getbuf() const { return buf; }
    bool inbuf(const T *e) const { return e >= buf && e < &buf[ulen]; }

    template<class ST>
    void sort(int (__cdecl *cf)(ST *, ST *), int i = 0, int n = -1) 
    { 
        quicksort(&buf[i], n < 0 ? ulen : n, cf);
    }

    void growbuf(int sz)
    {
        int olen = alen;
        if(!alen) alen = max(MINSIZE, sz);
        else while(alen < sz) alen *= 2;
        if(alen <= olen) return;
        uchar *newbuf = new uchar[alen*sizeof(T)];
        if(olen > 0)
        {
            memcpy(newbuf, buf, olen*sizeof(T));
            delete[] (uchar *)buf;
        }
        buf = (T *)newbuf;
    }

    databuf<T> reserve(int sz)
    {
        if(ulen+sz > alen) growbuf(ulen+sz);
        return databuf<T>(&buf[ulen], sz);
    }

    void advance(int sz)
    {
        ulen += sz;
    }

    void addbuf(const databuf<T> &p)
    {
        advance(p.length());
    }

    T *pad(int n)
    {
        T *buf = reserve(n).buf;
        advance(n);
        return buf;
    }

    void put(const T &v) { add(v); }

    void put(const T *v, int n)
    {
        databuf<T> buf = reserve(n);
        buf.put(v, n);
        addbuf(buf);
    }

    void remove(int i, int n)
    {
        for(int p = i+n; p<ulen; p++) buf[p-n] = buf[p];
        ulen -= n;
    }

    T remove(int i)
    {
        T e = buf[i];
        for(int p = i+1; p<ulen; p++) buf[p-1] = buf[p];
        ulen--;
        return e;
    }

    T removeunordered(int i)
    {
        T e = buf[i];
        ulen--;
        if(ulen>0) buf[i] = buf[ulen];
        return e;
    }

    template<class U>
    int find(const U &o)
    {
        loopi(ulen) if(buf[i]==o) return i;
        return -1;
    }
    
    void removeobj(const T &o)
    {
        loopi(ulen) if(buf[i]==o) remove(i--);
    }

    void replacewithlast(const T &o)
    {
        if(!ulen) return;
        loopi(ulen-1) if(buf[i]==o)
        {
            buf[i] = buf[ulen-1];
        }
        ulen--;
    }

    T &insert(int i, const T &e)
    {
        add(T());
        for(int p = ulen-1; p>i; p--) buf[p] = buf[p-1];
        buf[i] = e;
        return buf[i];
    }

    T *insert(int i, const T *e, int n)
    {
        if(ulen+n>alen) growbuf(ulen+n);
        loopj(n) add(T());
        for(int p = ulen-1; p>=i+n; p--) buf[p] = buf[p-n];
        loopj(n) buf[i+j] = e[j];
        return &buf[i];
    }

    void reverse()
    {
        loopi(ulen/2) swap(buf[i], buf[ulen-1-i]);
    }

    static int heapparent(int i) { return (i - 1) >> 1; }
    static int heapchild(int i) { return (i << 1) + 1; }

    void buildheap()
    {
        for(int i = ulen/2; i >= 0; i--) downheap(i);
    }

    int upheap(int i)
    {
        float score = heapscore(buf[i]);
        while(i > 0)
        {
            int pi = heapparent(i);
            if(score >= heapscore(buf[pi])) break;
            swap(buf[i], buf[pi]);
            i = pi;
        }
        return i;
    }

    T &addheap(const T &x)
    {
        add(x);
        return buf[upheap(ulen-1)];
    }

    int downheap(int i)
    {
        float score = heapscore(buf[i]);
        for(;;)
        {
            int ci = heapchild(i);
            if(ci >= ulen) break;
            float cscore = heapscore(buf[ci]);
            if(score > cscore)
            {
               if(ci+1 < ulen && heapscore(buf[ci+1]) < cscore) { swap(buf[ci+1], buf[i]); i = ci+1; }
               else { swap(buf[ci], buf[i]); i = ci; }
            }
            else if(ci+1 < ulen && heapscore(buf[ci+1]) < score) { swap(buf[ci+1], buf[i]); i = ci+1; }
            else break;
        }
        return i;
    }

    T removeheap()
    {
        T e = removeunordered(0);
        if(ulen) downheap(0);
        return e;
    }
};

static inline uint hthash(const char *key)
{
    uint h = 5381;
    for(int i = 0, k; (k = key[i]); i++) h = ((h<<5)+h)^k;    // bernstein k=33 xor
    return h;
}

static inline bool htcmp(const char *x, const char *y)
{
    return !strcmp(x, y);
}

static inline uint hthash(int key)
{   
    return key;
}

static inline bool htcmp(int x, int y)
{
    return x==y;
}

template<class T> struct hashset
{
    typedef T elem;
    typedef const T const_elem;

    enum { CHUNKSIZE = 64 };

    struct chain { T elem; chain *next; };
    struct chainchunk { chain chains[CHUNKSIZE]; chainchunk *next; };

    int size;
    int numelems;
    chain **chains;

    chainchunk *chunks;
    chain *unused;

    hashset(int size = 1<<10)
      : size(size)
    {
        numelems = 0;
        chunks = NULL;
        unused = NULL;
        chains = new chain *[size];
        loopi(size) chains[i] = NULL;
    }

    ~hashset()
    {
        DELETEA(chains);
        deletechunks();
    }

    chain *insert(uint h)
    {
        if(!unused)
        {
            chainchunk *chunk = new chainchunk;
            chunk->next = chunks;
            chunks = chunk;
            loopi(CHUNKSIZE-1) chunk->chains[i].next = &chunk->chains[i+1];
            chunk->chains[CHUNKSIZE-1].next = unused;
            unused = chunk->chains;
        }
        chain *c = unused;
        unused = unused->next;
        c->next = chains[h];
        chains[h] = c;
        numelems++;
        return c;
    }
     
    #define HTFIND(key, success, fail) \
        uint h = hthash(key)&(this->size-1); \
        for(chain *c = this->chains[h]; c; c = c->next) \
        { \
            if(htcmp(key, c->elem)) return (success); \
        } \
        return (fail);

    template<class K>
    T *access(const K &key)
    {
        HTFIND(key, &c->elem, NULL);
    }

    template<class K>
    T &access(const K &key, const T &elem)
    {
        HTFIND(key, c->elem, insert(h)->elem = elem);
    }

    template<class K>
    T &operator[](const K &key)
    {
        HTFIND(key, c->elem, insert(h)->elem);
    }

    template<class K>
    bool remove(const K &key)
    {
        uint h = hthash(key)&(size-1);
        for(chain **p = &chains[h], *c = chains[h]; c; p = &c->next, c = c->next)
        {
            if(htcmp(key, c->elem))
            {
                *p = c->next;
                c->elem.~T();
                new (&c->elem) T;
                c->next = unused;
                unused = c;
                numelems--;
                return true;
            }
        }
        return false;
    }

    void deletechunks()
    {
        for(chainchunk *nextchunk; chunks; chunks = nextchunk)
        {
            nextchunk = chunks->next;
            delete chunks;
        }
    }

    void clear()
    {
        if(!numelems) return;
        loopi(size) chains[i] = NULL;
        numelems = 0;
        unused = NULL;
        deletechunks();
    }

    static inline chain *getnext(void *i) { return ((chain *)i)->next; }
    static inline T &getdata(void *i) { return ((chain *)i)->elem; }
};

template<class K, class T> struct hashtableentry
{
    K key;
    T data;

    hashtableentry() {}
    hashtableentry(const K &key, const T &data) : key(key), data(data) {}
};

template<class U, class K, class T>
static inline bool htcmp(const U *x, const hashtableentry<K, T> &y)
{
    return htcmp(x, y.key);
}

template<class U, class K, class T>
static inline bool htcmp(const U &x, const hashtableentry<K, T> &y)
{
    return htcmp(x, y.key);
}

template<class K, class T> struct hashtable : hashset<hashtableentry<K, T> >
{
    typedef hashtableentry<K, T> entry;
    typedef struct hashset<entry>::chain chain;
    typedef K key;
    typedef T value;

    hashtable(int size = 1<<10) : hashset<entry>(size) {}

    entry &insert(const K &key, uint h)
    {
        chain *c = hashset<entry>::insert(h);
        c->elem.key = key;
        return c->elem;
    }
    
    T *access(const K &key)
    {
        HTFIND(key, &c->elem.data, NULL);
    }

    T &access(const K &key, const T &data)
    {
        HTFIND(key, c->elem.data, insert(key, h).data = data);
    }

    T &operator[](const K &key)
    {
        HTFIND(key, c->elem.data, insert(key, h).data);
    }

    static inline chain *getnext(void *i) { return ((chain *)i)->next; }
    static inline K &getkey(void *i) { return ((chain *)i)->elem.key; }
    static inline T &getdata(void *i) { return ((chain *)i)->elem.data; }
};

#define enumerates(ht,t,e,b)      loopi((ht).size)  for(hashset<t>::chain *enumc = (ht).chains[i]; enumc;) { t &e = enumc->elem; enumc = enumc->next; b; }
#define enumeratekt(ht,k,e,t,f,b) loopi((ht).size)  for(hashtable<k,t>::chain *enumc = (ht).chains[i]; enumc;) { const hashtable<k,t>::key &e = enumc->elem.key; t &f = enumc->elem.data; enumc = enumc->next; b; }
#define enumerate(ht,t,e,b)       loopi((ht).size) for(void *enumc = (ht).chains[i]; enumc;) { t &e = (ht).getdata(enumc); enumc = (ht).getnext(enumc); b; }

struct unionfind
{
    struct ufval
    {
        int rank, next;

        ufval() : rank(0), next(-1) {}
    };
    
    vector<ufval> ufvals;

    int find(int k)
    {
        if(k>=ufvals.length()) return k;
        while(ufvals[k].next>=0) k = ufvals[k].next;
        return k;
    }
    
    int compressfind(int k)
    {
        if(ufvals[k].next<0) return k;
        return ufvals[k].next = compressfind(ufvals[k].next);
    }
    
    void unite (int x, int y)
    {
        while(ufvals.length() <= max(x, y)) ufvals.add();
        x = compressfind(x);
        y = compressfind(y);
        if(x==y) return;
        ufval &xval = ufvals[x], &yval = ufvals[y];
        if(xval.rank < yval.rank) xval.next = y;
        else
        {
            yval.next = x;
            if(xval.rank==yval.rank) yval.rank++;
        }
    }
};

template <class T, int SIZE> struct ringbuf
{
    int index, len;
    T data[SIZE];

    ringbuf() { clear(); }

    void clear()
    {
        index = len = 0;
    }

    bool empty() const { return !len; }

    const int length() const { return len; }

    T &add(const T &e)
    {
        T &t = (data[index] = e);
        index++;
        if(index >= SIZE) index -= SIZE;
        if(len < SIZE) len++;
        return t;
    }

    T &add() { return add(T()); }

    T &operator[](int i)
    {
        i += index - len;
        return data[i < 0 ? i + SIZE : i%SIZE];
    }

    const T &operator[](int i) const
    {
        i += index - len;
        return data[i < 0 ? i + SIZE : i%SIZE];
    }
};

template <class T, int SIZE> struct queue
{
    int head, tail, len;
    T data[SIZE];
    
    queue() { clear(); }
    
    void clear() { head = tail = len = 0; }

    int length() const { return len; }
    bool empty() const { return !len; }
    bool full() const { return len == SIZE; }

    T &added() { return data[tail > 0 ? tail-1 : SIZE-1]; }
    T &added(int offset) { return data[tail-offset > 0 ? tail-offset-1 : tail-offset-1 + SIZE]; }
    T &adding() { return data[tail]; }
    T &adding(int offset) { return data[tail+offset >= SIZE ? tail+offset - SIZE : tail+offset]; }
    T &add()
    {
        ASSERT(len < SIZE);    
        T &t = data[tail];
        tail = (tail + 1)%SIZE;
        len++;
        return t;
    }

    T &removing() { return data[head]; }
    T &removing(int offset) { return data[head+offset >= SIZE ? head+offset - SIZE : head+offset]; }
    T &remove()
    {
        ASSERT(len > 0);
        T &t = data[head];
        head = (head + 1)%SIZE;
        len--;
        return t;
    }
};

inline char *newstring(size_t l)                { return new char[l+1]; }
inline char *newstring(const char *s, size_t l) { return copystring(newstring(l), s, l+1); }
inline char *newstring(const char *s)           { return newstring(s, strlen(s));          }
inline char *newstringbuf(const char *s)        { return newstring(s, MAXSTRLEN-1);       }

#if defined(WIN32) && !defined(__GNUC__)
#ifdef _DEBUG
//#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
inline void *__cdecl operator new(size_t n, const char *fn, int l) { return ::operator new(n, 1, fn, l); }
inline void __cdecl operator delete(void *p, const char *fn, int l) { ::operator delete(p, 1, fn, l); }
#define new new(__FILE__,__LINE__)
#endif 
#endif

const int islittleendian = 1;
#ifdef SDL_BYTEORDER
#define endianswap16 SDL_Swap16
#define endianswap32 SDL_Swap32
#else
inline ushort endianswap16(ushort n) { return (n<<8) | (n>>8); }
inline uint endianswap32(uint n) { return (n<<24) | (n>>24) | ((n>>8)&0xFF00) | ((n<<8)&0xFF0000); }
#endif
template<class T> inline T endianswap(T n) { union { T t; uint i; } conv; conv.t = n; conv.i = endianswap32(conv.i); return conv.t; }
template<> inline ushort endianswap<ushort>(ushort n) { return endianswap16(n); }
template<> inline short endianswap<short>(short n) { return endianswap16(n); }
template<> inline uint endianswap<uint>(uint n) { return endianswap32(n); }
template<> inline int endianswap<int>(int n) { return endianswap32(n); }
template<class T> inline void endianswap(T *buf, int len) { for(T *end = &buf[len]; buf < end; buf++) *buf = endianswap(*buf); }
template<class T> inline T endiansame(T n) { return n; }
template<class T> inline void endiansame(T *buf, int len) {}
#ifdef SDL_BYTEORDER
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define lilswap endiansame
#define bigswap endianswap
#else
#define lilswap endianswap
#define bigswap endiansame
#endif
#else
template<class T> inline T lilswap(T n) { return *(const uchar *)&islittleendian ? n : endianswap(n); }
template<class T> inline void lilswap(T *buf, int len) { if(!*(const uchar *)&islittleendian) endianswap(buf, len); }
template<class T> inline T bigswap(T n) { return *(const uchar *)&islittleendian ? endianswap(n) : n; }
template<class T> inline void bigswap(T *buf, int len) { if(*(const uchar *)&islittleendian) endianswap(buf, len); }
#endif

/* workaround for some C platforms that have these two functions as macros - not used anywhere */
#ifdef getchar
#undef getchar
#endif
#ifdef putchar
#undef putchar
#endif

struct stream
{
    virtual ~stream() {}
    virtual void close() = 0;
    virtual bool end() = 0;
    virtual long tell() { return -1; }
    virtual bool seek(long offset, int whence = SEEK_SET) { return false; }
    virtual long size();
    virtual int read(void *buf, int len) { return 0; }
    virtual int write(const void *buf, int len) { return 0; }
    virtual int getchar() { uchar c; return read(&c, 1) == 1 ? c : -1; }
    virtual bool putchar(int n) { uchar c = n; return write(&c, 1) == 1; }
    virtual bool getline(char *str, int len);
    virtual bool putstring(const char *str) { int len = (int)strlen(str); return write(str, len) == len; }
    virtual bool putline(const char *str) { return putstring(str) && putchar('\n'); }
    virtual int printf(const char *fmt, ...) { return -1; }
    virtual uint getcrc() { return 0; }

    template<class T> bool put(T n) { return write(&n, ES_SIZEOV(n)) == ES_SIZEOV(n); }
    template<class T> bool putlil(T n) { return put<T>(lilswap(n)); }
    template<class T> bool putbig(T n) { return put<T>(bigswap(n)); }

    template<class T> T get() { T n; return read(&n, ES_SIZEOV(n)) == ES_SIZEOV(n) ? n : 0; }
    template<class T> T getlil() { return lilswap(get<T>()); }
    template<class T> T getbig() { return bigswap(get<T>()); }
};

#endif

