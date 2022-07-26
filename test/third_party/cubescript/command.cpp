// command.cpp: implements the parsing and execution of a tiny script language which
// is largely backwards compatible with the quake console language.

// XXX Emscripten
  #define STANDALONE

  #include "limits.h"
  #include "stdarg.h"
  #include "string.h"
  #include "stdio.h"
  #include "stdlib.h"
  #include "ctype.h"
  #include "math.h"
  #include "time.h"
  #include <new>

  #include "tools.h"
  #include "command.h"

  // console
  enum
  {
      CON_INFO  = 1<<0,
      CON_WARN  = 1<<1,
      CON_ERROR = 1<<2,
      CON_DEBUG = 1<<3,
      CON_INIT  = 1<<4,
      CON_ECHO  = 1<<5
  };
  extern void conoutf(const char *s, ...);
  extern void conoutf(int type, const char *s, ...);

  // command
  extern int variable(const char *name, int min, int cur, int max, int *storage, void (*fun)(), int flags);
  extern float fvariable(const char *name, float min, float cur, float max, float *storage, void (*fun)(), int flags);
  extern char *svariable(const char *name, const char *cur, char **storage, void (*fun)(), int flags);
  extern void setvar(const char *name, int i, bool dofunc = true, bool doclamp = true);
  extern void setfvar(const char *name, float f, bool dofunc = true, bool doclamp = true);
  extern void setsvar(const char *name, const char *str, bool dofunc = true);
  extern void setvarchecked(ident *id, int val);
  extern void setfvarchecked(ident *id, float val);
  extern void setsvarchecked(ident *id, const char *val);
  extern void touchvar(const char *name);
  extern int getvar(const char *name);
  extern int getvarmin(const char *name);
  extern int getvarmax(const char *name);
  extern bool identexists(const char *name);
  extern ident *getident(const char *name);
  extern ident *newident(const char *name);
  extern bool addcommand(const char *name, void (*fun)(), const char *narg);
  extern int execute(const char *p);
  extern char *executeret(const char *p);
  extern bool execfile(const char *cfgfile, bool msg = true);
  extern void alias(const char *name, const char *action);
  extern const char *getalias(const char *name);

  // main
  extern void fatal(const char *s, ...);

  extern char *path(char *s);
  extern char *path(const char *s, bool copy);
  extern const char *parentdir(const char *directory);
  extern bool fileexists(const char *path, const char *mode);
  extern bool createdir(const char *path);
  extern size_t fixpackagedir(char *dir);
  extern void sethomedir(const char *dir);
  extern void addpackagedir(const char *dir);
  extern const char *findfile(const char *filename, const char *mode);
  extern stream *openrawfile(const char *filename, const char *mode);
  extern stream *openzipfile(const char *filename, const char *mode);
  extern stream *openfile(const char *filename, const char *mode);
  extern stream *opentempfile(const char *filename, const char *mode);
  extern char *loadfile(const char *fn, int *size);
  extern bool listdir(const char *dir, const char *ext, vector<char *> &files);
  extern int listfiles(const char *dir, const char *ext, vector<char *> &files);
  extern int listzipfiles(const char *dir, const char *ext, vector<char *> &files);
  extern void seedMT(uint seed);
  extern uint randomMT(void);
// XXX =========================

char *exchangestr(char *o, const char *n) { delete[] o; return newstring(n); }

typedef hashtable<const char *, ident> identtable;

identtable *idents = NULL;        // contains ALL vars/commands/aliases

bool overrideidents = false, persistidents = true;

void clearstack(ident &id)
{
    identstack *stack = id.stack;
    while(stack)
    {
        delete[] stack->action;
        identstack *tmp = stack;
        stack = stack->next;
        delete tmp;
    }
    id.stack = NULL;
}

void clear_command()
{
    enumerate(*idents, ident, i, if(i.type==ID_ALIAS) { DELETEA(i.name); DELETEA(i.action); if(i.stack) clearstack(i); });
    if(idents) idents->clear();
}

void clearoverride(ident &i)
{
    if(i.override==NO_OVERRIDE) return;
    switch(i.type)
    {
        case ID_ALIAS:
            if(i.action[0])
            {
                if(i.action != i.isexecuting) delete[] i.action;
                i.action = newstring("");
            }
            break;
        case ID_VAR:
            *i.storage.i = i.overrideval.i;
            i.changed();
            break;
        case ID_FVAR:
            *i.storage.f = i.overrideval.f;
            i.changed();
            break;
        case ID_SVAR:
            delete[] *i.storage.s;
            *i.storage.s = i.overrideval.s;
            i.changed();
            break;
    }
    i.override = NO_OVERRIDE;
}

void clearoverrides()
{
    enumerate(*idents, ident, i, clearoverride(i));
}

void pushident(ident &id, char *val)
{
    if(id.type != ID_ALIAS) return;
    identstack *stack = new identstack;
    stack->action = id.isexecuting==id.action ? newstring(id.action) : id.action;
    stack->next = id.stack;
    id.stack = stack;
    id.action = val;
}

void popident(ident &id)
{
    if(id.type != ID_ALIAS || !id.stack) return;
    if(id.action != id.isexecuting) delete[] id.action;
    identstack *stack = id.stack;
    id.action = stack->action;
    id.stack = stack->next;
    delete stack;
}

ident *newident(const char *name)
{
    ident *id = idents->access(name);
    if(!id)
    {
        ident init(ID_ALIAS, newstring(name), newstring(""), persistidents ? IDF_PERSIST : 0);
        id = &idents->access(init.name, init);
    }
    return id;
}

void pusha(const char *name, char *action)
{
    pushident(*newident(name), action);
}

void push(char *name, char *action)
{
    pusha(name, newstring(action));
}

void pop(char *name)
{
    ident *id = idents->access(name);
    if(id) popident(*id);
}

void resetvar(char *name)
{
    ident *id = idents->access(name);
    if(!id) return;
    if(id->flags&IDF_READONLY) conoutf(CON_ERROR, "variable %s is read-only", id->name);
    else clearoverride(*id);
}

