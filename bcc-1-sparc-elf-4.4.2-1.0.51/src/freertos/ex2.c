#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define rx_TASK_STACK_SIZE		( configMINIMAL_STACK_SIZE )
#define tx_TASK_STACK_SIZE		( configMINIMAL_STACK_SIZE )
#define mainCREATOR_TASK_PRIORITY       ( tskIDLE_PRIORITY + 3 )
#define intqQUEUE_LENGTH		( ( unsigned portBASE_TYPE ) 16 )

typedef int rxToken;
xQueueHandle xRxQueue = 0;

void vRxTask( void *pvParameters ) {
	while (1) {
		portBASE_TYPE xQueueStatus;
		rxToken tok;
		if(xRxQueue && ( xQueueStatus = xQueueReceive( xRxQueue, &tok, portMAX_DELAY ) ) == pdPASS ) {
			printf("rx 0x%x\n", freeos_getsp());
		}
	}
}

void vTxTask( void *pvParameters ) {
	while (1) {
		printf("tx 0x%x\n",freeos_getsp());
		vTaskDelay( 10 );
	}
}

int timer_tick=0;
int timer_handler() {
	
	portBASE_TYPE xQueueStatus;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        rxToken tok;
	if (!(timer_tick++ % 32)) {
		if( xRxQueue && xQueueIsQueueFullFromISR( xRxQueue ) != pdTRUE ) {
			unsigned portBASE_TYPE uxSavedInterruptStatus;
			uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
			xQueueSendFromISR( xRxQueue, ( void * ) &tok, &xHigherPriorityTaskWoken );
			portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );
		}
	}
	/*portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );*/
	return 0;
}

#define TIMERIRQ 9
volatile int *irqreg = (volatile int *) 0x80000200; 
volatile int *gptreg = (volatile int *) 0x80000300; 

int main() {

	xTaskCreate( vRxTask, ( signed portCHAR * ) "rx", rx_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, NULL );
	xTaskCreate( vTxTask, ( signed portCHAR * ) "tx", tx_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, NULL );
	
	/* The suicide tasks must be created last as they need to know how many
	   tasks were running prior to their creation in order to ascertain whether
	   or not the correct/expected number of tasks are running at any given time. */
	vCreateSuicidalTasks( mainCREATOR_TASK_PRIORITY );

	xRxQueue = xQueueCreate( intqQUEUE_LENGTH, ( unsigned portBASE_TYPE ) sizeof( rxToken ) );
	vQueueAddToRegistry( xRxQueue, ( signed portCHAR * ) "xRxQueue" );

	catch_interrupt(timer_handler, TIMERIRQ);    

	irqreg[0] = 0;
	irqreg[0x04/4] = 0;
	irqreg[0x08/4] = 0;
	irqreg[0x40/4] = (1<<TIMERIRQ);  
	irqreg[0x80/4] = 0;  
	
	gptreg[0] = 50;
	gptreg[0x4/4] = 50;
	gptreg[0x20/4] = 0x800;
	gptreg[0x24/4] = 0x800;
	gptreg[0x28/4] = 0x0B;
	
	/* Start the scheduler. */
	vTaskStartScheduler();
	
	while(1) {
        
	}
}
