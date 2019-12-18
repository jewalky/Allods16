#pragma once

#include <string>
#include <vector>

#include <cstdlib>

// string-related
std::string __c_Format(const char* format, ...);
std::vector<std::string> Explode(const std::string& what, const std::string& separator);
bool IsWhitespace(char what);
std::string TrimLeft(const std::string& what, bool (callback)(char) = IsWhitespace);
std::string TrimRight(const std::string& what, bool (callback)(char) = IsWhitespace);
std::string Trim(const std::string& what, bool (callback)(char) = IsWhitespace);
std::string ToLower(const std::string& what);
std::string ToUpper(const std::string& what);
unsigned long StrToInt(const std::string& what);
unsigned long HexToInt(const std::string& what);
double StrToFloat(const std::string& what);
bool StrToBool(const std::string& what);
bool CheckInt(const std::string& what);
bool CheckBool(const std::string& what);
bool CheckHex(const std::string& what);
bool CheckFloat(const std::string& what);
bool CheckIP(const std::string& what);

// file related
bool FileExists(const std::string& file);
std::string Basename(const std::string& file);
std::string FixSlashes(const std::string& filename);
std::string TruncateSlashes(const std::string& filename);

// Formatting

struct _Printf_flags
{
    int prec;
    int flg_zero;
    int flg_minus;
    int flg_plus;
    int flg_spac;
    int pad;

    enum
    {
        Tauto,
        Tstring,
        Tint,
        Thex,
        Tbighex,
        Tfloat,
        Tdouble,
        Tpointer,
        Tbigint,
    } type;
};

std::string _impl_Printf(_Printf_flags& flags, int p);
std::string _impl_Printf(_Printf_flags& flags, void* p);
std::string _impl_Printf(_Printf_flags& flags, std::string p);
std::string _impl_Printf(_Printf_flags& flags, double p);

std::string Format(const char* s);

template<typename T, typename... Args>
std::string Format(const char* s, T value, Args... args)
{
    _Printf_flags flags;
    bool reading = false;
    int32_t read_size = 0;
    std::string format_str = "";
    std::string out_str = "";
    while (*s)
    {
        if (reading)
        {
            if (*s == '%' && !read_size) // %%
            {
                out_str += "%";
                reading = false;
                continue;
            }

            format_str.push_back(*s);

            if (*s == '+') // pad before string
                flags.flg_plus = 1;
            else if (*s == '-') // pad after string
                flags.flg_minus = 1;
            else if (*s == '0') // pad with zeros
                flags.flg_zero = 1;
             else if (*s == ' ') // pad with spaces
                flags.flg_spac = 1;
            else if (*s == '*' && flags.prec == -2)
                flags.prec = -1;
            else if (*s >= '1' && *s <= '9')
            {
                std::string num_pad;
                num_pad.push_back(*s);
                s++;
                format_str.push_back(*s);
                while (*s && *s >= '1' && *s <= '9')
                {
                    num_pad.push_back(*s);
                    s++;
                    format_str.push_back(*s);
                }
                if (flags.prec == -2) flags.prec = StrToInt(num_pad);
                else flags.pad = StrToInt(num_pad);
                s--;
            }
            else if (*s == '.') // precision
                flags.prec = -2;
            else
            {
                if (*s == 's')
                    flags.type = _Printf_flags::Tstring;
                else if (*s == 'd')
                    flags.type = _Printf_flags::Tint;
                else if (*s == 'X')
                    flags.type = _Printf_flags::Tbighex;
                else if (*s == 'x')
                    flags.type = _Printf_flags::Thex;
                else if (*s == 'p')
                    flags.type = _Printf_flags::Tpointer; // will ALWAYS be same as format 0x%08X
                else if (*s == 'u')
                    flags.type = _Printf_flags::Tbigint;
                else if (*s == 'f')
                    flags.type = _Printf_flags::Tdouble;

                if (flags.type == _Printf_flags::Tauto)
                {
                    out_str += format_str;
                    reading = false;
                    s++;
                    continue;
                }
                else
                {
                    out_str += _impl_Printf(flags, value);
                    s++;
                    return out_str+Format(s, args...);
                }
            }

            read_size++;
        }
        else if (*s == '%')
        {
            reading = true;
            read_size = 0;
            flags.prec = -1;
            flags.flg_zero = 0;
            flags.flg_minus = 0;
            flags.flg_plus = 0;
            flags.flg_spac = 0;
            flags.pad = 0;
            format_str = "%";
            flags.type = _Printf_flags::Tauto;
        }
        else
        {
            out_str += *s;
        }

        s++;
    }

    return out_str;
}