COMMAND(push, "ss");
COMMAND(pop, "s");
COMMAND(resetvar, "s");

void aliasa(const char *name, char *action)
{
    ident *b = idents->access(name);
    if(!b)
    {
        ident b(ID_ALIAS, newstring(name), action, persistidents ? IDF_PERSIST : 0);
        if(overrideidents) b.override = OVERRIDDEN;
        idents->access(b.name, b);
    }
    else if(b->type != ID_ALIAS)
    {
        conoutf(CON_ERROR, "cannot redefine builtin %s with an alias", name);
        delete[] action;
    }
    else
    {
        if(b->action != b->isexecuting) delete[] b->action;
        b->action = action;
        if(overrideidents) b->override = OVERRIDDEN;
        else
        {
            if(b->override != NO_OVERRIDE) b->override = NO_OVERRIDE;
            if(persistidents)
            {
                if(!(b->flags & IDF_PERSIST)) b->flags |= IDF_PERSIST;
            }
            else if(b->flags & IDF_PERSIST) b->flags &= ~IDF_PERSIST;
        }
    }
}

void alias(const char *name, const char *action) { aliasa(name, newstring(action)); }

COMMAND(alias, "ss");

// variable's and commands are registered through globals, see cube.h

int variable(const char *name, int min, int cur, int max, int *storage, void (*fun)(), int flags)
{
    if(!idents) idents = new identtable;
    ident v(ID_VAR, name, min, cur, max, storage, (void *)fun, flags);
    idents->access(name, v);
    return cur;
}

float fvariable(const char *name, float min, float cur, float max, float *storage, void (*fun)(), int flags)
{
    if(!idents) idents = new identtable;
    ident v(ID_FVAR, name, min, cur, max, storage, (void *)fun, flags);
    idents->access(name, v);
    return cur;
}

char *svariable(const char *name, const char *cur, char **storage, void (*fun)(), int flags)
{
    if(!idents) idents = new identtable;
    ident v(ID_SVAR, name, newstring(cur), storage, (void *)fun, flags);
    idents->access(name, v);
    return v.val.s;
}

#define _GETVAR(id, vartype, name, retval) \
    ident *id = idents->access(name); \
    if(!id || id->type!=vartype) return retval;
#define GETVAR(id, name, retval) _GETVAR(id, ID_VAR, name, retval)
#define OVERRIDEVAR(errorval, saveval, resetval, clearval) \
    if(overrideidents || id->flags&IDF_OVERRIDE) \
    { \
        if(id->flags&IDF_PERSIST) \
        { \
            conoutf(CON_ERROR, "cannot override persistent variable %s", id->name); \
            errorval; \
        } \
        if(id->override==NO_OVERRIDE) { saveval; id->override = OVERRIDDEN; } \
        else { clearval; } \
    } \
    else \
    { \
        if(id->override!=NO_OVERRIDE) { resetval; id->override = NO_OVERRIDE; } \
        clearval; \
    }

void setvar(const char *name, int i, bool dofunc, bool doclamp)
{
    GETVAR(id, name, );
    OVERRIDEVAR(return, id->overrideval.i = *id->storage.i, , )
    if(doclamp) *id->storage.i = clamp(i, id->minval, id->maxval);
    else *id->storage.i = i;
    if(dofunc) id->changed();
}
void setfvar(const char *name, float f, bool dofunc, bool doclamp)
{
    _GETVAR(id, ID_FVAR, name, );
    OVERRIDEVAR(return, id->overrideval.f = *id->storage.f, , );
    if(doclamp) *id->storage.f = clamp(f, id->minvalf, id->maxvalf);
    else *id->storage.f = f;
    if(dofunc) id->changed();
}
void setsvar(const char *name, const char *str, bool dofunc)
{
    _GETVAR(id, ID_SVAR, name, );
    OVERRIDEVAR(return, id->overrideval.s = *id->storage.s, delete[] id->overrideval.s, delete[] *id->storage.s);
    *id->storage.s = newstring(str);
    if(dofunc) id->changed();
}
int getvar(const char *name)
{
    GETVAR(id, name, 0);
    return *id->storage.i;
}
int getvarmin(const char *name)
{
    GETVAR(id, name, 0);
    return id->minval;
}
int getvarmax(const char *name)
{
    GETVAR(id, name, 0);
    return id->maxval;
}
bool identexists(const char *name) { return idents->access(name)!=NULL; }
ident *getident(const char *name) { return idents->access(name); }

void touchvar(const char *name)
{
    ident *id = idents->access(name);
    if(id) switch(id->type)
    {
        case ID_VAR:
        case ID_FVAR:
        case ID_SVAR:
            id->changed();
            break;
    }
}

const char *getalias(const char *name)
{
    ident *i = idents->access(name);
    return i && i->type==ID_ALIAS ? i->action : "";
}

void setvarchecked(ident *id, int val)
{
    if(id->flags&IDF_READONLY) conoutf(CON_ERROR, "variable %s is read-only", id->name);
#ifndef STANDALONE
    else if(!(id->flags&IDF_OVERRIDE) || overrideidents || game::allowedittoggle())
#else
    else
#endif
    {
        OVERRIDEVAR(return, id->overrideval.i = *id->storage.i, , )
        if(val<id->minval || val>id->maxval)
        {
            val = val<id->minval ? id->minval : id->maxval;                // clamp to valid range
            conoutf(CON_ERROR,
                id->flags&IDF_HEX ?
                    (id->minval <= 255 ? "valid range for %s is %d..0x%X" : "valid range for %s is 0x%X..0x%X") :
                    "valid range for %s is %d..%d",
                id->name, id->minval, id->maxval);
        }
        *id->storage.i = val;
        id->changed();                                             // call trigger function if available
#ifndef STANDALONE
        if(id->flags&IDF_OVERRIDE && !overrideidents) game::vartrigger(id);
#endif
    }
}

