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

class Log {
	public:
	Log(const std::vector<std::string>& filenames){
		int last_fd, names_fd;
		char* names_ptr;
		last_fd = shm_open("/LOG_LAST", O_CREAT | O_RDWR, 0666);
		ftruncate(last_fd, 8);
		last_ptr = (int*)mmap(0, 8, PROT_READ | PROT_WRITE, MAP_SHARED, last_fd, 0);
		close(last_fd);
		names_fd = shm_open("/LOG_NAMES",  O_CREAT | O_RDWR, 0666);
		ftruncate(names_fd, 1024);
		names_ptr = (char*)mmap(0, 1024, PROT_WRITE | PROT_READ, MAP_SHARED, names_fd, 0);
		close(names_fd);
		pointers.push_back(last_ptr);
		*last_ptr = 0;
		printf("MEM INIT\n");
		for(int i = 0; i < filenames.size(); i++)
		{
			memcpy(names_ptr, filenames[i].c_str(), filenames[i].size());
			names_ptr+=filenames[i].size();
			*names_ptr = '\n';
			names_ptr++;
			
			std::string shm_name = "/LOG_" + std::to_string(i + 1);
			int shm_fd = shm_open(shm_name.c_str(), O_CREAT | O_RDWR, 0666);
			ftruncate(shm_fd, 1024);
			void* ptr = mmap(0, 1024, PROT_WRITE, MAP_SHARED, shm_fd, 0);
			close(shm_fd);
			pointers.push_back(ptr);
		}
		write(1, "NAMES\n", 6);
		*names_ptr = 0;
		int comm[2];
		pipe(comm);
		logger_pid = fork();
		
		if (logger_pid == 0)
		{
			dup2(comm[1], 1);
			close(comm[0]);
			int exec_res = execlp("./logger.exe", "logger.exe", 0);
		}
		close(comm[1]);
		write(1, "FORK1\n", 6);
		write(1, "OUT\n", 4);
		char buff[5];
		int result = read(comm[0], buff, 5);
		write(1, buff, 5);
		write(1, "\n", 1);
	}
	void write_message(const std::string& message, int log_num) {
		while(__sync_val_compare_and_swap(last_ptr, 0, log_num) != 0)
		{}
		write(1, "VALCHG\n", 7);
		memcpy((char*)pointers[log_num], message.c_str(), message.size());
		*((char*)pointers[log_num] + message.size()) = 0;
		kill(logger_pid, SIGUSR1);
		}
	~Log() {
		kill(logger_pid, SIGTERM);
		int ret;
		wait(&ret);
	}
	private:
	int* last_ptr;
	std::vector<void*> pointers;
	int logger_pid;
};

int main()
{
	std::vector<std::string> files_list;
	files_list.push_back("Testlog1");
	files_list.push_back("Testlog2");
	files_list.push_back("Testlog3");
	files_list.push_back("Testlog4");
	Log logger(files_list);
	logger.write_message("Testing log number 1\n", 1);
	logger.write_message("Testing log number 2\n", 2);
	logger.write_message("Testing log number 3\n", 3);
	logger.write_message("Testing log number 4\n", 4);
	logger.write_message("Other Testing log number 1 again\n", 1);
	logger.write_message("Other Testing log number 2 again\n", 2);
	logger.write_message("Other Testing log number 3 again\n", 3);
	logger.write_message("Other Testing log number 4 again\n", 4);
	
}
