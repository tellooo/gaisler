#include "rtemscompat.h"
#include <termios.h>

/* shared/leon3/include/leon.h */
#define LEON_REG_UART_CTRL_RE     0x00000001 /* Receiver enable */
#define LEON_REG_UART_CTRL_TE     0x00000002 /* Transmitter enable */
#define LEON_REG_UART_CTRL_RI     0x00000004 /* Receiver interrupt enable */
#define LEON_REG_UART_CTRL_TI     0x00000008 /* Transmitter interrupt enable */
#define LEON_REG_UART_CTRL_PS     0x00000010 /* Parity select */
#define LEON_REG_UART_CTRL_PE     0x00000020 /* Parity enable */
#define LEON_REG_UART_CTRL_FL     0x00000040 /* Flow control enable */
#define LEON_REG_UART_CTRL_LB     0x00000080 /* Loop Back enable */
#define LEON_REG_UART_CTRL_DB     0x00000800 /* Debug FIFO enable */
#define LEON_REG_UART_CTRL_SI     0x00004000 /* TX shift register empty IRQ enable */
#define LEON_REG_UART_CTRL_FA     0x80000000 /* FIFO Available */
#define LEON_REG_UART_CTRL_FA_BIT 31

#define LEON_REG_UART_STATUS_DR   0x00000001 /* Data Ready */
#define LEON_REG_UART_STATUS_TSE  0x00000002 /* TX Send Register Empty */
#define LEON_REG_UART_STATUS_THE  0x00000004 /* TX Hold Register Empty */
#define LEON_REG_UART_STATUS_BR   0x00000008 /* Break Error */
#define LEON_REG_UART_STATUS_OE   0x00000010 /* RX Overrun Error */
#define LEON_REG_UART_STATUS_PE   0x00000020 /* RX Parity Error */
#define LEON_REG_UART_STATUS_FE   0x00000040 /* RX Framing Error */
#define LEON_REG_UART_STATUS_TF   0x00000200 /* FIFO Full */
#define LEON_REG_UART_STATUS_ERR  0x00000078 /* Error Mask */

#define DEBUG 1

#define CONFIGURE_NUMBER_OF_TERMIOS_PORTS 2

#include "sparc/shared/uart/apbuart_cons.c"
#include "sparc/shared/uart/cons.c"