void setfvarchecked(ident *id, float val)
{
    if(id->flags&IDF_READONLY) conoutf(CON_ERROR, "variable %s is read-only", id->name);
#ifndef STANDALONE
    else if(!(id->flags&IDF_OVERRIDE) || overrideidents || game::allowedittoggle())
#else
    else
#endif
    {
        OVERRIDEVAR(return, id->overrideval.f = *id->storage.f, , );
        if(val<id->minvalf || val>id->maxvalf)
        {
            val = val<id->minvalf ? id->minvalf : id->maxvalf;                // clamp to valid range
            conoutf(CON_ERROR, "valid range for %s is %s..%s", id->name, floatstr(id->minvalf), floatstr(id->maxvalf));
        }
        *id->storage.f = val;
        id->changed();
#ifndef STANDALONE
        if(id->flags&IDF_OVERRIDE && !overrideidents) game::vartrigger(id);
#endif
    }
}

void setsvarchecked(ident *id, const char *val)
{
    if(id->flags&IDF_READONLY) conoutf(CON_ERROR, "variable %s is read-only", id->name);
#ifndef STANDALONE
    else if(!(id->flags&IDF_OVERRIDE) || overrideidents || game::allowedittoggle())
#else
    else
#endif
    {
        OVERRIDEVAR(return, id->overrideval.s = *id->storage.s, delete[] id->overrideval.s, delete[] *id->storage.s);
        *id->storage.s = newstring(val);
        id->changed();
#ifndef STANDALONE
        if(id->flags&IDF_OVERRIDE && !overrideidents) game::vartrigger(id);
#endif
    }
}

bool addcommand(const char *name, void (*fun)(), const char *narg)
{
    if(!idents) idents = new identtable;
    ident c(ID_COMMAND, name, narg, (void *)fun);
    idents->access(name, c);
    return false;
}

void addident(const char *name, ident *id)
{
    if(!idents) idents = new identtable;
    idents->access(name, *id);
}

static vector<vector<char> *> wordbufs;
static int bufnest = 0;

char *parseexp(const char *&p, int right);

void parsemacro(const char *&p, int level, vector<char> &wordbuf)
{
    int escape = 1;
    while(*p=='@') p++, escape++;
    if(level > escape)
    {
        while(escape--) wordbuf.add('@');
        return;
    }
    if(*p=='(')
    {
        char *ret = parseexp(p, ')');
        if(ret)
        {
            for(char *sub = ret; *sub; ) wordbuf.add(*sub++);
            delete[] ret;
        }
        return;
    }
    static vector<char> ident;
    ident.setsize(0);
    while(isalnum(*p) || *p=='_') ident.add(*p++);
    ident.add(0);
    const char *alias = getalias(ident.getbuf());
    while(*alias) wordbuf.add(*alias++);
}

const char *parsestring(const char *p)
{
    for(; *p; p++) switch(*p)
    {
        case '\r':
        case '\n':
        case '\"':
            return p;
        case '^':
            if(*++p) break;
            return p;
    }
    return p;
}

int escapestring(char *dst, const char *src, const char *end)
{
    char *start = dst;
    while(src < end)
    {
        int c = *src++;
        if(c == '^')
        {
            if(src >= end) break;
            int e = *src++;
            switch(e)
            {
                case 'n': *dst++ = '\n'; break;
                case 't': *dst++ = '\t'; break;
                case 'f': *dst++ = '\f'; break;
                default: *dst++ = e; break;
            }
        }
        else *dst++ = c;
    }
    return dst - start;
}

char *parseexp(const char *&p, int right)          // parse any nested set of () or []
{
    if(bufnest++>=wordbufs.length()) wordbufs.add(new vector<char>);
    vector<char> &wordbuf = *wordbufs[bufnest-1];
    int left = *p++;
    for(int brak = 1; brak; )
    {
        size_t n = strcspn(p, "\r@\"/()[]");
        wordbuf.put(p, n);
        p += n;

        int c = *p++;
        switch(c)
        {
            case '\r': continue;
            case '@':
                if(left == '[') { parsemacro(p, brak, wordbuf); continue; }
                break;
            case '\"':
            {
                wordbuf.add(c);
                const char *end = parsestring(p);
                wordbuf.put(p, end - p);
                p = end;
                if(*p=='\"') wordbuf.add(*p++);
                continue;
            }
            case '/':
                if(*p=='/')
                {
                    p += strcspn(p, "\n\0");
                    continue;
                }
                break;
            case '\0':
                p--;
                conoutf(CON_ERROR, "missing \"%c\"", right);
                wordbuf.setsize(0);
                bufnest--;
                return NULL;
            case '(': case '[': if(c==left) brak++; break;
            case ')': case ']': if(c==right) brak--; break;
        }
        wordbuf.add(c);
    }
    wordbuf.pop();
    char *s;
    if(left=='(')
    {
        wordbuf.add(0);
        char *ret = executeret(wordbuf.getbuf());                    // evaluate () exps directly, and substitute result
        wordbuf.pop();
        s = ret ? ret : newstring("");
    }
    else
    {
        s = newstring(wordbuf.getbuf(), wordbuf.length());
    }
    wordbuf.setsize(0);
    bufnest--;
    return s;
}

char *lookup(char *n)                           // find value of ident referenced with $ in exp
{
    ident *id = idents->access(n+1);
    if(id) switch(id->type)
    {
        case ID_VAR: return exchangestr(n, intstr(*id->storage.i));
        case ID_FVAR: return exchangestr(n, floatstr(*id->storage.f));
        case ID_SVAR: return exchangestr(n, *id->storage.s);
        case ID_ALIAS: return exchangestr(n, id->action);
    }
    conoutf(CON_ERROR, "unknown alias lookup: %s", n+1);
    return n;
}

