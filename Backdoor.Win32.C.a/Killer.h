#define MASBUF 4096
#define ARRAYSIZE 4096
#define GIABUF 2048
#define BIGBUF 1024
#define IRCBUF 512
#define MEDBUF 256
#define LOWBUF 128
#define FLOOD_DELAY 1000

char *stristr(const char *str, const char *strSearch);
bool file_exists(char *pszFilePath);
bool file_delete(char *pszFilePath);
void *memmem(const void *buf, const void *pattern, size_t buflen, size_t len);
bool process_killpid(DWORD dwPID);
DWORD WINAPI botkiller_main(LPVOID param);
bool botkiller_memscan(DWORD dwPID, char *pszBuffer, DWORD dwSize);
int botkiller_removebot(char *pszFileName, LPVOID param);
