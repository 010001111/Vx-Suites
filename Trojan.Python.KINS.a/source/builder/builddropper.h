/*
  ������ droppera.
*/
#pragma once

#include "..\common\config0.h"

namespace BuildDropper
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
    ������ ������ ����.

    IN owner      - �������� ����.
    IN output     - ���� ��� ������ �������.
    IN config     - ������������.
    IN destFolder - ����� ���������� ��� ��������� �����.

    Return        - true - � ������ ������,
                    false - � ������ ������.
  */
  bool _run(HWND owner, HWND output, Config0::CFGDATA *config, LPWSTR destFolder);
};
