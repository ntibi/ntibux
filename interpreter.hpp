#include "kernel.hpp"
#include "terminal.hpp"

#ifndef INTERPRETER_HPP
# define INTERPRETER_HPP

#define MAX_COMMAND_NAME_LEN 32
#define MAX_COMMANDS_NUMBER 64

class terminal;
class interpreter;

class command
{
public:
    command(void);
    command(const command &c);
    command(char name[], int (interpreter::*fun)(char **));
    char name[MAX_COMMAND_NAME_LEN];
    int (interpreter::*fun)(char **args);
};

class interpreter
{
public:
    interpreter();
    int interpret(char *line);
    int add_command(char const name[], int (interpreter::*fun)(char **args));
private:
    int exec_cmd(char *cmd, char **args);
    void split(char *string, char *splits[]);
    command commands[MAX_COMMANDS_NUMBER];
    size_t commands_nbr;

    int command_help(char **args);
    int command_clear(char **args);
    int command_exit(char **args);
    int command_echo(char **args);
    int command_motd(char **args);
    int command_colortest(char **args);
    int command_log_level(char **args);
    int command_trace(char **args);
    int command_mem(char **args);
    int command_x(char **args);
    int command_int(char **args);
    int command_time(char **args);
    int command_sched(char **args);
    int command_cpus(char **args);
};

#endif
