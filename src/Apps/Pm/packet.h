/******************************************************************************
*  	         	Pierre Auger Observatory Communications Software	          *
*******************************************************************************
*
*  Filename : packet.h
*
*  Version Number : 1.0
*
*  Description : Header file for the packet driver library
*
*  History :
*       Date        Who	Details
*       ====        === =======
*       09/09/02	VT	Created as version 1.0
*
******************************************************************************/

// message states
#define MSG_STATE_IDLE 			0
#define MSG_STATE_PREAMBLE		1
#define MSG_STATE_LENGTH		2
#define MSG_STATE_TYPE			3
#define MSG_STATE_BSUID			4
#define MSG_STATE_DATA			5
#define MSG_STATE_CRC			6
#define MSG_STATE_ETX			7

#define RECEIVE_BUFFER_LENGTH		1500

// Packet Constants
#define DEFAULT_COM_PORT			COM_PORT5 	// Default com port
#define CBR_115200					0xFF22		// word for 115200 baud
#define RX_QUE_SIZE					2000		// size of receive que
#define TX_QUE_SIZE					2000		// size of transmit que
#define RX_PREAMBLE					"!BS2PC!"	// preamble received from board
#define RX_PREAMBLE_LENGTH    		7
#define TX_PREAMBLE 				"  !PC2BS!"	// preamble sent to board
#define TX_PREAMBLE_LENGTH			9
#define ETX							0xFF		// ETX character
#define DATA_LENGTH_MODIFIER		6			// byte difference between LENGTH and DATA
												// packet DATA length
#define CRC_LENGTH					4     		// No. of CRC bytes
#define MAX_DATA_LENGTH				300   		// Max length of message
/*
#define FROM_BSx_RX_PREAMBLE		"!BSx2LSx!"
#define FROM_BSx_RX_PREAMBLE_LENGTH 9

#define TO_NMPC_TX_PREAMBLE			"!BS2PC!"
#define TO_NMPC_TX_PREAMBLE_LENGTH	7
*/
#define MAX_RX_PREAMBLE_LENGTH		15

#define MAX_MSG_LENGTH				249
#define MIN_MSG_LENGTH				7
