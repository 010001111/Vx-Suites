#pragma once
/*
  ������� ��� �������� � �������� ���������� ������� �� ������ (BO_DEBUG == 2) ��� ��������
  (BO_DEBUG == 1).
*/

#if(BO_DEBUG > 0)

//��� ������
enum
{
	WDDT_INFO,    //����������.
	WDDT_WARNING, //��������������
	WDDT_ERROR    //������.
};

//������� ��� �������� �������.
#define WDEBUG_PREFIX(t, f) table->debug_WriteString(__FUNCTION__, __FILE__, __LINE__, t, L##f
#define WDEBUG0(t, f)                                                WDEBUG_PREFIX(t, f))
#define WDEBUG1(t, f, c1)                                            WDEBUG_PREFIX(t, f), c1)
#define WDEBUG2(t, f, c1, c2)                                        WDEBUG_PREFIX(t, f), c1, c2)
#define WDEBUG3(t, f, c1, c2, c3)                                    WDEBUG_PREFIX(t, f), c1, c2, c3)
#define WDEBUG4(t, f, c1, c2, c3, c4)                                WDEBUG_PREFIX(t, f), c1, c2, c3, c4)
#define WDEBUG5(t, f, c1, c2, c3, c4, c5)                            WDEBUG_PREFIX(t, f), c1, c2, c3, c4, c5)
#define WDEBUG6(t, f, c1, c2, c3, c4, c5, c6)                        WDEBUG_PREFIX(t, f), c1, c2, c3, c4, c5, c6)
#define WDEBUG7(t, f, c1, c2, c3, c4, c5, c6, c7)                    WDEBUG_PREFIX(t, f), c1, c2, c3, c4, c5, c6, c7)
#define WDEBUG8(t, f, c1, c2, c3, c4, c5, c6, c7, c8)                WDEBUG_PREFIX(t, f), c1, c2, c3, c4, c5, c6, c7, c8)
#define WDEBUG9(t, f, c1, c2, c3, c4, c5, c6, c7, c8, c9)            WDEBUG_PREFIX(t, f), c1, c2, c3, c4, c5, c6, c7, c8, c9)
#define WDEBUG10(t, f, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)      WDEBUG_PREFIX(t, f), c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)
#define WDEBUG11(t, f, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) WDEBUG_PREFIX(t, f), c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11)
#else
#  define WDEBUG0(t, f)
#  define WDEBUG1(t, f, c1)
#  define WDEBUG2(t, f, c1, c2)
#  define WDEBUG3(t, f, c1, c2, c3)
#  define WDEBUG4(t, f, c1, c2, c3, c4)
#  define WDEBUG5(t, f, c1, c2, c3, c4, c5)
#  define WDEBUG6(t, f, c1, c2, c3, c4, c5, c6)
#  define WDEBUG7(t, f, c1, c2, c3, c4, c5, c6, c7)
#  define WDEBUG8(t, f, c1, c2, c3, c4, c5, c6, c7, c8)
#  define WDEBUG9(t, f, c1, c2, c3, c4, c5, c6, c7, c8, c9)
#  define WDEBUG10(t, f, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)
#  define WDEBUG11(t, f, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11)
#endif