char *parseword(const char *&p, int arg, int &infix)                       // parse single argument, including expressions
{
    for(;;)
    {
        p += strspn(p, " \t\r");
        if(p[0]!='/' || p[1]!='/') break;
        p += strcspn(p, "\n\0");
    }
    if(*p=='\"')
    {
        p++;
        const char *end = parsestring(p);
        char *s = newstring(end - p);
        s[escapestring(s, p, end)] = '\0';
        p = end;
        if(*p=='\"') p++;
        return s;
    }
    if(*p=='(') return parseexp(p, ')');
    if(*p=='[') return parseexp(p, ']');
    const char *word = p;
    for(;;)
    {
        p += strcspn(p, "/; \t\r\n\0");
        if(p[0]!='/' || p[1]=='/') break;
        else if(p[1]=='\0') { p++; break; }
        p += 2;
    }
    if(p-word==0) return NULL;
    if(arg==1 && p-word==1) switch(*word)
    {
        case '=': infix = *word; break;
    }
    char *s = newstring(word, p-word);
    if(*s=='$') return lookup(s);               // substitute variables
    return s;
}

char *conc(char **w, int n, bool space)
{
    int len = space ? max(n-1, 0) : 0;
    loopj(n) len += (int)strlen(w[j]);
    char *r = newstring("", len);
    loopi(n)
    {
        strcat(r, w[i]);  // make string-list out of all arguments
        if(i==n-1) break;
        if(space) strcat(r, " ");
    }
    return r;
}

VARN(numargs, _numargs, 25, 0, 0);

static inline bool isinteger(char *c)
{
    return isdigit(c[0]) || ((c[0]=='+' || c[0]=='-' || c[0]=='.') && isdigit(c[1]));
}

char *commandret = NULL;

char *executeret(const char *p)               // all evaluation happens here, recursively
{
    const int MAXWORDS = 25;                    // limit, remove
    char *w[MAXWORDS];
    char *retval = NULL;
    #define setretval(v) { char *rv = v; if(rv) retval = rv; }
    for(bool cont = true; cont;)                // for each ; seperated statement
    {
        int numargs = MAXWORDS, infix = 0;
        loopi(MAXWORDS)                         // collect all argument values
        {
            w[i] = parseword(p, i, infix);   // parse and evaluate exps
            if(!w[i]) { numargs = i; break; }
        }

        p += strcspn(p, ";\n\0");
        cont = *p++!=0;                         // more statements if this isn't the end of the string
        char *c = w[0];
        if(!c || !*c) continue;                       // empty statement

        DELETEA(retval);

        if(infix)
        {
            switch(infix)
            {
                case '=':
                    aliasa(c, numargs>2 ? w[2] : newstring(""));
                    w[2] = NULL;
                    break;
            }
        }
        else
        {
            ident *id = idents->access(c);
            if(!id)
            {
                if(!isinteger(c))
                    conoutf(CON_ERROR, "unknown command: %s", c);
                setretval(newstring(c));
            }
            else switch(id->type)
            {
                case ID_CCOMMAND:
                case ID_COMMAND:                     // game defined commands
                {
                    void *v[MAXWORDS];
                    union
                    {
                        int i;
                        float f;
                    } nstor[MAXWORDS];
                    int n = 0, wn = 0;
                    char *cargs = NULL;
                    if(id->type==ID_CCOMMAND) v[n++] = id->self;
                    for(const char *a = id->narg; *a; a++, n++) switch(*a)
                    {
                        case 's': v[n] = ++wn < numargs ? w[wn] : (char *)""; break;
                        case 'i': nstor[n].i = ++wn < numargs ? parseint(w[wn]) : 0;  v[n] = &nstor[n].i; break;
                        case 'f': nstor[n].f = ++wn < numargs ? parsefloat(w[wn]) : 0.0f; v[n] = &nstor[n].f; break;
#ifndef STANDALONE
                        case 'D': nstor[n].i = addreleaseaction(id->name) ? 1 : 0; v[n] = &nstor[n].i; break;
#endif
                        case 'V': v[n++] = w+1; nstor[n].i = numargs-1; v[n] = &nstor[n].i; break;
                        case 'C': if(!cargs) cargs = conc(w+1, numargs-1, true); v[n] = cargs; break;
                        default: fatal("builtin declared with illegal type");
                    }
                    switch(n)
                    {
                        case 0: ((void (__cdecl *)()                                      )id->fun)();                             break;
                        case 1: ((void (__cdecl *)(void *)                                )id->fun)(v[0]);                         break;
                        case 2: ((void (__cdecl *)(void *, void *)                        )id->fun)(v[0], v[1]);                   break;
                        case 3: ((void (__cdecl *)(void *, void *, void *)                )id->fun)(v[0], v[1], v[2]);             break;
                        case 4: ((void (__cdecl *)(void *, void *, void *, void *)        )id->fun)(v[0], v[1], v[2], v[3]);       break;
                        case 5: ((void (__cdecl *)(void *, void *, void *, void *, void *))id->fun)(v[0], v[1], v[2], v[3], v[4]); break;
                        case 6: ((void (__cdecl *)(void *, void *, void *, void *, void *, void *))id->fun)(v[0], v[1], v[2], v[3], v[4], v[5]); break;
                        case 7: ((void (__cdecl *)(void *, void *, void *, void *, void *, void *, void *))id->fun)(v[0], v[1], v[2], v[3], v[4], v[5], v[6]); break;
                        case 8: ((void (__cdecl *)(void *, void *, void *, void *, void *, void *, void *, void *))id->fun)(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]); break;
                        default: fatal("builtin declared with too many args (use V?)");
                    }
                    if(cargs) delete[] cargs;
                    setretval(commandret);
                    commandret = NULL;
                    break;
                }

                case ID_VAR:                        // game defined variables
                    if(numargs <= 1)
                    {
                        if(id->flags&IDF_HEX && id->maxval==0xFFFFFF)
                            conoutf("%s = 0x%.6X (%d, %d, %d)", c, *id->storage.i, (*id->storage.i>>16)&0xFF, (*id->storage.i>>8)&0xFF, *id->storage.i&0xFF);
                        else
                            conoutf(id->flags&IDF_HEX ? "%s = 0x%X" : "%s = %d", c, *id->storage.i);      // var with no value just prints its current value
                    }
                    else
                    {
                        int val = parseint(w[1]);
                        if(id->flags&IDF_HEX && numargs > 2)
                        {
                            val <<= 16;
                            val |= parseint(w[2])<<8;
                            if(numargs > 3) val |= parseint(w[3]);
                        }
                        setvarchecked(id, val);
                    }
                    break;

                case ID_FVAR:
                    if(numargs <= 1) conoutf("%s = %s", c, floatstr(*id->storage.f));
                    else setfvarchecked(id, parsefloat(w[1]));
                    break;

                case ID_SVAR:
                    if(numargs <= 1) conoutf(strchr(*id->storage.s, '"') ? "%s = [%s]" : "%s = \"%s\"", c, *id->storage.s);
                    else setsvarchecked(id, w[1]);
                    break;

                case ID_ALIAS:                              // alias, also used as functions and (global) variables
                {
                    delete[] w[0];
                    static vector<ident *> argids;
                    for(int i = 1; i<numargs; i++)
                    {
                        if(i > argids.length())
                        {
                            defformatstring(argname)("arg%d", i);
                            argids.add(newident(argname));
                        }
                        pushident(*argids[i-1], w[i]); // set any arguments as (global) arg values so functions can access them
                    }
                    _numargs = numargs-1;
                    bool wasoverriding = overrideidents;
                    if(id->override!=NO_OVERRIDE) overrideidents = true;
                    char *wasexecuting = id->isexecuting;
                    id->isexecuting = id->action;
                    setretval(executeret(id->action));
                    if(id->isexecuting != id->action && id->isexecuting != wasexecuting) delete[] id->isexecuting;
                    id->isexecuting = wasexecuting;
                    overrideidents = wasoverriding;
                    for(int i = 1; i<numargs; i++) popident(*argids[i-1]);
                    continue;
                }
            }
        }
        loopj(numargs) if(w[j]) delete[] w[j];
    }
    return retval;
}

