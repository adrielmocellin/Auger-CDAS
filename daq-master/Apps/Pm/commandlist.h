/******************************************************************************
*  	         	Pierre Auger Observatory Communications Software	          *
*******************************************************************************
*
*  Filename : CommandList.c
*
*  Version Number : 1.0
*
*  Description : Packet type variables for the command interpreter
*
*  History :
*       Date        Who	Details
*       ====        === =======
*       13/10/02	VT	Created as version 1.0
*
******************************************************************************/


// Standard commands
#define DATA								'D'
#define DATA_ACKNOWLEDGE					'd'
#define ECHO								'E'
#define ECHO_ACKNOWLEDGE					'e'
#define CONTROL								'C'
#define CONTROL_ACKNOWLEDGE					'c'
#define SOFT_RESET							'R'
#define SOFT_RESET_ACKNOWELDGE				'r'
#define POWER_UP							'P'
#define RESET_AND_STATUS					'S'
#define VERSION_REQUEST						'V'
#define VERSION_REPLY						'v'
#define INVALID_COMMAND						'I'
#define TOGGLE_RESET_LINE					'T'
#define POWER_FAIL							'F'
#define GPS_POSITION_REPLY					'g'
#define DATA_INPUT_BUFFER_STATUS_REQUEST	'B'
#define DATA_LIMIT_EXCEEDED					'x'


// Control commands
#define SLOT_TYPE					't'
#define ARQ_MONITORING				'a'
#define PACKET_DISCARDED			'f'
