#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <string>
#include <vector>
#include <unordered_set>
#include <kvm.h>
#include <dirent.h>
#include <sys/sysctl.h>
#include <sys/socket.h>
#include <sys/proc.h>
#include <sys/param.h>
#include <libprocstat.h>
#include <sys/user.h>
#include <sys/wait.h>

struct Process {
	int PID;
	int PPID;
	int PGID;
	char start_time[8];
	char status[2];
	char cpu[5];
	int UID;
	int TT;
	int VSZ;
	int RSS;
	char memory[5];
	char command[19];
	char LF;
};

class Monitor
{
public:
	Monitor();
	~Monitor();
	const std::vector<Process>& getProcesses();
private:
	void die();
	std::vector<Process> _processes;
	bool* status_ptr;
	Process* value_ptr;
	int monitor_pid;
};
