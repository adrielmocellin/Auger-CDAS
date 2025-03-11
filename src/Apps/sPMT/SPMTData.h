#include "SPMTRoot.h"

class SPMTData
{
public:
  SPMTData();
  ~SPMTData();

  int SetData(char *);
  void SetOutPath(const char *path);

private:
  int _Process_data(int lsId,unsigned char *buf,int size);
  SPMTRoot *fFileManager;

  int debug_raw_index;
};