int execute(const char *p)
{
    char *ret = executeret(p);
    int i = 0;
    if(ret) { i = parseint(ret); delete[] ret; }
    return i;
}

#ifndef STANDALONE
static int sortidents(ident **x, ident **y)
{
    return strcmp((*x)->name, (*y)->name);
}

void writeescapedstring(stream *f, const char *s)
{
    f->putchar('"');
    for(; *s; s++) switch(*s)
    {
        case '\n': f->write("^n", 2); break;
        case '\t': f->write("^t", 2); break;
        case '\f': f->write("^f", 2); break;
        case '"': f->write("^\"", 2); break;
        default: f->putchar(*s); break;
    }
    f->putchar('"');
}
#endif

// below the commands that implement a small imperative language. thanks to the semantics of
// () and [] expressions, any control construct can be defined trivially.

static string retbuf[3];
static int retidx = 0;

const char *intstr(int v)
{
    retidx = (retidx + 1)%3;
    formatstring(retbuf[retidx])("%d", v);
    return retbuf[retidx];
}

void intret(int v)
{
    commandret = newstring(intstr(v));
}

const char *floatstr(float v)
{
    retidx = (retidx + 1)%3;
    formatstring(retbuf[retidx])(v==int(v) ? "%.1f" : "%.7g", v);
    return retbuf[retidx];
}

void floatret(float v)
{
    commandret = newstring(floatstr(v));
}

#undef ICOMMANDNAME
#define ICOMMANDNAME(name) _stdcmd

ICOMMAND(if, "sss", (char *cond, char *t, char *f), commandret = executeret(cond[0] && (!isinteger(cond) || parseint(cond)) ? t : f));
ICOMMAND(?, "sss", (char *cond, char *t, char *f), result(cond[0] && (!isinteger(cond) || parseint(cond)) ? t : f));
ICOMMAND(loop, "sis", (char *var, int *n, char *body),
{
    if(*n<=0) return;
    ident *id = newident(var);
    if(id->type!=ID_ALIAS) return;
    loopi(*n)
    {
        if(i) sprintf(id->action, "%d", i);
        else pushident(*id, newstring("0", 16));
        execute(body);
    }
    popident(*id);
});
ICOMMAND(loopwhile, "siss", (char *var, int *n, char *cond, char *body),
{
    if(*n<=0) return;
    ident *id = newident(var);
    if(id->type!=ID_ALIAS) return;
    loopi(*n)
    {
        if(i) sprintf(id->action, "%d", i);
        else pushident(*id, newstring("0", 16));
        if(!execute(cond)) break;
        execute(body);
    }
    popident(*id);
});
ICOMMAND(while, "ss", (char *cond, char *body), while(execute(cond)) execute(body));    // can't get any simpler than this :)

void concat(const char *s) { commandret = newstring(s); }
void result(const char *s) { commandret = newstring(s); }

void concatword(char **args, int *numargs)
{
    commandret = conc(args, *numargs, false);
}

void format(char **args, int *numargs)
{
    vector<char> s;
    char *f = args[0];
    while(*f)
    {
        int c = *f++;
        if(c == '%')
        {
            int i = *f++;
            if(i >= '1' && i <= '9')
            {
                i -= '0';
                const char *sub = i < *numargs ? args[i] : "";
                while(*sub) s.add(*sub++);
            }
            else s.add(i);
        }
        else s.add(c);
    }
    s.add('\0');
    result(s.getbuf());
}

#define whitespaceskip s += strspn(s, "\n\t ")
#define elementskip *s=='"' ? (++s, s += strcspn(s, "\"\n\0"), s += *s=='"') : s += strcspn(s, "\n\t \0")

void explodelist(const char *s, vector<char *> &elems)
{
    whitespaceskip;
    while(*s)
    {
        const char *elem = s;
        elementskip;
        elems.add(*elem=='"' ? newstring(elem+1, s-elem-(s[-1]=='"' ? 2 : 1)) : newstring(elem, s-elem));
        whitespaceskip;
    }
}

