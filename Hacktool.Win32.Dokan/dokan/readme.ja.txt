
    Dokan ���C�u����

 Copyright(c) Hiroki Asakawa http://dokan-dev.net


�� Dokan���C�u�����Ƃ�

Windows�Ńt�@�C���V�X�e���C�Ⴆ�� FAT �� NTFS �̉��ǃo�[�W�������쐬��
�悤�Ǝv�����Ƃ��ɂ́C�t�@�C���V�X�e���h���C�o���쐬����K�v������܂��D
Windows�ŃJ�[�l�����[�h�œ��삷��C�f�o�C�X�h���C�o���쐬����͔̂���
����C�ȒP�ɍ쐬���邱�Ƃ��o���܂���DDokan���C�u�������g�p���邱�Ƃ�
�f�o�C�X�h���C�o�������Ȃ��Ă��C�V�����t�@�C���V�X�e�����쐬���邱�Ƃ�
�o���܂��DFUSE �� Windows �łƂ������郉�C�u�����ł��D


�� ���C�Z���X�ɂ���

Dokan���C�u�����͕����̃��C�Z���X�̃v���O��������\������Ă��܂��D

���[�U���[�h���C�u���� (dokan.dll) LGPL
�h���C�o (dokan.sys)               LGPL
�⏕�v���O���� (dokanctl.exe)      MIT
�}�E���g�T�[�r�X (mounter.exe)     MIT

���ꂼ��̃��C�Z���X�̏ڍׂ́C
LGPL license.lgpl.txt
GPL  license.gpl.txt
MIT  license.mit.txt
���Q�Ƃ��ĉ������D
�\�[�X�R�[�h�� http://dokan-dev.net/download �������ł��܂��D

�� �����

Windowx XP,Vista,2003 x86�ŁCWindows Vista x64�łœ��삵�܂��D


�� ����̎d�g��

Dokan���C�u�����́C���[�U���[�h�̃��C�u�����ł��� dokan.dll �ƃt�@�C��
�V�X�e���h���C�o�ł��� dokan.sys ����\������Ă��܂��Ddokan.sys���C��
�X�g�[�����邱�ƂŁCWindows ����͒ʏ�̃t�@�C���V�X�e�������݂���悤
�ɂɌ����܂��Ddokan�t�@�C���V�X�e���ɑ΂���A�N�Z�X������ƁC���[�U���[
�h�ɃR�[���o�b�N����܂��D�Ⴆ�΁C�G�N�X�v���[���Ńf�B���N�g�����J����
�Ƃ��ɁCWindows�̓t�@�C���V�X�e���ɑ΂��āC�f�B���N�g���ꗗ��v�����܂��D
���̗v�����Cdokan.dll �ɃR�[���o�b�N����܂��D���̗v���ɑ΂��āC�K����
�f�B���N�g���ꗗ���t�@�C���V�X�e���ɖ߂��ƁCdokan�t�@�C���V�X�e���́C��
���Windows�ɕԂ��܂��DDokan�t�@�C���V�X�e�����v���L�V�Ƃ��ĉ�邱��
�ŁC���[�U���[�h�̒ʏ�v���O�����Ƃ��ăt�@�C���V�X�e���������ł���킯
�ł��D


�� ���C�u�����̍\���ƃC���X�g�[��

�C���X�g�[���[�����s����ƕW���ł͈ȉ��̃t�@�C�����C���X�g�[������C
�h���C�o�[�ƃT�[�r�X�v���O�������V�X�e���ɓo�^����܂��D

SystemFolder\dokan.dll
    Dokan���[�U���[�h���C�u����

SystemFolder\drivers\dokan.sys
    Dokan�t�@�C���V�X�e���h���C�o

ProgramFilesFolder\Dokan\DokanLibrary\mounter.exe
    Dokan�}���g�T�[�r�X

ProgramFilesFolder\Dokan\DokanLibrary\dokanctl.exe
    Dokan�R���g���[���v���O����

ProgramFilesFolder\Dokan\DokanLibrary\dokan.lib
    Dokan�C���|�[�g���C�u����

ProgramFilesFolder\Dokan\DokanLibrary\dokan.h
    Dokan���C�u�����w�b�_�[

ProgramFilesFolder\Dokan\DokanLibrary\readme.txt
    ���̃t�@�C��


�A���C���X�g�[���̓R���g���[���p�l���̃A�v���P�[�V�����̒ǉ��ƍ폜����
�s���Ă��������D�A���C���X�g�[����ăC���X�g�[������ɂ͍ċN�����K�v��
�Ȃ�܂��D(�K���ċN�����Ă�������)


�� �t�@�C���V�X�e���̍���

dokan.h �� DOKAN_OPERATIONS �̊e�֐����������CDokanMain �ɓn�����ƂŃ}
�E���g���܂��D���ꂼ��̊֐���WindowsAPI���̈��������܂��D�e�֐��̂�
���܂��Ȏd�l�͓�����WindowsAPI�Ƃقړ����ł��D
DOKAN_OPERATIONS �̊e�֐��́C�C�ӂ̃X���b�h����Ă΂�܂��D�X���b�h�Z�[
�t�łȂ��A�v���P�[�V�����Ŗ��ɂȂ�ꍇ������܂��D

���̏����ŌĂ΂�܂�

1. CreateFile(OpenDirectory, OpenDirectory)
2. ���̑��̊֐�
3. Cleanup
4. CloseFile

�t�@�C���ɑ΂���A�N�Z�X(�f�B���N�g���ꗗ��C�t�@�C���̑����̎擾�Ȃǁj
�̏��߂ɁC���Ȃ炸CreateFile�n�̊֐����Ă΂�C�t�@�C���n���h�����N���[
�Y�����Ƃ�(WindowsAPI��CloseFile���Ă΂ꂽ�Ƃ�)��Cleanup���Ă΂�C��
�̌�CloseFile ���Ă΂�܂��D

���ꂼ��̊֐��͐���I�������ꍇ�C0���C�G���[���������ꍇ�́CWindows��
�G���[�R�[�h�� -1�{�������̂�Ԃ��Ă��������D

���ꂼ��̊֐��̍Ō�̈����ɂ� DOKAN_FILE_INFO ���n����܂��D���̍\����
�́C���̂悤�ɒ�`����Ă��܂��D

    typedef struct _DOKAN_FILE_INFO {

        ULONG64 Context; 
        ULONG64 DokanContext;
        ULONG   ProcessId;
        BOOL    IsDirectory;

    } DOKAN_FILE_INFO, *PDOKAN_FILE_INFO;

���[�U���[�h���瓯���t�@�C���n���h���𗘗p���ẴA�N�Z�X�ɂ������ẮC
���� DOKAN_FILE_INFO ���֐��ɓn����Ă��܂��D���̍\���̂́CCreateFile
�Ő�������CCloseFile�ŉ������܂��DDokanFileInfo->Context �̓t�@�C��
�V�X�e�������R�Ɏg�p�ł���ϐ��ł��D��A�̃t�@�C���A�N�Z�X�ɑ΂��āC��
�������̂ŁC�t�@�C���n���h���Ȃǂ�ۑ�����̂ɗ��p�ł��܂��D
DokanFileInfo->DokanContext �͓��������p�ł��D�����Ȃ��ł��������D
DokanFileInfo->ProcessId �� IO���N�G�X�g�𐶐������v���Z�XID�ł��D
DokanFileInfo->IsDirectory �̓f�B���N�g���ɑ΂���A�N�Z�X���� TRUE ���Z�b
�g����Ă��܂�(��O�����q)�D

    int (*CreateFile) (
        LPCWSTR,      // FileName
        DWORD,        // DesiredAccess
        DWORD,        // ShareMode
        DWORD,        // CreationDisposition
        DWORD,        // FlagsAndAttributes
        PDOKAN_FILE_INFO);

    int (*OpenDirectory) (
        LPCWSTR,          // FileName
        PDOKAN_FILE_INFO);

    int (*CreateDirectory) (
        LPCWSTR,          // FileName
        PDOKAN_FILE_INFO);

CreateFile �t�@�C���̐V�K�쐬�C�J���Ȃ�OpenDirectory �f�B���N�g�����J��
CreateDirectory �f�B���N�g�����쐬���邻�ꂼ��̊֐��́C�t�@�C���ւ̃A
�N�Z�X�J�n���ɌĂ΂�܂��D�֐��̎d�l�́CWindowsAPI�Ɏ����Ă���܂��D
CreateFile ��DesiredAccess ShareMode CreationDisposition
FlagsAndAttributes �ɂ��ẮCMSDN �� CreateFile ���Q�Ƃ��Ă��������D
�f�B���N�g���ɑ΂���A�N�Z�X�̎��ɂ́COpenDirectory �܂��́C
CreateDirectory ���Ă΂�܂��D���̏ꍇ�CDokanFileInfo->IsDirectory ��
TRUE �ɂȂ��Ă��܂��D�f�B���N�g���ɑ΂���A�N�Z�X�ŗL��̂ɂ��ւ�炸�C
CreateFile ���Ă΂�邱�Ƃ�����܂��D���̏ꍇ�́C
DokanFileInfo->IsDirectory �� FALSE ���Z�b�g����Ă��܂��D�f�B���N�g��
�̑������擾����ꍇ�Ȃǂ� OpenDirectory �ł͂Ȃ��CCreateFile���Ă΂��
�悤�ł��D�f�B���N�g���ɑ΂���A�N�Z�X�Ȃ̂ɂ��ւ�炸�CCreateFile ����
�΂ꂽ�ꍇ�́C�K�� DokanFileInfo->IsDirectory ��TRUE���Z�b�g���Ă���
return���Ă��������D�������Z�b�g����Ă��Ȃ��ƁCDokan���C�u�����́C����
�A�N�Z�X���f�B���N�g���ɑ΂���A�N�Z�X���ǂ������f�ł����CDokan�t�@�C��
�V�X�e���� Windows �ɑ΂��Đ��m�ȏ���Ԃ����Ƃ��o���Ȃ��Ȃ�܂��D

CreateFile �� CreationDisposition �� CREATE_ALWAYS ��������
OPEN_ALWAYS �̏ꍇ�ŁC�t�@�C�������łɑ��݂��Ă����ꍇ�́C0�ł͂Ȃ��C
ERROR_ALREADY_EXISTS(183) (���̒l) ��Ԃ��Ă��������D


    int (*Cleanup) (
        LPCWSTR,      // FileName
        PDOKAN_FILE_INFO);

    int (*CloseFile) (
        LPCWSTR,      // FileName
        PDOKAN_FILE_INFO);

Cleanup �̓��[�U�� WindowsAPI �� CloseHandle ���Ă񂾂Ƃ��ɌĂ΂�܂��D
CreateFile �̎��ɁC�t�@�C�����J���C�t�@�C���n���h����Ⴆ�΁C
DokanFileInfo->Context �ɕۑ����Ă���ꍇ�CCloseFile���ł͂Ȃ��C
Cleanup���ɂ��̃t�@�C���n���h�������ׂ��ł��D���[�U�� WindowsAPI ��
CloseHandle ���Ă�ŁC���̌㓯���t�@�C�����J�����ꍇ�CCleanup ���Ă΂�
�Ă� CloseFile ���V���Ƀt�@�C�����J���O�ɌĂ΂�Ȃ��ꍇ������܂��D�t�@
�C���V�X�e�����t�@�C�����J�����܂܂ɂ��Ă���ꍇ�C�t�@�C���̋��L�ᔽ��
�ēx�J���Ȃ��Ȃ邩������܂���D�y���Ӂz���[�U���t�@�C�����������}�b�v
�h�t�@�C���Ƃ��ĊJ���Ă���ꍇ�CCleanup ���Ă΂ꂽ��� WriteFile ��
ReadFile ���Ă΂��ꍇ������܂��D���̏ꍇ�ɂ�����ɓǂݍ��ݏ������݂�
�ł���悤�ɂ���ׂ��ł��D


    int (*FindFiles) (
        LPCWSTR,           // PathName
        PFillFindData,     // call this function with PWIN32_FIND_DATAW
        PDOKAN_FILE_INFO); //  (see PFillFindData definition)


    // You should implement FindFires or FindFilesWithPattern
    int (*FindFilesWithPattern) (
        LPCWSTR,           // PathName
        LPCWSTR,           // SearchPattern
        PFillFindData,     // call this function with PWIN32_FIND_DATAW
        PDOKAN_FILE_INFO);


FindFiles �� FindFilesWithPattern �̓f�B���N�g���ꗗ���擾����Ƃ��Ɍ�
�΂�܂��D�f�B���N�g������ WIN32_FIND_DATAW �Ɋi�[���C�����ɓn�����
FillFindData �֐��|�C���^��ʂ��āCFillFindData(&win32FindDataw,
DokanFileInfo) ���P�G���g�����ƂɌĂяo���Ă��������D
FindFilesWithPattern �̓f�B���N�g���̌����p�^�[���t���ŌĂяo����܂��D
Windows �́C���C���h�J�[�h�̓W�J���V�F���ł͂Ȃ��C�t�@�C���V�X�e���ōs
���܂��D���C���h�J�[�h�W�J�𐧌䂵�����ꍇ�́CFindFilesWithPattern ���
�`���Ă��������DFindFiles �̓��C���h�J�[�h����� Dokan���C�u�������s��
�܂��Ddokan.dll �� DokanIsNameInExpression ���G�N�X�|�[�g���Ă���C���C
���h�J�[�h�̃}�b�`���O�ɗ��p�ł��܂��D


�� �}�E���g

    typedef struct _DOKAN_OPTIONS {
        WCHAR   DriveLetter; // driver letter to be mounted
        ULONG   ThreadCount; // number of threads to be used
        UCHAR   DebugMode; // ouput debug message
        UCHAR   UseStdErr; // ouput debug message to stderr
        UCHAR   UseAltStream; // use alternate stream

    } DOKAN_OPTIONS, *PDOKAN_OPTIONS;

    int DOKANAPI DokanMain(
        PDOKAN_OPTIONS    DokanOptions,
        PDOKAN_OPERATIONS DokanOperations);

DokanOptions �ɂ́CDokan �̎��s�I�v�V�����CDokanOptions �Ɋe�f�B�X�p�b�`
�֐��̊֐��|�C���^���w�肵�āCDokanMain ���Ăт܂��DDokanMain �̓A���}�E
���g����܂Ő����Ԃ��܂���D�܂��C�e�f�B�X�p�b�`�֐��́CDokanMain ����
�񂾃X���b�h�R���e�L�X�g�ƈقȂ镡���̃X���b�h�R���e�L�X�g����Ă΂�܂��D
�f�B�X�p�b�`�֐��́C�X���b�h�Z�[�t�ɂȂ�悤�ɂ��ĉ������D

DOKAN_OPTIONS
   DriveLetter: �}�E���g����h���C�u
   ThreadCount: Dokan ���C�u���������Ŏg�p����X���b�h�̌��D0���w�肷
                ��΃f�t�H���g�l���g���܂��D�f�o�b�O���� 1 ���w�肷��
                �ƃf�o�b�O���₷���Ȃ�܂��D
   DebugMode  : 1 �ɃZ�b�g����ƁC�f�o�b�O���b�Z�[�W���f�o�b�O�o�͂ɏo��
                ����܂��D
   UseStdErr  : 1 �ɃZ�b�g����ƁC�f�o�b�O���b�Z�[�W���W���G���[�o�͂ɏo
   �@�@�@�@�@�@�͂���܂��D
   UseAltStream : ��փX�g���[��(alternate stream, ���X�g���[��)���g�p��
                  �܂��D

DokanMain �̓}�E���g�Ɏ��s����Ǝ��̂悤�ȃG���[�R�[�h��Ԃ��܂��D

    #define DOKAN_SUCCESS                0
    #define DOKAN_ERROR                 -1 /* General Error */
    #define DOKAN_DRIVE_LETTER_ERROR    -2 /* Bad Drive letter */
    #define DOKAN_DRIVER_INSTALL_ERROR  -3 /* Can't install driver */
    #define DOKAN_START_ERROR           -4 /* Driver something wrong */
    #define DOKAN_MOUNT_ERROR           -5 /* Can't assign a drive letter */



�� �A���}�E���g

DokanUnmount ���Ăׂ΃A���}�E���g�ł��܂��D�v���O�������n���O�����ꍇ��C
�G�N�X�v���[�����n���O�����ꍇ�́C�A���}�E���g���s���Α��̏ꍇ���ɖ�
��܂��D

    > dokanctl.exe /u DriveLetter

�� unmount ���s���܂��D



�� ���̂ق�

Dokan���C�u�����������́C���̃��C�u�����𗘗p���č쐬�����t�@�C���V�X�e
���̕s��̂��߂ɁC�u���[�X�N���[���ɂȂ邱�Ƃ�����܂��D�t�@�C���V�X
�e���̊J���ɂ́CVirtual Machine�𗘗p���邱�Ƃ����������߂��܂��D
