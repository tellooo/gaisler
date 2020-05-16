#ifndef _FLAT
 #define INT_HANDLER            __bcc_trap_interrupt_svt_tn0018
#else
 #define INT_HANDLER            __bcc_trap_interrupt_flat_svt_tn0018
#endif

	.section	".rodata"
	.align 4
	.global		__bcc_trap_table_svt_level0_tn0018
	.global		__bcc_trap_table_svt_1_tn0018

__bcc_trap_table_svt_1_tn0018:
	.long __bcc_trap_table_svt_bad  ! 10 reserved
	.long INT_HANDLER               ! 11 interrupt_level_1
	.long INT_HANDLER               ! 12 interrupt_level_2
	.long INT_HANDLER               ! 13 interrupt_level_3
	.long INT_HANDLER               ! 14 interrupt_level_4
	.long INT_HANDLER               ! 15 interrupt_level_5
	.long INT_HANDLER               ! 16 interrupt_level_6
	.long INT_HANDLER               ! 17 interrupt_level_7
	.long INT_HANDLER               ! 18 interrupt_level_8
	.long INT_HANDLER               ! 19 interrupt_level_9
	.long INT_HANDLER               ! 1A interrupt_level_1
	.long INT_HANDLER               ! 1B interrupt_level_11
	.long INT_HANDLER               ! 1C interrupt_level_12
	.long INT_HANDLER               ! 1D interrupt_level_13
	.long INT_HANDLER               ! 1E interrupt_level_14
	.long INT_HANDLER               ! 1F interrupt_level_15

	/* Declare subtable symbols as weak so they can be replaced when linking */
	.weak __bcc_trap_table_svt_1_tn0018

__bcc_trap_table_svt_level0_tn0018:
	.long __bcc_trap_table_svt_0
	.long __bcc_trap_table_svt_1_tn0018
	.long __bcc_trap_table_svt_2
	.long __bcc_trap_table_svt_3
	.long __bcc_trap_table_svt_4
	.long __bcc_trap_table_svt_5
	.long __bcc_trap_table_svt_6
	.long __bcc_trap_table_svt_7
	.long __bcc_trap_table_svt_8
	.long __bcc_trap_table_svt_9
	.long __bcc_trap_table_svt_a
	.long __bcc_trap_table_svt_b
	.long __bcc_trap_table_svt_c
	.long __bcc_trap_table_svt_d
	.long __bcc_trap_table_svt_e
	.long __bcc_trap_table_svt_f