char *indexlist(const char *s, int pos)
{
    whitespaceskip;
    loopi(pos)
    {
        elementskip;
        whitespaceskip;
        if(!*s) break;
    }
    const char *e = s;
    elementskip;
    if(*e=='"')
    {
        e++;
        if(s[-1]=='"') --s;
    }
    return newstring(e, s-e);
}

int listlen(const char *s)
{
    int n = 0;
    whitespaceskip;
    for(; *s; n++) elementskip, whitespaceskip;
    return n;
}

void at(char *s, int *pos)
{
    commandret = indexlist(s, *pos);
}

void substr(char *s, int *start, char *count)
{
    int len = strlen(s), offset = clamp(*start, 0, len);
    commandret = newstring(&s[offset], count[0] ? clamp(parseint(count), 0, len - offset) : len - offset);
}

void getalias_(char *s)
{
    result(getalias(s));
}

COMMAND(concat, "C");
COMMAND(result, "s");
COMMAND(concatword, "V");
COMMAND(format, "V");
COMMAND(at, "si");
COMMAND(substr, "sis");
ICOMMAND(listlen, "s", (char *s), intret(listlen(s)));
COMMANDN(getalias, getalias_, "s");

void looplist(const char *var, const char *list, const char *body, bool search)
{
    ident *id = newident(var);
    if(id->type!=ID_ALIAS) { if(search) intret(-1); return; }
    int n = 0;
    for(const char *s = list;;)
    {
        whitespaceskip;
        if(!*s) { if(search) intret(-1); break; }
        const char *start = s;
        elementskip;
        const char *end = s;
        if(*start=='"') { start++; if(end[-1]=='"') --end; }
        char *val = newstring(start, end-start);
        if(n++) aliasa(id->name, val);
        else pushident(*id, val);
        if(execute(body) && search) { intret(n-1); break; }
    }
    if(n) popident(*id);
}

void prettylist(const char *s, const char *conj)
{
    vector<char> p;
    whitespaceskip;
    for(int len = listlen(s), n = 0; *s; n++)
    {
        const char *elem = s;
        elementskip;
        p.put(elem, s - elem);
        if(n+1 < len)
        {
            if(len > 2 || !conj[0]) p.add(',');
            if(n+2 == len && conj[0])
            {
                p.add(' ');
                p.put(conj, strlen(conj));
            }
            p.add(' ');
        }
        whitespaceskip;
    }
    p.add('\0');
    result(p.getbuf());
}
COMMAND(prettylist, "ss");

int listincludes(const char *list, const char *needle, int needlelen)
{
    const char *s = list;
    whitespaceskip;
    int offset = 0;
    while(*s)
    {
        const char *elem = s;
        elementskip;
        int len = s-elem;
        if(*elem=='"')
        {
            elem++;
            len -= s[-1]=='"' ? 2 : 1;
        }
        if(needlelen == len && !strncmp(needle, elem, len)) return offset;
        whitespaceskip;
        offset++;
    }
    return -1;
}
    
char *listdel(const char *s, const char *del)
{
    vector<char> p;
    whitespaceskip;
    while(*s)
    {
        const char *elem = s;
        elementskip;
        int len = s-elem;
        if(*elem=='"')
        {
            elem++;
            len -= s[-1]=='"' ? 2 : 1;
        }
        if(listincludes(del, elem, len) < 0)
        {
            if(!p.empty()) p.add(' ');
            p.put(elem, len);
        }
        whitespaceskip;
    }
    p.add('\0');
    return newstring(p.getbuf());
}

ICOMMAND(listdel, "ss", (char *list, char *del), commandret = listdel(list, del));
ICOMMAND(indexof, "ss", (char *list, char *elem), intret(listincludes(list, elem, strlen(elem))));
ICOMMAND(listfind, "sss", (char *var, char *list, char *body), looplist(var, list, body, true));
ICOMMAND(looplist, "sss", (char *var, char *list, char *body), looplist(var, list, body, false));

ICOMMAND(+, "ii", (int *a, int *b), intret(*a + *b));
ICOMMAND(*, "ii", (int *a, int *b), intret(*a * *b));
ICOMMAND(-, "ii", (int *a, int *b), intret(*a - *b));
ICOMMAND(+f, "ff", (float *a, float *b), floatret(*a + *b));
ICOMMAND(*f, "ff", (float *a, float *b), floatret(*a * *b));
ICOMMAND(-f, "ff", (float *a, float *b), floatret(*a - *b));
ICOMMAND(=, "ii", (int *a, int *b), intret((int)(*a == *b)));
ICOMMAND(!=, "ii", (int *a, int *b), intret((int)(*a != *b)));
ICOMMAND(<, "ii", (int *a, int *b), intret((int)(*a < *b)));
ICOMMAND(>, "ii", (int *a, int *b), intret((int)(*a > *b)));
ICOMMAND(<=, "ii", (int *a, int *b), intret((int)(*a <= *b)));
ICOMMAND(>=, "ii", (int *a, int *b), intret((int)(*a >= *b)));
ICOMMAND(=f, "ff", (float *a, float *b), intret((int)(*a == *b)));
ICOMMAND(!=f, "ff", (float *a, float *b), intret((int)(*a != *b)));
ICOMMAND(<f, "ff", (float *a, float *b), intret((int)(*a < *b)));
ICOMMAND(>f, "ff", (float *a, float *b), intret((int)(*a > *b)));
ICOMMAND(<=f, "ff", (float *a, float *b), intret((int)(*a <= *b)));
ICOMMAND(>=f, "ff", (float *a, float *b), intret((int)(*a >= *b)));
ICOMMAND(^, "ii", (int *a, int *b), intret(*a ^ *b));
ICOMMAND(!, "i", (int *a), intret(*a == 0));
ICOMMAND(&, "ii", (int *a, int *b), intret(*a & *b));
ICOMMAND(|, "ii", (int *a, int *b), intret(*a | *b));
ICOMMAND(~, "i", (int *a), intret(~*a));
ICOMMAND(^~, "ii", (int *a, int *b), intret(*a ^ ~*b));
ICOMMAND(&~, "ii", (int *a, int *b), intret(*a & ~*b));
ICOMMAND(|~, "ii", (int *a, int *b), intret(*a | ~*b));
ICOMMAND(<<, "ii", (int *a, int *b), intret(*a << *b));
ICOMMAND(>>, "ii", (int *a, int *b), intret(*a >> *b));
ICOMMAND(&&, "V", (char **args, int *numargs),
{
    int val = 1;
    loopi(*numargs) { val = execute(args[i]); if(!val) break; }
    intret(val);
});
ICOMMAND(||, "V", (char **args, int *numargs),
{
    int val = 0;
    loopi(*numargs) { val = execute(args[i]); if(val) break; }
    intret(val);
});

