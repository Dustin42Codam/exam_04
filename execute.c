#include "microshell.h"

static void	restore_fd(t_cmd *cmd)
{
	if (dup2(cmd->fd->save_stdin, STDIN) == -1)
		exit_shell(errno);
	if (dup2(cmd->fd->save_stdout, STDOUT) == -1)
		exit_shell(errno);
	if (cmd->fd->pipe[0])
		ft_close(cmd->fd->pipe[0]);
	if (cmd->fd->pipe[1])
		ft_close(cmd->fd->pipe[1]);
	cmd->fd->pipe[0] = 0;
	cmd->fd->pipe[1] = 0;
	cmd->fd->dup_stdin = 0;
	cmd->fd->dup_stdout = 0;
	cmd->fd->write = STDOUT;
	cmd->fd->read = STDIN;
}

static void	execute_child(t_cmd *cmd)
{
	errno = 0;
	if (cmd->fd->dup_stdin)
		dup2(cmd->fd->read, STDIN);
	if (cmd->fd->dup_stdout)
		dup2(cmd->fd->write, STDOUT);
	execve(cmd->argv[0], cmd->argv, cmd->env);
	write(STDERR, "error: cannot execute ", 22);
	write(STDERR, cmd->argv[0], ft_strlen(cmd->argv[0]));
	write(STDERR, "\n", 1);
	exit(errno);
}

void	execute_cd(t_cmd *cmd)
{
	if (cmd->argc != 2)
	{
		write(STDERR, "error: cd: bad arguments\n", 25);
		return ;
	}
	if (chdir(cmd->argv[1]) == -1)
	{
		write(STDERR, "error: cd: cannot change directory to ", 38);
		write(STDERR, cmd->argv[1], ft_strlen(cmd->argv[1]));
		write(STDERR, "\n", 1);
	}
}

static void	execute_command(t_cmd *cmd)
{
	pid_t	pid;

	if (!strncmp(cmd->argv[0], "cd", 3) && cmd->end_of_pipe)
		return (execute_cd(cmd));
	pid = fork();
	if (pid == -1)
		exit_shell(errno);
	else if (pid == 0)
		execute_child(cmd);
	waitpid(pid, NULL, 0);
}

static void	setup_next_pipe(t_cmd *cmd)
{
	ft_close(cmd->fd->write);
	cmd->fd->read = cmd->fd->pipe[0];
	if (pipe(cmd->fd->pipe) == -1)
		exit_shell(errno);
	cmd->fd->write = cmd->fd->pipe[1];
	cmd->fd->dup_stdout = 1;
	cmd->fd->dup_stdin = 1;
}

static void	setup_pipe_end(t_cmd *cmd)
{
	ft_close(cmd->fd->write);
	cmd->fd->read = cmd->fd->pipe[0];
	cmd->fd->dup_stdin = 1;
	cmd->fd->dup_stdout = 0;
}

static t_cmd *execute_pipeline(t_cmd *cmd)
{
	if (pipe(cmd->fd->pipe) == -1)
		exit_shell(errno);
	cmd->fd->dup_stdin = 0;
	cmd->fd->dup_stdout = 1;
	cmd->fd->write = cmd->fd->pipe[1];
	execute_command(cmd);
	cmd = cmd->next;
	while (cmd->end_of_pipe == 0)
	{
		setup_next_pipe(cmd);
		execute_command(cmd);
		ft_close(cmd->fd->read);
		cmd = cmd->next;
	}
	setup_pipe_end(cmd);
	execute_command(cmd);
	ft_close(cmd->fd->pipe[0]);
	return (cmd);
}

void	execute(t_cmd *cmd)
{
	if (cmd == NULL)
		return ;
	if (cmd->end_of_pipe == 0)
		cmd = execute_pipeline(cmd);
	else
		execute_command(cmd);
	restore_fd(cmd);
	return (execute(cmd->next));
}
