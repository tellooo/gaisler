/* Target OS builtins.  */
#undef TARGET_SUB_OS_CPP_BUILTINS
#define TARGET_SUB_OS_CPP_BUILTINS()		\
  do						\
    {						\
	builtin_define_std ("__leonbare__");	\
	builtin_define_std ("__threadx__");	\
    }						\
  while (0)

#define LIB_SPEC_LEONMORE \
    " -lthreadx "
