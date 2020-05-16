/* Target OS builtins.  */
#undef TARGET_SUB_OS_CPP_BUILTINS
#define TARGET_SUB_OS_CPP_BUILTINS()		\
  do						\
    {						\
	builtin_define_std ("__lynxos__");	\
	builtin_define_std ("__BIG_ENDIAN__");	\
    }						\
  while (0)

/*	builtin_define_std ("__LYNXOS");	\*/

#define LIB_SPEC_LEONMORE \
    " -llynxos "


#undef STARTFILE_SPEC
#define STARTFILE_SPEC \
"%{!shared:    %{!mthreads:      %{p:gcrt1.o%s} %{pg:gcrt1.o%s}      %{!p:%{!pg:crt1.o%s}}}    %{mthreads:      %{p:thread/gcrt1.o%s} %{pg:thread/gcrt1.o%s}      %{!p:%{!pg:thread/crt1.o%s }}}} %{mthreads: thread/crti.o%s} %{!mthreads: crti.o%s}  %{!shared: crtbegin.o%s}  %{shared: crtbeginS.o%s}"

#undef  ENDFILE_SPEC
#define ENDFILE_SPEC \
"%{!shared: crtend.o%s}  %{shared: crtendS.o%s}  %{mthreads: thread/crtn.o%s} %{!mthreads: crtn.o%s}"

#undef LINK_SPEC
#define LINK_SPEC \
"%{shared} %{static}  %{mshared: %{static: %eCannot use mshared and static together.}}  %{!mshared: %{!shared: %{!static: -static}}} \
  %{mshared: -rpath /usr/lib/sparc-lynxos-prefix}  %{shared: %{!static: -rpath /usr/lib/sparc-lynxos-prefix}}  %{L*}  %{mthreads:    %{mshared: %:getenv(ENV_PREFIX -L /lib/thread/shlib) \
 -rpath /lib/thread/shlib}    %{shared: %{!static: %:getenv(ENV_PREFIX -L /lib/thread/shlib) -rpath /lib/thread/shlib}}    %{static: %:getenv(ENV_PREFIX -L /lib/thread)} \
   %{!shared: %{!mshared: %:getenv(ENV_PREFIX -L /lib/thread)}}}  %{!mthreads:    %{mshared: %:getenv(ENV_PREFIX -L /lib/shlib) -rpath /lib/shlib} \
   %{shared: %{!static: %:getenv(ENV_PREFIX -L /lib/shlib) -rpath /lib/shlib}}    %{static: %:getenv(ENV_PREFIX -L /lib)}    %{!shared: %{!mshared: %:getenv(ENV_PREFIX -L /lib)}}}  %:getenv(ENV_PREFIX -L /usr/lib) "
