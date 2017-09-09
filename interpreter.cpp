#include "interpreter.hpp"

command::command(void) : fun(NULL) { }
command::command(const command &c) : fun(c.fun) { strcpy(this->name, c.name); }
command::command(char name[], int (interpreter::*fun)(char **args)) : fun(fun) { strncpy(this->name, name, MAX_COMMAND_NAME_LEN); }

interpreter::interpreter() : commands_nbr(0)
{
    this->add_command("help", &interpreter::command_help);
    this->add_command("clear", &interpreter::command_clear);
    this->add_command("exit", &interpreter::command_exit);
    this->add_command("quit", &interpreter::command_exit);
    this->add_command("echo", &interpreter::command_echo);
    this->add_command("motd", &interpreter::command_motd);
    this->add_command("colortest", &interpreter::command_colortest);
    this->add_command("log_level", &interpreter::command_log_level);
    this->add_command("trace", &interpreter::command_trace);
    this->add_command("mem", &interpreter::command_mem);
    this->add_command("x", &interpreter::command_x);
    this->add_command("int", &interpreter::command_int);
}

int interpreter::add_command(char const name[], int (interpreter::*fun)(char **args))
{
    if (!(this->commands_nbr < MAX_COMMANDS_NUMBER))
        return 0;
    strncpy(this->commands[this->commands_nbr].name, name, MAX_COMMAND_NAME_LEN);
    this->commands[this->commands_nbr].fun = fun;
    this->commands_nbr++;
    return 1;
}

int interpreter::interpret(char *line)
{
    char *splits[MAX_LINE_LEN / 2 + 1] = {0};

    if (!*line)
        return 0;
    this->split(line, splits);
    if (!splits[0])
        return 0;
    return this->exec_cmd(splits[0], splits);
}

int interpreter::exec_cmd(char *cmd, char **args)
{
    size_t i = 0;

    while (i < this->commands_nbr)
    {
        if (!strcmp(cmd, this->commands[i].name))
        {
            return (this->*commands[i].fun)(args);
        }
        ++i;
    }
    term.printk("%12gcommand%14g %s%12g not found%r\n", cmd);
    return 1;
}

void interpreter::split(char *string, char *splits[])
{
    size_t i = 0;
    size_t splitn = 0;
    char c;
    char last = ' ';

    while (string[i])
    {
        c = string[i];
        if (is_sep(c))
            string[i] = '\0';
        if (!is_sep(c) && is_sep(last))
        {
            splits[splitn++] = string + i;
        }
        ++i;
        last = c;
    }
    splits[splitn] = 0;
}

int interpreter::command_help(char **args)
{
    size_t i = 0;

    (void)args;
    term.printk("%10gavailable commands:%r \n");
    while (i < this->commands_nbr)
    {
        term.printk("    %11g%s%r\n", this->commands[i].name);
        ++i;
    }
    return 0;
}

int interpreter::command_clear(char **args)
{
    (void)args;
    term.clear();
    return 0;
}

int interpreter::command_exit(char **args)
{
    (void)args;
    while (inb(0x64 & 1))
        inb(0x60);
    outb(0x64, 0xfe);
    return 0;
}

int interpreter::command_echo(char **args)
{
    ++args;
    while (*args)
    {
        term.tputs_noup(*args++);
        term.tputc_noup(' ');
    }
    term.tputc_noup('\n');
    term.update_cursor();
    return 0;
}

int interpreter::command_motd(char **args)
{
    (void)args;
    motd();
    return 0;
}

int interpreter::command_colortest(char **args)
{
    (void)args;
    for (u8 c = 0; c < 16; ++c)
    {
        term.set_fg((vga_color)c);
        term.printk("%2d ", c);
        term.set_bg((vga_color)c);
        term.printk("  \n", c);
        term.reset_color();
    }
    return 0;
}

