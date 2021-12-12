/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mandatory.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aysarrar <aysarrar@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/12 11:41:13 by aysarrar          #+#    #+#             */
/*   Updated: 2021/12/12 16:24:56 by aysarrar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/libft.h"
void	handle_errors(char *str)
{
	if (errno != 0)
		perror(str);
	else
		ft_putstr_fd(str, 1);
	exit(EXIT_FAILURE);
}
char	*get_path(char **envp)
{
	int		index;

	index = 0;
	while (envp[index])
	{
		if (ft_strnstr(envp[index], "PATH", ft_strlen(envp[index])))
			return (envp[index]);
		index++;
	}
	return (NULL);
}
void	execute(char *str, char *cmd, char *envp[], char **command)
{
	char	*full_cmd;
	char	*new_path;
	int		execve_r;

	new_path = ft_strjoin(str, "/");
	if (!new_path)
		handle_errors("ft_strjoin fail");
	full_cmd = ft_strjoin(new_path, cmd);
	if (!full_cmd)
		handle_errors("ft_strjoin fail");
	if ((access(full_cmd, X_OK) == 0) && (access(full_cmd, F_OK) == 0))
	{
		free(new_path);
		execve_r = execve(full_cmd, command, envp);
		if (execve_r == -1)
			handle_errors("error");
	}
	free(new_path);
	free(full_cmd);
}

int	get_path_and_execute(char **cmd, char *envp[])
{
	char	*get_env;
	int		index;
	char	**exec_path;
	char	*path;

	index = 0;
	get_env = get_path(envp);
	if (!get_env)
		handle_errors("couldnt get the path");
	path = ft_substr(get_env, 5, ft_strlen(get_env) - 5);
	if (!path)
		handle_errors("path error");
	exec_path = ft_split(path, ':');
	free(path);
	while (exec_path[index] != NULL)
	{
		execute(exec_path[index], cmd[0], envp, cmd);
		index++;
	}
	return (-1);
}
void	child_one(t_pipex *pipe_list, char **av, char *envp[])
{
	int exec;
	
	dup2(pipe_list->infile, 0);
	dup2(pipe_list->fd[1], 1);
	close(pipe_list->fd[0]);
	close(pipe_list->outfile);
	pipe_list->cmd1 = ft_split(av[2], ' ');
	exec = get_path_and_execute(pipe_list->cmd1, envp);
	if (exec == -1)
		handle_errors("command not found");
	
}

void	child_two(t_pipex *pipe_list, char **av, char *envp[])
{
	int exec;
	
	dup2(pipe_list->outfile, 1);
	dup2(pipe_list->fd[0], 0);
	close(pipe_list->fd[1]);
	close(pipe_list->outfile);
	pipe_list->cmd2 = ft_split(av[3], ' ');
	exec = get_path_and_execute(pipe_list->cmd2, envp);
	if (exec == -1)
		handle_errors("command not found");
}

void	pipex(t_pipex *pipe_list, char **av, char *envp[])
{
	pipe_list->status = 0;
	pipe(pipe_list->fd);
	pipe_list->childpid1 = fork();
	if (pipe_list->childpid1 < 0)
		return (perror("Fork: "));
	if (pipe_list->childpid1 == 0)
			child_one(pipe_list, av, envp);
	pipe_list->childpid2 = fork();
	if (pipe_list->childpid2 < 0)
		return (perror("Fork: "));
	if (pipe_list->childpid2 == 0)
		child_two(pipe_list, av, envp);
	close(pipe_list->fd[0]);
	close(pipe_list->fd[1]);
	waitpid(pipe_list->childpid1, &pipe_list->status, 0);
	waitpid(pipe_list->childpid2, &pipe_list->status, 0);
}

int main(int ac, char **av, char *envp[])
{
	if (ac == 5)
	{
    	t_pipex	*pipe_list;

	 	pipe_list = malloc(sizeof(t_pipex *));
	 	if (!pipe_list)
	 		return (-1);
		pipe_list->infile = open(av[1], O_RDONLY);
		pipe_list->outfile = open(av[4], O_CREAT | O_WRONLY | O_TRUNC, 0666);
		if (pipe_list->infile < 0 || pipe_list->outfile < 0)
			handle_errors(av[1]);
		pipex(pipe_list, av, envp);
	}
}