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

bool refresh = true;
bool finish = false;

struct RawProcess {
	int PID;
	std::string status;
};

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

void sighandler(int number) 
{
	refresh = true;
}
void terminator(int number) 
{
	finish = true;
}

int main() {
	close(0);
	close(2);
	int result = fork();
	if(result != 0)
		return 0;
	setsid();
	signal(SIGUSR1, sighandler);
	signal(SIGTERM, terminator);
	int fd_val, fd_data;
	void* val_ptr;
	void* data_ptr;
	fd_val = shm_open("/PS_STATUS", O_CREAT | O_RDWR, 0666);
	ftruncate(fd_val, 8);
	val_ptr = mmap(0, 8, PROT_WRITE, MAP_SHARED, fd_val, 0);
	*(bool*)val_ptr = false;
	fd_data = shm_open("/PS_VALUE", O_CREAT | O_RDWR, 0666);
	ftruncate(fd_data, 30000);
	data_ptr = mmap(0, 30000, PROT_WRITE | PROT_READ, MAP_SHARED, fd_data, 0);
	
	while(signal(SIGUSR1, sighandler) != sighandler)
	{}
	
	int res = fork();
	if(res != 0)
	{
		write(1, &res, 4);
		exit(0);
	}
	close(1);
	setsid();
	while(true)
	{
		if(finish)
		{
			shm_unlink("/PS_STATUS");
			shm_unlink("/PS_VALUE");
			exit(0);
		}
		if (refresh)
		{
		*(bool*)val_ptr = true;
		std::vector<Process> parsed;
		long long total_memory = 0;
		struct kinfo_proc kp[1000] = {};
		size_t len = sizeof(kp);
		int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_PROC};
		sysctl(mib, 3, &kp, &len, NULL, 0);
		for (int i = 0; i < 1000; i++)
		{
			struct Process buffer;
			buffer.PID = kp[i].ki_pid;
			buffer.PGID = kp[i].ki_pgid;
			memcpy(buffer.command, kp[i].ki_comm, 19);
			struct tm *start = localtime(&kp[i].ki_start.tv_sec);
			memcpy(buffer.start_time, (std::to_string(start->tm_hour) + ":" + std::to_string(start->tm_min) + ":" + std::to_string(start->tm_sec) + "0").c_str(), 8);
			buffer.TT = static_cast<int>(kp[i].ki_tdev);
			memcpy(buffer.status, std::to_string(kp[i].ki_stat).c_str(), 2);
			buffer.PPID = kp[i].ki_ppid;
			buffer.VSZ = kp[i].ki_size;
			buffer.RSS = kp[i].ki_rssize;
			total_memory += buffer.RSS;
			memcpy(buffer.cpu, std::to_string((double)kp[i].ki_pctcpu/100).c_str(), 5);
			buffer.LF = '\n';
			if(kp[i].ki_start.tv_sec != 0)
				parsed.push_back(buffer);
		}
		parsed.push_back({INT_MAX, INT_MAX, INT_MAX, "NULLNUL"});
		char* data_ptr_buff = (char*)data_ptr;
		for(int i = 0; i < parsed.size(); i++)
		{
			memcpy(parsed[i].memory, std::to_string(((double)parsed[i].RSS/(double)total_memory)*100).c_str(), 5);
			memcpy(data_ptr_buff, &parsed[i], sizeof(Process));
			data_ptr_buff+=sizeof(Process);
		}
		data_ptr_buff = (char*)data_ptr;
		refresh = false;
		*(bool*)val_ptr = false;
	}
		else
		{
		pause();
		}
}
}

