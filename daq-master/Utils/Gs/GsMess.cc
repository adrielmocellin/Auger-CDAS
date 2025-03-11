/*-----------------------------------------------------------------------------*/
/*
  GsMessFormat.c  : routines to decode messages.
*/
/*-----------------------------------------------------------------------------*/

#include <netinet/in.h>
#include <string.h>

#include "GsMess.h"

/******************************************************************************
 *
 * longWord GsMessNumServ(GsStdHeader *header)
 *              message : message received including the header
 *
 *  This routine does the following - 
 *            - return the number of services specified in the data block
 *                  of the message
 *
 *  Returns -
 *       SUCCESS : number of services
 *       FAILURE : ---
 *
 ******************************************************************************
 */
longWord GsMessNumServ(GsStdHeader *header)
{
  GSMESSAGE *gsMessage = (GSMESSAGE *)header;

  return(ntohl(gsMessage->msg.numServices));
}

/******************************************************************************
 *
 * longWord GsMessServInfo(GsStdHeader *header, longWord numServ, void **params)
 *            message : message received including the header
 *            numServ : number of the service about which to return information
 *            params : returned pointer, pointing to the start of the param block
 *
 *  This routine does the following - 
 *            - return the id corresponding to the numServ service in the message.*                  also return a pointer to the start of the data block
 *
 *  Returns -
 *       SUCCESS : service Id
 *       FAILURE : 0 if numServ does not refer to a service in the message 
 *                   (i.e. - the number is too high)
 *
 ******************************************************************************
 */
longWord GsMessServInfo(GsStdHeader *header, longWord numServ, 
			longWord **params)
{
  GSMESSAGE *gsMessage = (GSMESSAGE *)header;
  longWord serviceId = 0, paramBytes = 0;
  SERVICEPARAMS *serviceParams;
  int i;
  char *charPtr;

  if ((numServ < 1) || (numServ > GsMessNumServ(header))) {
    /* an invalid service was requested */
    *params = NULL;
  } else {
    /* the service requested exists in the message. point things to the first
       service */
    *params = (longWord *)(((char*)(&(gsMessage->msg.block))) + 
			   NUMSERVICESLEN);
    charPtr = (char *)*params;       /* we need to do char ptr math below */
    for (i = 1; i <= numServ; ++i) {
      charPtr += paramBytes;             /* move to next service id */
      serviceParams = (SERVICEPARAMS *)charPtr;
      serviceId = ntohl(serviceParams->serviceId);
      paramBytes = ntohl(serviceParams->paramBytes);
      charPtr += SERVICEPARAMSLEN;       /* point to the start of the params */
    }
    *params = (longWord *)charPtr;
  }
  return serviceId;
}

/******************************************************************************
 *
 * void *GsMessParam(void *parameters, PARAMTYPE paramType, void **nextParam)
 *            parameters : point to a parameter  in the parameter block
 *            paramType  : the type of the next parameter
 *            nextParam  : a returned pointer, pointing to the start of the
 *                           following parameter
 *
 *  This routine does the following - 
 *            - return the decoded parameter that the initial pointer,
 *                  parameters, is pointing to.  also return a pointer to the
 *                  start of the next parameter.
 *
 *  Returns -
 *       SUCCESS : decoded parameter value
 *       FAILURE : nextParam will be set to NULL, the return value is NULL
 *
 ******************************************************************************
 */
void *GsMessParam(void *parameters, PARAMTYPE paramType, void **nextParam)
{
  char *c_nextParam = (char *)parameters;
  longWord lWord;
  word sWord;
  double aDouble, *aDoublePtr;
  long int ret;

  switch (paramType) {
  case GSLINT:
    /* decode an unsigned long integer */
    c_nextParam += sizeof(longWord);
    *nextParam = c_nextParam;
    memcpy(&lWord, parameters, sizeof(longWord));
    ret=ntohl(lWord);
    return((void *)ret);
    break;
  case GSSINT:
    /* decode an unsigned short integer */
    c_nextParam += sizeof(word);
    *nextParam = c_nextParam;
    memcpy(&sWord, parameters, sizeof(word));
    ret=ntohs(sWord);
    return((void *)(ret));
    break;
  case GSDBL:
    aDoublePtr = &aDouble;
    /* decode a double */
    c_nextParam += sizeof(double);
    *nextParam = c_nextParam;
    memcpy(aDoublePtr, parameters, sizeof(double));
    return((void *)aDoublePtr);
    break;
  default:
    /* this is an error, we shouldn't reach here unless paramType is invalid */
    *nextParam = NULL;
    return(NULL);
    break;
  }
}

/******************************************************************************
 *
 * void *GsMessParamSet(void *param, void *parameters, PARAMTYPE paramType, 
 *                      void **nextParam)
 *            param      : the parameter to add to the parameter block
 *            parameters : point to the next available space in the parameter 
 *                           block
 *            paramType  : the type of the parameter 
 *            nextParam  : a returned pointer, pointing to the next available
 *                           byte after the current parameter
 *
 *  This routine does the following - 
 *            - encode the passed  parameter into the parameter block pointed
 *                  to by parameter.
 *
 *  Returns -
 *       SUCCESS : CDAS_SUCCESS
 *       FAILURE : nextParam will be set to NULL, the return value is NULL
 *
 ******************************************************************************
 */
int GsMessParamSet(void *param, void *parameters, PARAMTYPE paramType, 
		   void **nextParam)
{
  int rtn = CDAS_SUCCESS;
  char *c_nextParam = (char *)parameters;
  longWord aLong;
  word aShort;
  double aDouble;

  switch (paramType) {
  case GSLINT:
    /* encode an unsigned long integer */
    c_nextParam += sizeof(longWord);
    aLong = htonl(*((longWord *)param));
    memcpy(parameters, &aLong, sizeof(longWord));
    break;
  case GSSINT:
    /* encode an unsigned short integer */
    c_nextParam += sizeof(word);
    aShort = htons(*((word *)param));
    memcpy(parameters, &aShort, sizeof(word));
    break;
  case GSDBL:
    /* encode a double */
    c_nextParam += sizeof(double);
    aDouble = *((double *)param);
    memcpy(parameters, &aDouble, sizeof(double));
    break;
  default:
    /* this is an error, we shouldn't reach here unless paramType is invalid */
    *nextParam = NULL;
    rtn = CDAS_NOT_FOUND;
    break;
  }
  *nextParam = c_nextParam;
  return(rtn);
}
