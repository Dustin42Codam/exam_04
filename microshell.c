#include "microshell.h"

int	ft_strlen(char *s)
{
	int	i = 0;

	if (!s)
		return (0);
	while (s[i])
		i++;
	return (i);
}


void	exit_shell(int error)
{
	write(STDERR, "error: fatal\n", 13);
	exit(error);
}

char	*ft_strdup(char *s)
{
	char	*dup;
	int		len;

	if (!s)
		return (NULL);
	len = ft_strlen(s);
	dup = malloc(len + 1);
	if (!dup)
		exit_shell(errno);
	dup[len] = 0;
	for (size_t i = 0; s[i]; i++)
		dup[i] = s[i];
	return (dup);
}

void	ft_close(int fd)
{
	if (close(fd) == -1 && errno && errno != EBADF)
		exit_shell(errno);
}

void	ft_lstadd_back(t_cmd **head, t_cmd *new)
{
	t_cmd	*last;

	if (*head == NULL)
	{
		*head = new;
		return ;
	}
	last = *head;
	while (last->next)
		last = last->next;
	last->next = new;
}

t_cmd	*parser(int argc, char **argv, char **envp, t_fd *fd)
{
	t_cmd	*head = NULL;
	t_cmd	*new = NULL;
	int	words = 0;

	for (int i = 1; i <= argc; i++)
	{
		if (i == argc || !strncmp(argv[i], "|", 2) || !strncmp(argv[i], ";", 2))
		{
			new = malloc(sizeof(t_cmd));
			if (!new)
				exit_shell(errno);
			new->next = NULL;
			new->env = envp;
			new->fd = fd;
			new->argv = malloc(sizeof(char *) * (words + 1));
			if (!new->argv)
				exit_shell(errno);
			int	dst_index = 0;
			for (int src_index = i - words; src_index < i; src_index++)
			{
				new->argv[dst_index] = ft_strdup(argv[src_index]);
				dst_index++;
			}
			new->argv[dst_index] = 0;
			new->argc = words;
			if (i == argc || (argv[i] && !strncmp(argv[i], ";", 2)))
				new->end_of_pipe = 1;
			else
				new->end_of_pipe = 0;
			ft_lstadd_back(&head, new);
			words = 0;
		}
		else
			words++;
	}
	return (head);
}

void	cleanup(t_cmd *head)
{
	t_cmd	*tmp;
	t_cmd	*save;

	if (head == NULL)
		return ;
	ft_close(head->fd->save_stdin);
	ft_close(head->fd->save_stdout);
	tmp = head;
	while (tmp)
	{
		save = tmp->next;
		for (int i = 0; tmp->argv[i]; i++)
			free(tmp->argv[i]);
		free(tmp->argv);
		free(tmp);
		tmp = save;
	}
}

void	setup(t_fd *fd)
{
	fd->read = STDIN;
	fd->write = STDOUT;
	fd->save_stdin = dup(STDIN);
	if (fd->save_stdin == -1)
		exit_shell(errno);
	fd->save_stdout = dup(STDOUT);
	if (fd->save_stdout == -1)
		exit_shell(errno);
	fd->pipe[0] = 0;
	fd->pipe[1] = 0;
	fd->dup_stdin = 0;
	fd->dup_stdout = 0;
}

int	main(int argc, char **argv, char **envp)
{
	t_fd	fd;
	t_cmd	*cmd;

	if (argc > 1)
	{
		setup(&fd);
		cmd = parser(argc, argv, envp, &fd);
		execute(cmd);
		cleanup(cmd);
	}
	return (0);
}
