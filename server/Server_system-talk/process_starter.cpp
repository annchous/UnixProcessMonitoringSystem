#include "process_starter.hpp"

		int Process::run(bool need_wait, int UID, const std::string& command,
		const std::vector<std::string>& arguments, int *stdout_fd, int *err_fd)
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
				close(1);
				close(2);
				close(0);
				close(pipe_fd[0]);
				close(pipe_return[0]);
				close(pipe_err[0]);
				dup2(pipe_fd[1], 1);
				dup2(pipe_err[1], 2);
				
				write(2, " ", 1);
				int grand_child_pid = fork();
				if(grand_child_pid == 0)
				{
					setuid(UID);
					char* argv[10];
					argv[0] = (char*)command.c_str();
					for (int i = 1; i < 10; i++)
					{
						if (i < arguments.size())
							argv[i] = (char*)arguments[i].c_str();
						else
							argv[i] = nullptr;
					}
					int result = execvp(command.c_str(), argv);
					if(result != 0)
					{
						exit(-1);
					}
				}
				int code;
				close(1);
				if(need_wait)
				{
					wait(&code);
					code = WEXITSTATUS(code);
					write(pipe_return[1], &code, 8);
				}
				exit(0);
			}
			int code;
			close(pipe_fd[1]);
			close(pipe_return[1]);
			close(pipe_err[1]);
			if(need_wait)
			{
				read(pipe_return[0], &code, 8);
			}
			*stdout_fd = pipe_fd[0];
			*err_fd = pipe_err[0];
			if (need_wait)
				return code;
			else
				return 0;
		}

/*
int main () {
	Process test;
	int fd, err;
	std::vector<std::string> argv;
	//argv.push_back("grep");
	//argv.push_back("-r");
	argv.push_back("student");
	argv.push_back("/home");
	int result = test.run(true, 177, "grep", argv, &fd, &err);
	char buffer_out[100] = {};
	char buffer_err[100] = {};
	std::string out, erro, buff;
	while (read(fd, buffer_out, 100) != 0)
	{
		buff = buffer_out;
		out += buff;
		read(fd, buffer_out, 100);
	}
	while(read(err, buffer_err, 100) != 0)
	{
		buff = buffer_err;
		erro += buff;
		read(err, buffer_out, 100);
	}
	//printf("%d %.100s\n", result,  buffer);
	printf("ERR: %s\nSTDOUT: %s\n", erro.c_str(), out.c_str());
	printf("Exit code: %d\n", result);
	return result;
}
*/
