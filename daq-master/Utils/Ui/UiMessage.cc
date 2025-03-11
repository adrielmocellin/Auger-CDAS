/*---------------------------------------------------------------------------*/
/*
  Routines to handle the messages from Ik.
*/
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "UiMessage.h"

/***************************/
/* misc */
#define IKMODESTRIS(mode,flag) (strstr((mode),(flag))) 

/*********************************************/
/* temporary parser for data part of Ik message */
UiIkListInfo IkList;
int IkIntList[4096]; /* TO BE CHANGED? temporary */
int IkIntListTmp[30000]; /* TO BE CHANGED? temporary */
int IkIntListTmp2[30000]; /* TO BE CHANGED? temporary */

static void initIkList(UiIkListInfo *IkList)
{
  if (IkList) {
    IkList->mode = TYPEUNKNOWN;
    IkList->inputString = NULL;
    IkList->inputStringSize = 0;
    IkList->commonPart = NULL;
    IkList->commonPartSize = 0;
    IkList->specificPart = NULL;
    IkList->specificPartSize = 0;
    IkList->intList = NULL;
    IkList->number = 0;
  }
}

static UiIkListInfo *decodeLIST(char *inputStr, int usedChars, 
				UiIkListInfo *IkList)
{
  int i = 0;
  int *listPtr = IkIntList;

  while (sscanf(inputStr,"%d%n",listPtr++,&usedChars) >0 ) {
    inputStr += usedChars;
    i++;
  }
  IkList->mode = TYPELIST;
  IkList->number = i;
  IkList->intList = IkIntList;
  return (IkList);
}

static UiIkListInfo *decodeSLIST(char *inputStr, int usedChars,
				 UiIkListInfo *IkList)
{
  int size, i, j;
  int *listPtr;

  sscanf(inputStr,"%d%n",&size,&usedChars);
  if (size<0) {
    return(NULL);
  }
  inputStr += usedChars;
  for(i=0, listPtr = IkIntListTmp;i<size;i++) {
    sscanf(inputStr,"%d%n",listPtr++,&usedChars);
    inputStr += usedChars;
  }
  IkList->mode = TYPESLIST;
  IkList->commonPart = IkIntListTmp;
  IkList->commonPartSize = size;
  i = 0;
  listPtr = IkIntList;
  while (sscanf(inputStr,"%d%n",listPtr++,&usedChars) >0 ) {
    inputStr += usedChars;
    i++;
  }
  IkList->number = i;
  IkList->intList = IkIntList;  
  return (IkList);
}

static UiIkListInfo *decodeMLIST(char *inputStr, int usedChars,
				 UiIkListInfo *IkList)
{
  int size, i, j;
  int *listPtr, *listPtr2;

  sscanf(inputStr,"%d%n",&size,&usedChars);
  if (size<0){
    return(NULL);
  }
  inputStr += usedChars;
  for(i=0, listPtr = IkIntListTmp;i<size;i++) {
    sscanf(inputStr,"%d%n",listPtr++,&usedChars);
    inputStr += usedChars;
  }
  IkList->mode = TYPEMLIST;
  IkList->commonPart = IkIntListTmp ;
  IkList->commonPartSize = size;

  sscanf(inputStr,"%d%n",&size,&usedChars);
  i = 0;
  listPtr = IkIntList;
  listPtr2 = IkIntListTmp2;
  while (sscanf(inputStr,"%d%n",listPtr++,&usedChars) >0 ) {
    inputStr += usedChars;
    for(j=0;j<size;j++) {
      sscanf(inputStr,"%d%n",listPtr2++,&usedChars);
      inputStr += usedChars;
    }
    i++;
  }
  IkList->specificPart = listPtr2;
  IkList->specificPartSize = size;
  IkList->number = i;
  IkList->intList = listPtr;
  return (IkList);
}

static UiIkListInfo *decodeALL(char *inputStr, int usedChars,
			       UiIkListInfo *IkList)
{
  IkList->mode = TYPEALL;
  IkList->inputString = inputStr;
  IkList->inputStringSize = strlen(inputStr);
  return (IkList);
}

static UiIkListInfo *decodeBROADCAST(char *inputStr, int usedChars,
				     UiIkListInfo *IkList)
{
  IkList->mode = TYPEBROADCAST;
  IkList->inputString = inputStr;
  IkList->inputStringSize = strlen(inputStr);
  return (IkList);
}

static UiIkListInfo *decodeUNKNOWN(char *inputStr, UiIkListInfo *IkList)
{
  IkList->mode = TYPEUNKNOWN;
  IkList->inputString = inputStr;
  IkList->inputStringSize = strlen(inputStr);
  return(IkList);
}
/*****************************************************************************
 * 
 *  UiParseIkData()
 *
 *  This routine does the following -
 *         - parse the message according to the following assumptions :
 *
 *           o the text part of the message has the following format -
 *                command message-type message
 *
 *  Returns - None
 *
 *****************************************************************************
 */
UiIkListInfoP UiDecodeIkData(char *data)
{
  char * inputStr;
  int usedChars = 0;
  char command[30];
  char ltype[30];
  UiIkListInfo *p_IkList = &IkList;

  ltype[0] = 0; 
  sscanf(data,"%30s %30s %n",command,ltype,&usedChars);
  inputStr = data + usedChars;
  initIkList(p_IkList);
  if (IKMODESTRIS(ltype,LIST)) {
    p_IkList = decodeLIST(inputStr, usedChars, p_IkList);
  } else if (IKMODESTRIS(ltype,SLIST)) {
    p_IkList = decodeSLIST(inputStr, usedChars, p_IkList);
  } else if (IKMODESTRIS(ltype,MLIST)) {
    p_IkList = decodeMLIST(inputStr, usedChars, p_IkList);
  } else if (IKMODESTRIS(ltype,ALL)) {
    p_IkList = decodeMLIST(inputStr, usedChars, p_IkList);
  } else if (IKMODESTRIS(ltype,BROADCAST)) {
    p_IkList = decodeBROADCAST(inputStr, usedChars, p_IkList);
  } else {
    /* unknown format : FLIST for example */
    p_IkList = decodeUNKNOWN(data, p_IkList);
  }
  return(p_IkList);
}
