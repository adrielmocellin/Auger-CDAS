#ifndef _MrS_SUMMARYFILE_H
#define _MrS_SUMMARYFILE_H

//#include <stl.h>
//#include <TObject.h>
//#include "MoIO_ClassDef.h"

int OpenSummaryFile(string);
//static void BuildSummaryFilename(string);
//static int SummaryFileOk();
//static int BookSumTrees(Int_t);
void BookSpecialTrees();
//void DeleteSpecialTrees();
int FillSDSummary(Int_t);
int CloseRootSummaryFile();
int CheckFileToBeSummarizedHasNotYetBeenSummarized(string);
void CheckPointers();
#endif
