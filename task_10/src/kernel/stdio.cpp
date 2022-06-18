#include "stdio.h"
#include "os_type.h"
#include "asm_utils.h"
#include "os_modules.h"
#include "stdarg.h"
#include "stdlib.h"

STDIO::STDIO()
{
    initialize();
}

void STDIO::initialize()
{
    screen = (uint8 *)0xb8000;
}

void STDIO::print(uint x, uint y, uint8 c, uint8 color)
{

    if (x >= 25 || y >= 80)
    {
        return;
    }

    uint pos = x * 80 + y;
    screen[2 * pos] = c;
    screen[2 * pos + 1] = color;
}

void STDIO::print(uint8 c, uint8 color)
{
    uint cursor = getCursor();
    screen[2 * cursor] = c;
    screen[2 * cursor + 1] = color;
    cursor++;
    if (cursor == 25 * 80)
    {
        rollUp();
        cursor = 24 * 80;
    }
    moveCursor(cursor);
}

void STDIO::print(uint8 c)
{
    print(c, 0x07);
}

void STDIO::moveCursor(uint position)
{
    if (position >= 80 * 25)
    {
        return;
    }

    uint8 temp;

    // 处理高8位
    temp = (position >> 8) & 0xff;
    asm_out_port(0x3d4, 0x0e);
    asm_out_port(0x3d5, temp);

    // 处理低8位
    temp = position & 0xff;
    asm_out_port(0x3d4, 0x0f);
    asm_out_port(0x3d5, temp);
}

uint STDIO::getCursor()
{
    uint pos;
    uint8 temp;

    pos = 0;
    temp = 0;
    // 处理高8位
    asm_out_port(0x3d4, 0x0e);
    asm_in_port(0x3d5, &temp);
    pos = ((uint)temp) << 8;

    // 处理低8位
    asm_out_port(0x3d4, 0x0f);
    asm_in_port(0x3d5, &temp);
    pos = pos | ((uint)temp);

    return pos;
}

void STDIO::moveCursor(uint x, uint y)
{
    if (x >= 25 || y >= 80)
    {
        return;
    }

    moveCursor(x * 80 + y);
}

void STDIO::rollUp()
{
    uint length;
    length = 25 * 80;
    for (uint i = 80; i < length; ++i)
    {
        screen[2 * (i - 80)] = screen[2 * i];
        screen[2 * (i - 80) + 1] = screen[2 * i + 1];
    }

    for (uint i = 24 * 80; i < length; ++i)
    {
        screen[2 * i] = ' ';
        screen[2 * i + 1] = 0x07;
    }
}

int STDIO::print(const char *const str, int color)
{
    int i = 0;

    for (i = 0; str[i]; ++i)
    {
        switch (str[i])
        {
        case '\n':
            uint row;
            row = getCursor() / 80;
            if (row == 24)
            {
                rollUp();
            }
            else
            {
                ++row;
            }
            moveCursor(row * 80);
            break;

        default:
            print(str[i], color);
            break;
        }
    }

    return i;
}

int printf_add_to_buffer(char *buffer, char c, int &idx, const int BUF_LEN, int color = 0x07)
{
    int counter = 0;

    buffer[idx] = c;
    ++idx;

    if (idx == BUF_LEN)
    {
        buffer[idx] = '\0';
        counter = write(buffer,color);
        idx = 0;
    }

    return counter;
}

