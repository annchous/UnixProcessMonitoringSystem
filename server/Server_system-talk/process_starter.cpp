#include <unistd.h>
#include <string>
#include <stdio.h>
#include <sys/wait.h>

class Process {
	public:
		int run(bool need_wait, int UID, std::string command, char* argv[], int *stdout_fd, int *err_fd)
		{
			int pipe_fd[2];
			int pipe_err[2];
			int pipe_return[2];
			pipe(pipe_fd);
			pipe(pipe_return);
			pipe(pipe_err);
			int child_pid = fork();
			if (child_pid == 0)
			{

				dup2(pipe_fd[1], 1);
				dup2(pipe_err[1], 2);
				close(pipe_fd[0]);
				close(pipe_return[0]);
				close(pipe_err[0]);
				write(2, " ", 1);
				int grand_child_pid = fork();
				if(grand_child_pid == 0)
				{
					setuid(UID);
					int result = execvp(command.c_str(), argv);
					if(result != 0)
					{
						perror("exec");
						return -1;
					}
				}
				int code;
				if(need_wait)
				{
					wait(&code);
					write(pipe_return[1], &code, 8);
				}
			}
			int code;
			close(pipe_fd[1]);
			close(pipe_return[1]);
			if(need_wait)
			{
				read(pipe_return[0], &code, 8);
				code = code >> 8;
			}
			*stdout_fd = pipe_fd[0];
			*err_fd = pipe_err[0];
			if (need_wait)
			return code;
			else
			return 0;
		}
		
};

int main () {
	Process test;
	int fd, err;
	char *argv[] = {"logger.exe", 0}; 
	//printf("%s, %s", argv[0], argv[1]);
	int result = test.run(false, 177, "./logger.exe", argv, &fd, &err);
	char buffer_out[100] = {};
	char buffer_err[100] = {};
	read(fd, buffer_out, 100);
	read(err, buffer_err, 100);
	//printf("%d %.100s\n", result,  buffer);
	printf("ERR: %s\nSTDOUT: %s\n", buffer_err, buffer_err);
	printf("Exit code: %d\n", result);
	return result;
}
