/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include <asm-leon/leon.h>
#include <asm-leon/irq.h>
#include <asm-leon/time.h>

#ifndef configKERNEL_INTERRUPT_PRIORITY
	#define configKERNEL_INTERRUPT_PRIORITY 255
#endif

/* The priority used by the kernel is assigned to a variable to make access
from inline assembler easier. */
const unsigned long ulKernelPriority = configKERNEL_INTERRUPT_PRIORITY;

int  xPortScheduleHandler(struct leonbare_pt_regs *regs);

/* Each task maintains its own interrupt status in the critical nesting variable. */
static unsigned portBASE_TYPE uxCriticalNesting = 0xaaaaaaaa;

/* Exception handlers. */
void xPortSysTickHandler( void );
void vPortSVCHandler( void );
/*
 * See header file for description.
 */
portSTACK_TYPE *pxPortInitialiseStack( portSTACK_TYPE *pxTopOfStack, pdTASK_CODE pxCode, void *pvParameters )
{
	unsigned int stk; struct freertos_stack *a;
	
	stk = (((unsigned int) pxTopOfStack) - ( FREERTOS_STACK_SIZE + SF_REGS_SZ + (SF_REGS_SZ * 2)) & ~7);
	
	a = (struct freertos_stack *)((unsigned int)stk + SF_REGS_SZ);
	a->psr = freeos_getpsr() & ~SPARC_PSR_PIL_MASK;
	a->pc = ((unsigned int)pxCode)-8;
	a->o0 = (unsigned int)pvParameters;
	a->regs[8+6] = stk + SF_REGS_SZ; /* fp */
	
	return (portSTACK_TYPE *)stk;
}
	
typedef void tskTCB;
extern volatile tskTCB * volatile pxCurrentTCB;
extern int nestedirq;

/*
 * See header file for description.
 */
portBASE_TYPE xPortStartScheduler( void )
{

	no_inirq_check = 1;
	nestedirq = 0;
	
	portDISABLE_INTERRUPTS();
	
	leonbare_init_ticks();
	ticker_callback = (tickerhandler) xPortSysTickHandler;
#if configUSE_PREEMPTION == 1
	schedule_callback = (schedulehandler) xPortScheduleHandler;
#endif
	
	/* Initialise the critical nesting count ready for the first task. */
	uxCriticalNesting = 0;

	portENABLE_INTERRUPTS();
	
	/* Start the first task. */
	FreeRtosSwitchTo(pxCurrentTCB);

	/* Should not get here! */
	return 0;
}

void vPortEndScheduler( void )
{
	/* It is unlikely that the CM3 port will require this function as there
	is nothing to return to.  */
}

void vPortEnterCritical( void )
{
	portDISABLE_INTERRUPTS();
	uxCriticalNesting++;
}
/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
	uxCriticalNesting--;
	if( uxCriticalNesting == 0 )
	{
		portENABLE_INTERRUPTS();
	}
}

void xPortSysTickHandler( void )
{
	unsigned long ulDummy, psr;
	
	psr = freeos_getpsr();
	if ((psr & SPARC_PSR_PIL_MASK) != SPARC_PSR_PIL_MASK){
		freeos_error();
	}
	ulDummy = portSET_INTERRUPT_MASK_FROM_ISR();
	{
		vTaskIncrementTick();
	}
	portCLEAR_INTERRUPT_MASK_FROM_ISR( ulDummy );
	
#if configUSE_PREEMPTION == 1
	vPortYieldFromISR();
#endif
}

int  xPortScheduleHandler(struct leonbare_pt_regs *regs) {
	return 0;
}

__attribute__((weak)) void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName ) {
	printf("Fail vApplicationStackOverflowHook\n");
	for( ;; );
}

__attribute__((weak)) void vApplicationTickHook( void ) {
	
}

__attribute__((weak)) void vApplicationMallocFailedHook( void ) {
	printf("Fail vApplicationMallocFailedHook\n");
	for( ;; );
}
