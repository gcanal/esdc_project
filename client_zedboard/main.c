/*
 * Copyright (c) 2007-2009 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <stdio.h>

#include "xparameters.h"
#include "lwipopts.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/init.h"
#include "netif/xadapter.h"
#include "lwip/dhcp.h"
#include "task.h"
#include "portmacro.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "ZedboardOLED.h"
#include "xgpio.h"

#define THREAD_STACKSIZE 2048 // Empirically chosen
#define STDOUT_IS_PS7_UART
#define UART_DEVICE_ID 0
#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_0_BASEADDR
// Parameter definitions
#define INTC_DEVICE_ID 		XPAR_PS7_SCUGIC_0_DEVICE_ID
#define BTNS_DEVICE_ID		XPAR_BTNS_5BIT_DEVICE_ID//XPAR_AXI_GPIO_0_DEVICE_ID
#define INTC_GPIO_INTERRUPT_ID XPAR_FABRIC_BTNS_5BIT_IP2INTC_IRPT_INTR//XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR
#define BTN_INT 			XGPIO_IR_CH1_MASK // This is the interrupt mask for channel one
#define DELAY 100000000

XGpio   BTNInst;
XScuGic INTCInst;
static int btn_value;

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#include "game.h"
#define ENT 0x01
#define DWN 0x02
#define LT 0x04
#define RT 0x08
#define UP 0x10
#define DEFAULT_CURSOR_X 5
#define DEFAULT_CURSOR_Y 1
//----------------------------------------------------
// PROTOTYPE FUNCTIONS
//----------------------------------------------------
int main_thread();
void BTN_Intr_Handler(void *baseaddr_p);
static int InterruptSystemSetup(XScuGic *XScuGicInstancePtr);
static int IntcInitFunction(u16 DeviceId, XGpio *GpioInstancePtr);

int posx, posy;

game *g;
int FLAG;

//----------------------------------------------------
//  INTERRUPT SERVICE ROUTINE(ISR)
// also know as : INTERRUPT HANDLER FUNCTION
// - called by the buttons interrupt, performs push buttons read
// - OLED displaying
//----------------------------------------------------
void BTN_Intr_Handler(void *InstancePtr)
{
	printf("HOLA HANDLER\r\n");
	// Ignore additional button presses
	if ((XGpio_InterruptGetStatus(&BTNInst) & BTN_INT) != BTN_INT) {
		return;
		// Disable GPIO interrupts
		XGpio_InterruptDisable(&BTNInst, BTN_INT);
	 }
//	clear(); // clear the OLED
	btn_value = XGpio_DiscreteRead(&BTNInst, 1);
	switch (btn_value){
			//Checking if BTNC was pressed
			case LT:
				printf("MOVE LEFT\r\n");
				posx=max(posx-1,0);
				break;
			case RT:
				printf("MOVE RIGHT\r\n");
				posx=min(15,posx+1);
				break;
			case UP:
				printf("MOVE UP\r\n");
				posy=max(0,posy-1);
				break;
			//Checking if BTNR was pressed
			case DWN:
				printf("MOVE DOWN\r\n");
				posy=min(2,posy+1);
				break;
			//Checking if BTNU was pressed
			case ENT:
				printf("END TURN\r\n");
				if (posx>6 && posx<=9 && posy>=0 && posy<3) {
					FLAG=1;//READ MOVE
				}
				break;
	}

    print_board(g, posx, posy);

	// Acknowledge GPIO interrupts
    (void)XGpio_InterruptClear(&BTNInst, BTN_INT);
    // Enable GPIO interrupts
    XGpio_InterruptEnable(&BTNInst, BTN_INT);

}

//----------------------------------------------------
// INTERRUPT SETUP FUNCTIONS
//----------------------------------------------------
int IntcInitFunction(u16 DeviceId, XGpio *GpioInstancePtr)
{
	XScuGic_Config *IntcConfig;
	int status;

	// Interrupt controller initialization
	IntcConfig = XScuGic_LookupConfig(DeviceId);
	status = XScuGic_CfgInitialize(&INTCInst, IntcConfig, IntcConfig->CpuBaseAddress);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Call interrupt setup function
	status = InterruptSystemSetup(&INTCInst);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Register GPIO interrupt handler
	status = XScuGic_Connect(&INTCInst,
					  	  	 INTC_GPIO_INTERRUPT_ID,
					  	  	 (Xil_ExceptionHandler)BTN_Intr_Handler,
					  	  	 (void *)GpioInstancePtr);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Enable GPIO interrupts
	XGpio_InterruptEnable(GpioInstancePtr, 1);
	XGpio_InterruptGlobalEnable(GpioInstancePtr);

	// Enable GPIO interrupts in the controller
	XScuGic_Enable(&INTCInst, INTC_GPIO_INTERRUPT_ID);

	return XST_SUCCESS;
}

int InterruptSystemSetup(XScuGic *XScuGicInstancePtr)
{
	// Register GIC interrupt handler
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 	 	 	 	 	 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
			 	 	 	 	 	 XScuGicInstancePtr);
	Xil_ExceptionEnable();
	return XST_SUCCESS;

}

int get_move(){
	while (1){
		if (FLAG) { //value pressed
			FLAG=0;
			printf("FLAG CHANGED");
			if (posx>6 && posx<=9 && posy>=0 && posy<3) {
				int move=posx-7+posy*3;
				printf("MOVE is %d \n",move);
				return move;
			}
		}
		usleep(10000);//10ms sleep
	}
}


void print_ip(char *msg, struct ip_addr *ip) {
    print(msg);
    xil_printf("%d.%d.%d.%d\r\n", ip4_addr1(ip), ip4_addr2(ip),
            ip4_addr3(ip), ip4_addr4(ip));
}

void print_ip_settings(struct ip_addr *ip, struct ip_addr *mask, struct ip_addr *gw) {
    print_ip("Board IP: ", ip);
    print_ip("Netmask : ", mask);
    print_ip("Gateway : ", gw);
}

int main() {

	printf("----- GAME STARTED ------\r\n");
	//set flag
	FLAG=0;

	sys_thread_new("main_thrd", (void(*)(void*))main_thread, 0,
				THREAD_STACKSIZE,
				DEFAULT_THREAD_PRIO);
	vTaskStartScheduler();
	while(1);

    return 0;
}

struct netif server_netif;

void network_thread(void *p) {
	//----------------------------------------------------
	// INITIALIZE GAME PARAMETERS
	//----------------------------------------------------
	game g2;
	init_game(&g2);
	g2.client_win=0;
	g2.server_win=0;
	g2.server_plays=1;
	posx=DEFAULT_CURSOR_X;
	posy=DEFAULT_CURSOR_Y;
	//----------------------------------------------------
	// INITIALIZE THE PERIPHERALS & SET DIRECTIONS OF GPIO
	//----------------------------------------------------
	int status;
	status = XGpio_Initialize(&BTNInst, BTNS_DEVICE_ID);
	if(status != XST_SUCCESS) {printf("XST_FAILURE\n");return;}//return XST_FAILURE;
	// Set all buttons direction to inputs
	XGpio_SetDataDirection(&BTNInst, 1, 0xFF);
	// Initialize interrupt controller
	status = IntcInitFunction(INTC_DEVICE_ID, &BTNInst);
	if(status != XST_SUCCESS) {printf("XST_FAILURE\n");return;}//return XST_FAILURE;
	//----------------------------------------------------
	// INITIALIZE SOCKET
	//----------------------------------------------------
	struct netif *netif;
	struct ip_addr ipaddr, netmask, gw;
	/* the mac address of the board. this should be unique per board */
	unsigned char mac_ethernet_address[] = { 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

	int socket_desc;
	struct sockaddr_in server;
	netif = &server_netif;
	/* initliaze IP addresses to be used */
	IP4_ADDR(&ipaddr,  169, 254,   39, 55);
	IP4_ADDR(&netmask, 255, 255, 0,  0);
	IP4_ADDR(&gw,      169, 254,   39,  1);
	print_ip_settings(&ipaddr, &netmask, &gw);

	if (!xemac_add(netif, &ipaddr, &netmask, &gw, mac_ethernet_address, PLATFORM_EMAC_BASEADDR)) {
		printf("Error adding N/W interface\r\n");
		return;
	}  /* Add network interface to the netif_list, and set it as default */
	netif_set_default(netif);
	netif_set_up(netif);/* specify that the network if is up */
	/* start packet receive thread - required for lwIP operation */
	sys_thread_new("xemacif_input_thread", (void(*)(void*))xemacif_input_thread, netif,
		THREAD_STACKSIZE,
		DEFAULT_THREAD_PRIO);

	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1) {
		printf("Could not create socket");
	}
	//set server properties
	memset((void*)&server, 0, sizeof server);
	//server.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	struct ip_addr ipaddr_server;
	IP4_ADDR(&ipaddr_server,  169, 254,   39, 1);
	server.sin_addr.s_addr = ipaddr_server.addr;
	server.sin_family = AF_INET;
	server.sin_port = htons( PORT );

	//Connect to remote server
	int errcode=lwip_connect(socket_desc , (struct sockaddr *)&server , sizeof(server));
	if ((errcode) < 0) {
		printf("Error code = %d.\n", errcode);
		printf("Error string = %s.\n", strerror(errno));
		puts("connect error");
		return;
	}
	puts("Connected\n");

	// assign socket to the game
	g2.socket=socket_desc;
	puts("Game Initialized\n");
	//update g variable
	g=&g2;

	// Lets play
	int move = -1;
	int connection = 1;
	while (!g->game_finished && connection){
		while (!g->server_plays){ // my turn to play
			print_board(g, posx, posy);
			move = get_move();
			connection = send_move(g, move);
			receive_game(g);
			display_game(g);
		}
		while (g->server_plays){
			usleep(1000000);
			//send update request
			connection = send_move(g, 33);
			receive_game(g);
			printf("\n - Display game.\n");
			display_game(g);
			print_board(g, posx, posy);
		}
	}
	return;
}

int main_thread(){
	/* initialize lwIP before calling sys_thread_new */
    lwip_init();

    /* any thread using lwIP should be created using sys_thread_new */
    sys_thread_new("NW_THRD", network_thread, NULL,
            THREAD_STACKSIZE,
            DEFAULT_THREAD_PRIO);
    return 0;
}

#ifdef __arm__
void vApplicationMallocFailedHook( void ){
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue or
	semaphore is created.  It is also called by various parts of the demo
	application.  If heap_1.c or heap_2.c are used, then the size of the heap
	available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	xil_printf("Memory Allocation Error\r\n");
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed char *pcTaskName ){
	( void ) pcTaskName;
	( void ) pxTask;

	/* vApplicationStackOverflowHook() will only be called if
	configCHECK_FOR_STACK_OVERFLOW is set to either 1 or 2.  The handle and name
	of the offending task will be passed into the hook function via its
	parameters.  However, when a stack has overflowed, it is possible that the
	parameters will have been corrupted, in which case the pxCurrentTCB variable
	can be inspected directly. */
	xil_printf("Stack Overflow in %s\r\n", pcTaskName);
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
void vApplicationSetupHardware( void ){

}

#endif

