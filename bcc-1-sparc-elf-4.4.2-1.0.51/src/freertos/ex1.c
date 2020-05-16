#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define rx_TASK_STACK_SIZE		( configMINIMAL_STACK_SIZE )
#define tx_TASK_STACK_SIZE		( configMINIMAL_STACK_SIZE )
#define mainCREATOR_TASK_PRIORITY       ( tskIDLE_PRIORITY + 3 )

void vRxTask( void *pvParameters ) {
	while (1) {
		printf("rx 0x%x\n", freeos_getsp());
		vTaskDelay( 10 );
	}
}

void vTxTask( void *pvParameters ) {
	while (1) {
		printf("tx 0x%x\n",freeos_getsp());
		vTaskDelay( 10 );
	}
}

int main() {
	
	xTaskCreate( vRxTask, ( signed portCHAR * ) "rx", rx_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, NULL );
	xTaskCreate( vTxTask, ( signed portCHAR * ) "tx", tx_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, NULL );
	
	/* The suicide tasks must be created last as they need to know how many
	   tasks were running prior to their creation in order to ascertain whether
	   or not the correct/expected number of tasks are running at any given time. */
	vCreateSuicidalTasks( mainCREATOR_TASK_PRIORITY );
	
	/* Start the scheduler. */
	vTaskStartScheduler();
	
	while(1) {
        
	}
}
