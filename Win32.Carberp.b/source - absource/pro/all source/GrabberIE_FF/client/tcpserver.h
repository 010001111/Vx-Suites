/*
  ������ ���������� TCP-�������.
*/
#pragma once

#include "..\common\threadsgroup.h"

namespace TcpServer
{
  /*
    �������������.
  */
  void init(void);

  /*
    ���������������.
  */
  void uninit(void);

  /*
    �������� �������.

    IN OUT group - ������ �������, � ������� ����� ������������ ������ �������.
    
    Return       - true - � ������ ������,
                   false - � ������ ������.
  */
  bool _create(ThreadsGroup::GROUP *group);
};