int printf(const char *const fmt, ...)
{
    const int BUF_LEN = 32;

    char buffer[BUF_LEN + 1];
    char number[33];

    int idx, counter;
    va_list ap;

    va_start(ap, fmt);
    idx = 0;
    counter = 0;

    int temp, dict;// 为了进制转换定义
    for (int i = 0; fmt[i]; ++i)
    {
        if (fmt[i] != '%')
        {
            counter += printf_add_to_buffer(buffer, fmt[i], idx, BUF_LEN);
        }
        else
        {
            i++;
            if (fmt[i] == '\0')
            {
                break;
            }

            switch (fmt[i])
            {
            case '%':
                counter += printf_add_to_buffer(buffer, fmt[i], idx, BUF_LEN);
                break;

            case 'c':
                counter += printf_add_to_buffer(buffer, va_arg(ap, char), idx, BUF_LEN);
                break;

            case 's':
                buffer[idx] = '\0';
                idx = 0;
                counter += write(buffer);
                counter += write(va_arg(ap, const char *));
                break;

            case 'd':
            case 'x':
            case 'o':
            case 'b':
                temp = va_arg(ap, int);

                if (temp < 0 && fmt[i] == 'd')
                {
                    counter += printf_add_to_buffer(buffer, '-', idx, BUF_LEN);
                    temp = -temp;
                }
                dict = 0;
                switch (fmt[i])
                {
                case 'd':
                    dict = 10;
                    break;
                case 'x':
                    dict = 16;
                    break;
                case 'o':
                    dict = 8;
                    break;
                case 'b':
                    dict = 2;
                    break;
                default:
                    dict = 10;
                    break;
                }
                itos(number, temp, dict);

                for (int j = 0; number[j]; ++j)
                {
                    counter += printf_add_to_buffer(buffer, number[j], idx, BUF_LEN);
                }
                break;


            case 'f':
                double flt = va_arg(ap, double);
                char to_string[BUF_LEN + 1] = {0};
                int len_buffer = 0;
                if(flt < 0)
                {
                    to_string[len_buffer++] = '-';
                    flt = -flt;
                }
                int tmp = (int)flt;
                // printf("[DEBUG]%d\n", tmp);
                while (tmp)
                {
                    to_string[len_buffer++] = tmp % 10 + '0';
                    tmp /= 10;
                }
                for(int i = 0; i < (len_buffer / 2); ++i)
                    swap(to_string[len_buffer - i - 1], to_string[i]);
                to_string[len_buffer ++] = '.';
                double ftmp = flt - (int)flt;
                ftmp *= 10;
                for (int i = 0; i < 6; i++)
                {
                    to_string[len_buffer++] = (int)ftmp + '0';
                    ftmp = (ftmp - (int)ftmp) * 10;
                }
                buffer[idx] = '\0';
                idx = 0;
                counter += write(buffer);
                counter += write(to_string);
                break;
            }
        }
    }

    buffer[idx] = '\0';
    counter += write(buffer);

    return counter;
}




