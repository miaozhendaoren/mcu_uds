/***************************************************************************//**
    \file          network_layer.h
    \author        huanghai
    \mail          huanghai@auto-link.com
    \version       0
    \date          2016-09-24
    \description   uds network code, base on ISO 15765
*******************************************************************************/
#ifndef _NETWORK_LAYER_H_
#define _NETWORK_LAYER_H_

/*******************************************************************************
    Include Files
*******************************************************************************/
#include <stdint.h>

/*******************************************************************************
    Type Definition
*******************************************************************************/
/**
	\def TRUE
	Bool value true.
*/
#ifndef TRUE
#define	TRUE	(1)
#endif

/**
	\def FALSE
	Bool value false.
*/
#ifndef FALSE
#define FALSE	(0)
#endif
/**
	\def NULL
	NULL type.
*/
#ifndef NULL
#define NULL  ((void *) 0)
#endif
/**
	\typedef unsigned char bool_t
	Bool type.
*/
typedef unsigned char		bool_t;



typedef enum _N_TATYPE_T_
{
    N_TATYPE_NONE = 0,
    N_TATYPE_PHYSICAL,
    N_TATYPE_FUNCTIONAL
}n_tatype_t;

typedef enum _N_RESULT_
{
    N_OK = 0,
    N_TIMEOUT_A,
    N_TIMEOUT_Bs,
    N_TIMEOUT_Cr,
    N_WRONG_SN,
    N_INVALID_FS,
    N_UNEXP_PDU,
    N_WFT_OVRN,
    N_BUFFER_OVFLW,
    N_ERROR
}n_result_t;


typedef void
(*indication_func) (uint8_t msg_buf[], uint16_t msg_dlc, n_result_t n_result);
typedef void
(*confirm_func) (n_result_t n_result);

typedef struct _NETWORK_USER_DATA_T_
{
    indication_func indication;
    confirm_func    confirm;
}nt_usdata_t;
/*******************************************************************************
    Function  Definition
*******************************************************************************/


/**
 * network_main - network main task, should be schedule every one ms
 *
 * @void
 *
 * returns:
 *     void
 */
extern void
network_main(void);

/**
 * netowrk_recv_frame - recieved uds network can frame
 *
 * @frame_buf : uds can frame data buffer
 * @frame_dlc : uds can frame length
 *
 * returns:
 *     void
 */
extern void
netowrk_recv_frame(uint8_t frame_buf[], uint8_t frame_dlc);

/**
 * netowrk_send_udsmsg - send a uds msg by can
 *
 * @msg_buf : uds msg data buffer
 * @msg_dlc : uds msg length
 *
 * returns:
 *     void
 */
extern void
netowrk_send_udsmsg(uint8_t msg_buf[], uint16_t msg_dlc);


/**
 * netowrk_reg_usdata - reg usdata Function
 *
 * @usdata : uds msg data Function struct
 *
 * returns:
 *     0 - ok, other - err
 */
extern int
netowrk_reg_usdata(nt_usdata_t usdata);
#endif
/****************EOF****************/
