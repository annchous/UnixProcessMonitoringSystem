#include <unistd.h>
#include <string>
#include <stdio.h>
#include <sys/wait.h>
#include <vector>

class Process {
	public:
	int run(bool need_wait, int UID, const std::string& command,
		const std::vector<std::string>& arguments, int *stdout_fd, int *err_fd);
	private:
	
};