/*Define Printf for Log*/
int printf_log(const char *const fmt, ...)
{
#ifdef LOG
    const int BUF_LEN = 32;

    char buffer[BUF_LEN + 1];
    char number[33];

    int idx, counter;
    va_list ap;

    va_start(ap, fmt);
    idx = 0;
    counter = 0;

    int temp, dict;// 为了进制转换定义
    for (int i = 0; fmt[i]; ++i)
    {
        if (fmt[i] != '%')
        {
            counter += printf_add_to_buffer(buffer, fmt[i], idx, BUF_LEN, 0x07);
        }
        else
        {
            i++;
            if (fmt[i] == '\0')
            {
                break;
            }

            switch (fmt[i])
            {
            case '%':
                counter += printf_add_to_buffer(buffer, fmt[i], idx, BUF_LEN, 0x07);
                break;

            case 'c':
                counter += printf_add_to_buffer(buffer, va_arg(ap, char), idx, BUF_LEN, 0x07);
                break;

            case 's':
                buffer[idx] = '\0';
                idx = 0;
                counter += write(buffer, 0x07);
                counter += write(va_arg(ap, const char *), 0x07);
                break;

            case 'd':
            case 'x':
            case 'o':
            case 'b':
                temp = va_arg(ap, int);

                if (temp < 0 && fmt[i] == 'd')
                {
                    counter += printf_add_to_buffer(buffer, '-', idx, BUF_LEN, 0x07);
                    temp = -temp;
                }
                dict = 0;
                switch (fmt[i])
                {
                case 'd':
                    dict = 10;
                    break;
                case 'x':
                    dict = 16;
                    break;
                case 'o':
                    dict = 8;
                    break;
                case 'b':
                    dict = 2;
                    break;
                default:
                    dict = 10;
                    break;
                }
                itos(number, temp, dict);

                for (int j = 0; number[j]; ++j)
                {
                    counter += printf_add_to_buffer(buffer, number[j], idx, BUF_LEN, 0x07);
                }
                break;


            case 'f':
                double flt = va_arg(ap, double);
                char to_string[BUF_LEN + 1] = {0};
                int len_buffer = 0;
                if(flt < 0)
                {
                    to_string[len_buffer++] = '-';
                    flt = -flt;
                }
                int tmp = (int)flt;
                // printf("[DEBUG]%d\n", tmp);
                while (tmp)
                {
                    to_string[len_buffer++] = tmp % 10 + '0';
                    tmp /= 10;
                }
                for(int i = 0; i < (len_buffer / 2); ++i)
                    swap(to_string[len_buffer - i - 1], to_string[i]);
                to_string[len_buffer ++] = '.';
                double ftmp = flt - (int)flt;
                ftmp *= 10;
                for (int i = 0; i < 6; i++)
                {
                    to_string[len_buffer++] = (int)ftmp + '0';
                    ftmp = (ftmp - (int)ftmp) * 10;
                }
                buffer[idx] = '\0';
                idx = 0;
                counter += write(buffer, 0x07);
                counter += write(to_string, 0x07);
                break;
            }
        }
    }

    buffer[idx] = '\0';
    counter += write(buffer, 0x07);

    return counter;
#endif
}

int printf_warning(const char *const fmt, ...)
{
#if (defined LOG) || (defined WARNING)
    const int BUF_LEN = 32;

    char buffer[BUF_LEN + 1];
    char number[33];

    int idx, counter;
    va_list ap;

    va_start(ap, fmt);
    idx = 0;
    counter = 0;

    int temp, dict;// 为了进制转换定义
    for (int i = 0; fmt[i]; ++i)
    {
        if (fmt[i] != '%')
        {
            counter += printf_add_to_buffer(buffer, fmt[i], idx, BUF_LEN, 0x03);
        }
        else
        {
            i++;
            if (fmt[i] == '\0')
            {
                break;
            }

            switch (fmt[i])
            {
            case '%':
                counter += printf_add_to_buffer(buffer, fmt[i], idx, BUF_LEN, 0x03);
                break;

            case 'c':
                counter += printf_add_to_buffer(buffer, va_arg(ap, char), idx, BUF_LEN, 0x03);
                break;

            case 's':
                buffer[idx] = '\0';
                idx = 0;
                counter += write(buffer, 0x03);
                counter += write(va_arg(ap, const char *), 0x03);
                break;

            case 'd':
            case 'x':
            case 'o':
            case 'b':
                temp = va_arg(ap, int);

                if (temp < 0 && fmt[i] == 'd')
                {
                    counter += printf_add_to_buffer(buffer, '-', idx, BUF_LEN, 0x03);
                    temp = -temp;
                }
                dict = 0;
                switch (fmt[i])
                {
                case 'd':
                    dict = 10;
                    break;
                case 'x':
                    dict = 16;
                    break;
                case 'o':
                    dict = 8;
                    break;
                case 'b':
                    dict = 2;
                    break;
                default:
                    dict = 10;
                    break;
                }
                itos(number, temp, dict);

                for (int j = 0; number[j]; ++j)
                {
                    counter += printf_add_to_buffer(buffer, number[j], idx, BUF_LEN, 0x03);
                }
                break;


            case 'f':
                double flt = va_arg(ap, double);
                char to_string[BUF_LEN + 1] = {0};
                int len_buffer = 0;
                if(flt < 0)
                {
                    to_string[len_buffer++] = '-';
                    flt = -flt;
                }
                int tmp = (int)flt;
                // printf("[DEBUG]%d\n", tmp);
                while (tmp)
                {
                    to_string[len_buffer++] = tmp % 10 + '0';
                    tmp /= 10;
                }
                for(int i = 0; i < (len_buffer / 2); ++i)
                    swap(to_string[len_buffer - i - 1], to_string[i]);
                to_string[len_buffer ++] = '.';
                double ftmp = flt - (int)flt;
                ftmp *= 10;
                for (int i = 0; i < 6; i++)
                {
                    to_string[len_buffer++] = (int)ftmp + '0';
                    ftmp = (ftmp - (int)ftmp) * 10;
                }
                buffer[idx] = '\0';
                idx = 0;
                counter += write(buffer, 0x03);
                counter += write(to_string, 0x03);
                break;
            }
        }
    }

    buffer[idx] = '\0';
    counter += write(buffer, 0x03);

    return counter;
#endif
}


