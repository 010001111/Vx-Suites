/*
  ����� �����, � ������� ������.
*/
#pragma once

/*
  ��������� ����������� ��������, � ����������� ������� ���������� ������.

  Return - true - ���������� ����� �������,
           false - ���������� ������ �������.
*/
#define WM_CANCLOSE (WM_USER + 1)

//���������� ����������.
extern HMODULE currentModule;       //����� �������� ������.
extern WCHAR homePath[MAX_PATH];    //�������� ����������.
extern WCHAR settingsFile[MAX_PATH]; //���� �����.