int interpreter::command_log_level(char **args)
{
    ++args;
    term.printk("current log level: %8g%u%r\n", term.get_log_level());
    if (*args && term.set_log_level(**args - '0'))
    {
        term.printk("new log level: %8g%u%r\n", term.get_log_level());
    }
    return 0;
}

int interpreter::command_trace(char **args)
{
    (void)args;
    debug.trace();
    return 0;
}

int interpreter::command_mem(char **args)
{
    if (!args[1])
    {
        goto usage;
    }
    else if (!strcmp("status", args[1]))
    {
        mem.status();
    }
    else if (!strcmp("dump", args[1]))
    {
        mem.dump();
    }
    else if (!strcmp("alloc", args[1]))
    {
        u32 nbr;

        if (args[2] && (nbr = atoi(args[2])))
            mem.kheap.alloc(nbr * PAGESIZE);
        else
            term.printk("usage: mem alloc $page_nbr\n");
    }
    else if (!strcmp("free", args[1]))
    {
        u32 addr = atoi(args[2]);
        u32 nbr = atoi(args[3]);

        if (args[2] && args[3] && (addr = atoi(args[2])) && (nbr = atoi(args[3])))
            mem.kheap.free((void*)addr, nbr * PAGESIZE);
        else
            term.printk("usage: mem free $addr $page_nbr\n");
    }
    else
    {
        term.printk("unknown subcommand %s\n", args[1]);
        goto usage;
    }
    return 0;

usage:
    term.printk("usage: mem (status|dump|alloc|free)\n");
    return 1;
}

int interpreter::command_x(char **args)
{
    u32 address, rep, ptr;
    char *param;
    char fmt;

    if (!args[1])
        goto usage;
    if (args[1] && args[2] && args[1][0] == '/')
    {
        param = args[1] + 1; // skip the '/'
        if (strchr("xdiuc", param[0]))
        {
            fmt = param[0];
            ++param;
        }
        else if (is_digit(param[0]))
        {
            fmt = 'x';
        }
        else
        {
            goto usage;
        }
        if (!param[0])
            rep = 1;
        else
            rep = atoi(param);
        address = atoi(args[2]);
    }
    else
    {
        rep = 1;
        address = atoi(args[1]);
        fmt = 'x';
    }
    ptr = address;
    while (rep--)
    {
        if ((address & 0xf) == (ptr & 0xf))
            term.printk("%8x:", ptr);
        switch (fmt)
        {
            case 'd':
            case 'i':
                term.printk(" %d", *(u32*)ptr); break;
            case 'u':
                term.printk(" %u", *(u32*)ptr); break;
            case 'c':
                term.printk(" %c%c%c%c",
                        is_print(((char*)ptr)[0]) && !is_sep(((char*)ptr)[0]) ? (((char*)ptr)[0]) : ' ',
                        is_print(((char*)ptr)[1]) && !is_sep(((char*)ptr)[1]) ? (((char*)ptr)[1]) : ' ',
                        is_print(((char*)ptr)[2]) && !is_sep(((char*)ptr)[2]) ? (((char*)ptr)[2]) : ' ',
                        is_print(((char*)ptr)[3]) && !is_sep(((char*)ptr)[3]) ? (((char*)ptr)[3]) : ' ');
                break;
            case 'x':
                term.printk(" 0x%8x", *(u32*)ptr); break;
        }
        ptr += 4;
        if ((address & 0xf) == (ptr & 0xf))
            term.printk("\n");
    }
    term.printk("\n");

    return 0;

usage:
    term.printk("usage: %s (/(x|d|i|u|c)?nbr)? address\n", args[0]);
    return 1;
}

int interpreter::command_int(char **args)
{
    if (!args[1])
    {
        goto usage;
    }
    else if (!strcmp("dump", args[1]))
    {
        dump_int_summary();
    }
    else
    {
        term.printk("unknown subcommand %s\n", args[1]);
        goto usage;
    }
    return 0;

usage:
    term.printk("usage: int (dump)\n");
    return 1;
}