int printf_error(const char *const fmt, ...)
{
    const int BUF_LEN = 32;

    char buffer[BUF_LEN + 1];
    char number[33];

    int idx, counter;
    va_list ap;

    va_start(ap, fmt);
    idx = 0;
    counter = 0;

    int temp, dict;// 为了进制转换定义
    for (int i = 0; fmt[i]; ++i)
    {
        if (fmt[i] != '%')
        {
            counter += printf_add_to_buffer(buffer, fmt[i], idx, BUF_LEN, 0x47);
        }
        else
        {
            i++;
            if (fmt[i] == '\0')
            {
                break;
            }

            switch (fmt[i])
            {
            case '%':
                counter += printf_add_to_buffer(buffer, fmt[i], idx, BUF_LEN, 0x47);
                break;

            case 'c':
                counter += printf_add_to_buffer(buffer, va_arg(ap, char), idx, BUF_LEN, 0x47);
                break;

            case 's':
                buffer[idx] = '\0';
                idx = 0;
                counter += write(buffer, 0x47);
                counter += write(va_arg(ap, const char *), 0x47);
                break;

            case 'd':
            case 'x':
            case 'o':
            case 'b':
                temp = va_arg(ap, int);

                if (temp < 0 && fmt[i] == 'd')
                {
                    counter += printf_add_to_buffer(buffer, '-', idx, BUF_LEN, 0x47);
                    temp = -temp;
                }
                dict = 0;
                switch (fmt[i])
                {
                case 'd':
                    dict = 10;
                    break;
                case 'x':
                    dict = 16;
                    break;
                case 'o':
                    dict = 8;
                    break;
                case 'b':
                    dict = 2;
                    break;
                default:
                    dict = 10;
                    break;
                }
                itos(number, temp, dict);

                for (int j = 0; number[j]; ++j)
                {
                    counter += printf_add_to_buffer(buffer, number[j], idx, BUF_LEN, 0x47);
                }
                break;


            case 'f':
                double flt = va_arg(ap, double);
                char to_string[BUF_LEN + 1] = {0};
                int len_buffer = 0;
                if(flt < 0)
                {
                    to_string[len_buffer++] = '-';
                    flt = -flt;
                }
                int tmp = (int)flt;
                // printf("[DEBUG]%d\n", tmp);
                while (tmp)
                {
                    to_string[len_buffer++] = tmp % 10 + '0';
                    tmp /= 10;
                }
                for(int i = 0; i < (len_buffer / 2); ++i)
                    swap(to_string[len_buffer - i - 1], to_string[i]);
                to_string[len_buffer ++] = '.';
                double ftmp = flt - (int)flt;
                ftmp *= 10;
                for (int i = 0; i < 6; i++)
                {
                    to_string[len_buffer++] = (int)ftmp + '0';
                    ftmp = (ftmp - (int)ftmp) * 10;
                }
                buffer[idx] = '\0';
                idx = 0;
                counter += write(buffer, 0x47);
                counter += write(to_string, 0x47);
                break;
            }
        }
    }

    buffer[idx] = '\0';
    counter += write(buffer, 0x47);

    return counter;
}

