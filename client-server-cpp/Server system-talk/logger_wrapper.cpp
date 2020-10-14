#include "logger_wrapper.hpp"

Log::Log(const std::vector<std::string>& filenames){
		good = true;
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
		*names_ptr = 0;
		int comm[2];
		pipe(comm);
		logger_pid = fork();
		
		if (logger_pid == 0)
		{
			dup2(comm[1], 1);
			close(comm[0]);
			int exec_res = execlp("./logger", "logger", nullptr);
			if (exec_res != 0)
				{
					die();
					exit(-1);
					return;
				}
		}
		close(comm[1]);
		int ret_res;
		if (wait(&ret_res) == -1)
			{
				die();
				return;
			}
		if (WIFEXITED(ret_res) != 0)
			if (WEXITSTATUS(ret_res)!= 0)
			{
			die();
			return;
		}
		read(comm[0], &logger_pid, 4);
	}
	int Log::write_message(const std::string& message, int log_num) {
		if(!good)
			return -1;
		while(__sync_val_compare_and_swap(last_ptr, 0, log_num) != 0)
		{}
		memcpy((char*)pointers[log_num], message.c_str(), message.size());
		*((char*)pointers[log_num] + message.size()) = 0;
		kill(logger_pid, SIGUSR1);
		return 0;
		}
	void Log::die() {
		good = false;
		munmap(last_ptr, 8);
		for (int i = 1; i < pointers.size(); i++)
			munmap(pointers[i], 1024);
		}
	Log::~Log() {
		if (good)
		{
		kill(logger_pid, SIGTERM);
		die();
		int ret;
		wait(&ret);
	}
	}

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
