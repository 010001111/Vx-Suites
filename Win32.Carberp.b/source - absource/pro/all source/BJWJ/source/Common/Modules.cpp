

//************************************************************************
//
//  ���� ��������������� ���������/���������� ������������ �������
//
//************************************************************************

#include "Modules.h"
//************************************************************************


#ifdef RafaH  /* ������ Rafa.h */
	#include "Rafa.cpp"
#endif


// ������ ������ ����� �����������
#ifdef PrivatBankH /* ������ PrivatBank.h */
	#include "PrivatBank.cpp"
#endif



#ifdef HTMLInjectLogH
	#include "HTMLInjectLog.cpp"
#endif

#ifdef BitcoinH
	#include "bitcoin.cpp"
#endif

#ifdef AmmyyH
	#include "ammyy.cpp"
#endif