ICOMMAND(div, "ii", (int *a, int *b), intret(*b ? *a / *b : 0));
ICOMMAND(mod, "ii", (int *a, int *b), intret(*b ? *a % *b : 0));
ICOMMAND(divf, "ff", (float *a, float *b), floatret(*b ? *a / *b : 0));
ICOMMAND(modf, "ff", (float *a, float *b), floatret(*b ? fmod(*a, *b) : 0));
ICOMMAND(sin, "f", (float *a), floatret(sin(*a*RAD)));
ICOMMAND(cos, "f", (float *a), floatret(cos(*a*RAD)));
ICOMMAND(tan, "f", (float *a), floatret(tan(*a*RAD)));
ICOMMAND(asin, "f", (float *a), floatret(asin(*a)/RAD));
ICOMMAND(acos, "f", (float *a), floatret(acos(*a)/RAD));
ICOMMAND(atan, "f", (float *a), floatret(atan(*a)/RAD));
ICOMMAND(sqrt, "f", (float *a), floatret(sqrt(*a)));
ICOMMAND(pow, "ff", (float *a, float *b), floatret(pow(*a, *b)));
ICOMMAND(loge, "f", (float *a), floatret(log(*a)));
ICOMMAND(log2, "f", (float *a), floatret(log(*a)/M_LN2));
ICOMMAND(log10, "f", (float *a), floatret(log10(*a)));
ICOMMAND(exp, "f", (float *a), floatret(exp(*a)));
ICOMMAND(min, "V", (char **args, int *numargs),
{
    int val = *numargs > 0 ? parseint(args[*numargs - 1]) : 0;
    loopi(*numargs - 1) val = min(val, parseint(args[i]));
    intret(val);
});
ICOMMAND(max, "V", (char **args, int *numargs),
{
    int val = *numargs > 0 ? parseint(args[*numargs - 1]) : 0;
    loopi(*numargs - 1) val = max(val, parseint(args[i]));
    intret(val);
});
ICOMMAND(minf, "V", (char **args, int *numargs),
{
    float val = *numargs > 0 ? parsefloat(args[*numargs - 1]) : 0.0f;
    loopi(*numargs - 1) val = min(val, parsefloat(args[i]));
    floatret(val);
});
ICOMMAND(maxf, "V", (char **args, int *numargs),
{
    float val = *numargs > 0 ? parsefloat(args[*numargs - 1]) : 0.0f;
    loopi(*numargs - 1) val = max(val, parsefloat(args[i]));
    floatret(val);
});

ICOMMAND(cond, "V", (char **args, int *numargs),
{
    for(int i = 0; i < *numargs; i += 2)
    {
        if(execute(args[i]))
        {
            if(i+1 < *numargs) commandret = executeret(args[i+1]);
            break;
        }
    }
});
#define CASECOMMAND(name, fmt, type, compare) \
    ICOMMAND(name, fmt "V", (type *val, char **args, int *numargs), \
    { \
        int i; \
        for(i = 1; i+1 < *numargs; i += 2) \
        { \
            if(compare) \
            { \
                commandret = executeret(args[i+1]); \
                return; \
            } \
        } \
        if(i < *numargs) commandret = executeret(args[i]); \
    })
CASECOMMAND(case, "i", int, parseint(args[i]) == *val);
CASECOMMAND(casef, "f", float, parsefloat(args[i]) == *val);
CASECOMMAND(cases, "s", char, !strcmp(args[i], val));

ICOMMAND(rnd, "ii", (int *a, int *b), intret(*a - *b > 0 ? rnd(*a - *b) + *b : *b));
ICOMMAND(strcmp, "ss", (char *a, char *b), intret(strcmp(a,b)==0));
ICOMMAND(=s, "ss", (char *a, char *b), intret(strcmp(a,b)==0));
ICOMMAND(!=s, "ss", (char *a, char *b), intret(strcmp(a,b)!=0));
ICOMMAND(<s, "ss", (char *a, char *b), intret(strcmp(a,b)<0));
ICOMMAND(>s, "ss", (char *a, char *b), intret(strcmp(a,b)>0));
ICOMMAND(<=s, "ss", (char *a, char *b), intret(strcmp(a,b)<=0));
ICOMMAND(>=s, "ss", (char *a, char *b), intret(strcmp(a,b)>=0));
ICOMMAND(echo, "C", (char *s), conoutf("%s", s));
ICOMMAND(error, "C", (char *s), conoutf(CON_ERROR, s));
ICOMMAND(strstr, "ss", (char *a, char *b), { char *s = strstr(a, b); intret(s ? s-a : -1); });
ICOMMAND(strlen, "s", (char *s), intret(strlen(s)));

char *strreplace(const char *s, const char *oldval, const char *newval)
{
    vector<char> buf;

    int oldlen = strlen(oldval);
    if(!oldlen) return newstring(s);
    for(;;)
    {
        const char *found = strstr(s, oldval);
        if(found)
        {
            while(s < found) buf.add(*s++);
            for(const char *n = newval; *n; n++) buf.add(*n);
            s = found + oldlen;
        }
        else
        {
            while(*s) buf.add(*s++);
            buf.add('\0');
            return newstring(buf.getbuf(), buf.length());
        }
    }
}

ICOMMAND(strreplace, "sss", (char *s, char *o, char *n), commandret = strreplace(s, o, n));

