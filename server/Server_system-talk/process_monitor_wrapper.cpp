#include "process_monitor_wrapper.hpp"

Monitor::Monitor() {
	int value_fd, status_fd;
	status_fd = shm_open("/PS_STATUS", O_CREAT | O_RDWR, 0666);
	ftruncate(status_fd, 8);
	status_ptr = (bool*)mmap(0, 8, PROT_READ | PROT_WRITE, MAP_SHARED, status_fd, 0);
	close(status_fd);
	value_fd = shm_open("/PS_VALUE", O_CREAT | O_RDWR, 0666);
	ftruncate(value_fd, 30000);
	value_ptr = (Process*)mmap(0, 30000, PROT_READ, MAP_SHARED, value_fd, 0);
	close(value_fd);
	
	int comm[2];
	pipe(comm);
	signal(SIGUSR1, SIG_IGN);
	monitor_pid = fork();
	if(monitor_pid == 0)
	{
		dup2(comm[1], 1);
		close(comm[0]);
		int exec_res = execlp("./process_monitor", "process_monitor", nullptr);
		if (exec_res != 0)
		{
			die();
			exit(-1);
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
	read(comm[0], &monitor_pid, 4);
}

const std::vector<Process>& Monitor::getProcesses() {
	_processes.clear();
	*status_ptr = true;
	kill(monitor_pid, SIGUSR1);
	while(*status_ptr)
	{
	}
	for(int i = 0; i < 1000; i++)
	{
		if((value_ptr[i].start_time[0] == 0) || (value_ptr[i].start_time[0] == 'N'))
			break;
		_processes.push_back(value_ptr[i]);
	}
	return _processes;
}

Monitor::~Monitor()
{
	die();
	kill(monitor_pid, SIGTERM);
}

void Monitor::die() {
	munmap(value_ptr, 30000);
	munmap(status_ptr, 8);
}

/*
int main() {
	Monitor test;
	std::vector<Process> test_data = test.getProcesses();
	for (int i = 0; i < test_data.size(); i++)
	{
		printf("%d\t%.6s\t%.5s\t%.16s\n", test_data[i].PID, test_data[i].start_time, test_data[i].cpu, test_data[i].command);
	}
}
*/
