#include "terminal.hpp"

size_t terminal::putint_b(int nbr, unsigned int base, int padding)
{
    unsigned int nnbr;
    size_t w = 0;

    if (nbr < 0)
    {
        this->tputc_noup('-');
        nnbr = -nbr;
        ++w;
    }
    else
    {
        nnbr = nbr;
    }
    return w + this->putuint_b(nnbr, base, padding);
}

template <typename T>
size_t terminal::putuint_b(T nbr, unsigned int base, int padding)
{
    T tmp;
    T l;
    size_t w = 0;

    tmp = nbr;
    l = 1;
    while (tmp >= (T)base)
    {
        tmp /= base;
        l *= base;
        padding--;
    }
    while (--padding > 0)
    {
        tputc_noup('0');
        ++w;
    }
    while (l)
    {
        this->tputc_noup("0123456789abcdef"[nbr / l]);
        ++w;
        nbr = nbr % l;
        l /= base;
    }
    return w;
}

size_t terminal::printk(const char *format, ...)
{
    size_t w;
    va_list params;

    if (format[0] == *LOG_HDR && format[1] - '0' > this->log_level)
        return 0;
    va_start(params, format);
    w = vprintk(format, params);
    va_end(params);
    this->update_vga_buffer();
    return w;
}

size_t terminal::printk_noup(const char *format, ...)
{
    size_t w;
    va_list params;

    if (format[0] == *LOG_HDR && format[1] - '0' > this->log_level)
        return 0;
    va_start(params, format);
    w = vprintk(format, params);
    va_end(params);
    return w;
}

size_t terminal::vprintk(const char *format, va_list params)
{
    size_t w = 0;
    u8 param;

    if (*format == *LOG_HDR)
    {
        ++format;
        if (*format - '0' <= this->log_level)
        {
            this->printk_noup(terminal::klog[*format - '0']);
        }
        ++format;
    }
    while (*format)
    {
        if (*format == '%')
        {
            ++format;
            param = 0;
            while (*format && *format >= '0' && *format <= '9')
            {
                param = (param * 10) + *format - '0';
                ++format;
            }
            switch(*format)
            {
                case '%':
                    this->tputc_noup('%');
                    ++w;
                    break;
                case 's':
                    if (param)
                        w += this->tputs_noup(va_arg(params, char*), param);
                    else
                        w += this->tputs_noup(va_arg(params, char*));
                    break;
                case 'U': // 64bit variables get the val 4 bytes by 4
                    w += this->putuint_b(((u64)va_arg(params, unsigned int) << 32) + va_arg(params, unsigned int), 10, param);
                    break;
                case 'u':
                    w += this->putuint_b(va_arg(params, unsigned int), 10, param);
                    break;
                case 'i':
                case 'd':
                    w += this->putint_b(va_arg(params, unsigned int), 10, param);
                    break;
                case 'p':
                    this->tputs_noup("0x");
                    w += 2;
                case 'x':
                    w += this->putuint_b(va_arg(params, unsigned int), 16, param);
                    break;
                case 'b':
                    w += this->putuint_b(va_arg(params, unsigned int), 2, param);
                    break;
                case 'c':
                    this->tputc_noup(va_arg(params, unsigned int));
                    ++w;
                    break;
                case 'g': // set fg color
                    this->set_color(param | (this->color & 0xf0));
                    break;
                case 'G': // set bg color
                    this->set_color((this->color & 0xf) | param << 4);
                    break;
                case 'r': // reset colors
                    this->color = this->def_color;
                    break;
                default:
                    break;
            }
        }
        else
        {
            this->tputc_noup(*format);
            ++w;
        }
        ++format;
    }
    return w;
}
