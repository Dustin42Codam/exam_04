#ifndef MICROSHELL_H
# define MICROSHELL_H

# include <stdlib.h>
# include <errno.h>
# include <unistd.h>
# include <sys/wait.h>
# include <sys/types.h>
# include <string.h>

# include <stdio.h>

# define STDIN			0
# define STDOUT			1
# define STDERR			2

typedef struct s_fd
{
	int		pipe[2];
	int		read;
	int		write;
	int		dup_stdin;
	int		dup_stdout;
	int		save_stdin;
	int		save_stdout;
}	t_fd;

typedef struct 		s_cmd
{
	char			**env;
	char			**argv;
	int				argc;
	int				end_of_pipe;
	t_fd			*fd;
	struct s_cmd	*next;
}					t_cmd;

int			ft_strlen(char *s);
char		*ft_strdup(char *s);
void		exit_shell(int error);
void		ft_close(int fd);
void		execute(t_cmd *cmd);

#endif