#ifndef STANDALONE
ICOMMAND(getmillis, "i", (int *total), intret(*total ? totalmillis : lastmillis));

struct sleepcmd
{
    int delay, millis;
    char *command;
    bool override, persist;
};
vector<sleepcmd> sleepcmds;

void addsleep(int *msec, char *cmd)
{
    sleepcmd &s = sleepcmds.add();
    s.delay = max(*msec, 1);
    s.millis = lastmillis;
    s.command = newstring(cmd);
    s.override = overrideidents;
    s.persist = persistidents;
}

COMMANDN(sleep, addsleep, "is");

void checksleep(int millis)
{
    loopv(sleepcmds)
    {
        sleepcmd &s = sleepcmds[i];
        if(millis - s.millis >= s.delay)
        {
            char *cmd = s.command; // execute might create more sleep commands
            s.command = NULL;
            bool waspersisting = persistidents, wasoverriding = overrideidents;
            persistidents = s.persist;
            overrideidents = s.override;
            execute(cmd);
            persistidents = waspersisting;
            overrideidents = wasoverriding;
            delete[] cmd;
            if(sleepcmds.inrange(i) && !sleepcmds[i].command) sleepcmds.remove(i--);
        }
    }
}

void clearsleep(bool clearoverrides)
{
    int len = 0;
    loopv(sleepcmds) if(sleepcmds[i].command)
    {
        if(clearoverrides && !sleepcmds[i].override) sleepcmds[len++] = sleepcmds[i];
        else delete[] sleepcmds[i].command;
    }
    sleepcmds.shrink(len);
}

void clearsleep_(int *clearoverrides)
{
    clearsleep(*clearoverrides!=0 || overrideidents);
}

COMMANDN(clearsleep, clearsleep_, "i");
#endif


// XXX Emscripten: console.cpp

struct cline { char *line; int type, outtime; };
vector<cline> conlines;

int commandmillis = -1;
string commandbuf;
char *commandaction = NULL, *commandprompt = NULL;
int commandpos = -1;
int totalmillis = -1; // XXX Emscripten

VARFP(maxcon, 10, 200, 1000, { while(conlines.length() > maxcon) delete[] conlines.pop().line; });

#define CONSTRLEN 512

void conline(int type, const char *sf)        // add a line to the console buffer
{
    cline cl;
    cl.line = conlines.length()>maxcon ? conlines.pop().line : newstring("", CONSTRLEN-1);   // constrain the buffer size
    cl.type = type;
    cl.outtime = totalmillis;                       // for how long to keep line on screen
    conlines.insert(0, cl);
    copystring(cl.line, sf, CONSTRLEN);
}

void conoutfv(int type, const char *fmt, va_list args)
{
    static char buf[CONSTRLEN];
    vformatstring(buf, fmt, args, sizeof(char)*CONSTRLEN);
    conline(type, buf);
    //filtertext(buf, buf); // XXX Emscripten
    puts(buf);
}

void conoutf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    conoutfv(CON_INFO, fmt, args);
    va_end(args); 
}

void conoutf(int type, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    conoutfv(type, fmt, args);
    va_end(args);
}
// XXX =======================================

// XXX Emscripten: tools.cpp

#define N              (624)             
#define M              (397)                
#define K              (0x9908B0DFU)       
#define hiBit(u)       ((u) & 0x80000000U)  
#define loBit(u)       ((u) & 0x00000001U)  
#define loBits(u)      ((u) & 0x7FFFFFFFU)  
#define mixBits(u, v)  (hiBit(u)|loBits(v)) 

static uint state[N+1];     
static uint *next;          
static int left = -1;     

void seedMT(uint seed)
{
    register uint x = (seed | 1U) & 0xFFFFFFFFU, *s = state;
    register int j;
    for(left=0, *s++=x, j=N; --j; *s++ = (x*=69069U) & 0xFFFFFFFFU);
}

uint reloadMT(void)
{
    register uint *p0=state, *p2=state+2, *pM=state+M, s0, s1;
    register int j;
    if(left < -1) seedMT(time(NULL));
    left=N-1, next=state+1;
    for(s0=state[0], s1=state[1], j=N-M+1; --j; s0=s1, s1=*p2++) *p0++ = *pM++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);
    for(pM=state, j=M; --j; s0=s1, s1=*p2++) *p0++ = *pM++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);
    s1=state[0], *p0 = *pM ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);
    s1 ^= (s1 >> 11);
    s1 ^= (s1 <<  7) & 0x9D2C5680U;
    s1 ^= (s1 << 15) & 0xEFC60000U;
    return(s1 ^ (s1 >> 18));
}

uint randomMT(void)
{
    uint y;
    if(--left < 0) return(reloadMT());
    y  = *next++;
    y ^= (y >> 11);
    y ^= (y <<  7) & 0x9D2C5680U;
    y ^= (y << 15) & 0xEFC60000U;
    return(y ^ (y >> 18));
}

// XXX ==============================================

// XXX Emscripten: main.cpp

void fatal(const char *s, ...)    // failure exit
{
    static int errors = 0;
    errors++;

    if(errors <= 2) // print up to one extra recursive error
    {
        defvformatstring(msg,s,s);
        puts(msg);
    }

    exit(EXIT_FAILURE);
}

VARP(somevar, 0, 0, 1024);

int main()
{
  printf("*\n");
  execute("somevar 9");
  execute("temp = (+ 22 $somevar)");
  execute("if (> $temp 30) [ temp = (+ $temp 1) ] [ temp = (* $temp 2) ]");
  execute("if (< $temp 30) [ temp = 0 ] [ temp = (+ $temp 1) ]");
  execute("echo [Temp is] $temp");
  printf("%d\n", getvar("somevar"));
  execute("x = 2");
  execute("push x 5");
  execute("push x 11");
  execute("pop x");
  execute("echo $x");
  execute("greet = [ echo hello, $arg1 ]");
  execute("greet everyone");
  printf("*\n");
  return 0;
}

// XXX ===============================
