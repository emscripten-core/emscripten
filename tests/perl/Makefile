LD = $(CC)
CCFLAGS = -c -O2
MINIPERLMAIN = miniperlmain.c

DEFINES = -DPERL_CORE -DPERL_MICRO -DSTANDARD_C -DPERL_USE_SAFE_PUTENV \
	  -DNO_MATHOMS -DPURIFY
OPTIMIZE =
CFLAGS = $(DEFINES) $(OPTIMIZE)
LDFLAGS = -O2
LIBS = -lm
_O = .o
ENV = env
PERL = perl
_X =
RUN =

all:	plu

O = uav$(_O) udeb$(_O) udoio$(_O) udoop$(_O) udump$(_O) \
	uglobals$(_O) ugv$(_O) uhv$(_O) umro$(_O)\
	umg$(_O) uperlmain$(_O) uop$(_O) ureentr$(_O) \
	upad$(_O) uperl$(_O) uperlio$(_O) uperly$(_O) upp$(_O) \
	upp_ctl$(_O) upp_hot$(_O) upp_sys$(_O) upp_pack$(_O) upp_sort$(_O) \
	uregcomp$(_O) uregexec$(_O) urun$(_O) \
	uscope$(_O) usv$(_O) utaint$(_O) utoke$(_O) \
	unumeric$(_O) ulocale$(_O) umathoms$(_O) \
	uuniversal$(_O) uutf8$(_O) uutil$(_O) uperlapi$(_O) ukeywords$(_O)

plu: $(O)
	$(LD) -o $@ $(O) $(LDFLAGS) $(LIBS)

generated_headers = uuudmap.h ubitcount.h umg_data.h

H = av.h uconfig.h cop.h cv.h embed.h embedvar.h form.h gv.h handy.h \
	hv.h intrpvar.h iperlsys.h mg.h op.h opcode.h opnames.h pad.h \
	patchlevel.h perl.h perlsdio.h perlvars.h perly.h pp.h \
	pp_proto.h proto.h reentr.h regexp.h scope.h sv.h \
	thread.h unixish.h utf8.h util.h warnings.h $(generated_headers)

HE = $(H) EXTERN.h

clean:
	-rm -f $(O) ugenerate_uudmap$(_X) $(generated_headers)

distclean:	clean

# The microconfiguration.

# Cannot use $$ in the command line itself, so using var expansion instead.
Config = '$$Config{$$1}'
patch_uconfig:
	$(PERL) -MConfig -pi -e "s/^((?:short|int|long(?:dbl|long)?|ptr|double|[iun]v|u?quad|[iu]\d+|fpos|lseek)(?:size|type)|byteorder|d_quad|quadkind|use64.+)=.*/\\1='"$(Config)"'/g" uconfig.sh

regen_uconfig: uconfig.sh config_h.SH
	$(ENV) CONFIG_SH=uconfig.sh CONFIG_H=uconfig.h sh ./config_h.SH

regen_uconfig64: uconfig64.sh config_h.SH
	$(ENV) CONFIG_SH=uconfig64.sh CONFIG_H=uconfig.h sh ./config_h.SH

# Do not regenerate perly.c and perly.h.

perly.c: perly.y
	-@echo perly.c is uptodate

perly.h: perly.y
	-@echo perly.h is uptodate

uav$(_O):	$(HE) av.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) av.c

udeb$(_O):	$(HE) deb.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) deb.c

udoio$(_O):	$(HE) doio.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) doio.c

udoop$(_O):	$(HE) doop.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) doop.c

udump$(_O):	$(HE) dump.c regcomp.h regnodes.h
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) dump.c

uglobals$(_O):	$(H) globals.c INTERN.h perlapi.h $(generated_headers)
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) globals.c

ugv$(_O):	$(HE) gv.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) gv.c

umro$(_O):	$(HE) mro.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) mro.c

uhv$(_O):	$(HE) hv.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) hv.c

ukeywords$(_O):	$(HE) keywords.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) keywords.c

umg$(_O):	$(HE) mg.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) mg.c

uperlmain$(_O):	$(HE) $(MINIPERLMAIN)
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) $(MINIPERLMAIN)

uop$(_O):	$(HE) op.c keywords.h
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) -DPERL_EXTERNAL_GLOB op.c

ureentr$(_O):	$(HE) reentr.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) reentr.c

upad$(_O):	$(HE) pad.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) pad.c

uperl$(_O):	$(HE) perl.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) perl.c

uperlio$(_O):	$(HE) perlio.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) perlio.c

uperly$(_O):	$(HE) perly.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) perly.c

upp$(_O):	$(HE) pp.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) pp.c

upp_ctl$(_O):	$(HE) pp_ctl.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) pp_ctl.c

upp_hot$(_O):	$(HE) pp_hot.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) pp_hot.c

upp_sys$(_O):	$(HE) pp_sys.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) pp_sys.c

upp_pack$(_O):	$(HE) pp_pack.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) pp_pack.c

upp_sort$(_O):	$(HE) pp_sort.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) pp_sort.c

uregcomp$(_O):	$(HE) regcomp.c regcomp.h regnodes.h INTERN.h
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) regcomp.c

uregexec$(_O):	$(HE) regexec.c regcomp.h regnodes.h
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) regexec.c

urun$(_O):	$(HE) run.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) run.c

uscope$(_O):	$(HE) scope.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) scope.c

usv$(_O):	$(HE) sv.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) sv.c

utaint$(_O):	$(HE) taint.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) taint.c

utoke$(_O):	$(HE) toke.c keywords.h
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) toke.c

ulocale$(_O):	$(HE) locale.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) locale.c

unumeric$(_O):	$(HE) numeric.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) numeric.c

umathoms$(_O):	$(HE) mathoms.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) mathoms.c

uuniversal$(_O):	$(HE) universal.c XSUB.h
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) universal.c

uutf8$(_O):	$(HE) utf8.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) utf8.c

uutil$(_O):	$(HE) util.c
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) util.c

uperlapi$(_O):	$(HE) perlapi.c perlapi.h
	$(CC) $(CCFLAGS) -o $@ $(CFLAGS) perlapi.c

uuudmap.h umg_data.h: ubitcount.h

ubitcount.h: ugenerate_uudmap$(_X)
	./ugenerate_uudmap$(_X) $(generated_headers)

ugenerate_uudmap: generate_uudmap.c
	gcc -o ugenerate_uudmap $(CFLAGS) -lm generate_uudmap.c

plutest: plu
	- cd t && (rm -f perl; ln -s ../plu perl) \
	  && ./perl -I../lib TEST base/*.t cmd/*.t

# That's it, folks!
