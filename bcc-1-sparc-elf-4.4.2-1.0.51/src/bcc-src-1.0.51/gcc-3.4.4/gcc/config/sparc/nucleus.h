/* Target OS builtins.  */
#undef TARGET_SUB_OS_CPP_BUILTINS
#define TARGET_SUB_OS_CPP_BUILTINS()		\
  do						\
    {						\
	builtin_define_std ("__nucleus__");	\
    }						\
  while (0)

#define LIB_SPEC_KEEP
#undef LIB_SPEC
#define LIB_SPEC \
"%{!nostdlib: --start-group -lgcc -lplus -lesal -lhw_serial -lesal_board --end-group} "

#define STARTFILE_SPEC_KEEP
#undef STARTFILE_SPEC
#define STARTFILE_SPEC \
"%{!qsvt: --whole-archive -lesal_board -no-whole-archive } %{qsvt:  --whole-archive -lesal_board_svt -no-whole-archive } "
