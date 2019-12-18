#include "utils.h"

#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#include <ctime>

using namespace std;

/*
    utils::Format: функция, аналогичная sprintf(), но для STL строк.

    Возвращает отформатированную STL строку.

    format: формат
    ...: аргументы для формата
*/
std::string __c_Format(const char* format, ...)
{
    char* line = NULL;
    try
    {
        va_list list;
        va_start(list, format);
        size_t linesize = (vsnprintf(NULL, 0, format, list) + 1);
        line = new char[linesize + 1];
        if (!line) return "";
        line[linesize] = 0;
        vsnprintf(line, linesize, format, list);
        va_end(list);

        std::string sline(line);
        delete[] line;

        return sline;
    }
    catch (...)
    {
        if (line) delete[] line;
        return "";
    }
}

/*
    utils::Explode: функция для разбиения строки на подстроки.

    Возвращает массив из найденных подстрок.

    what: разбиваемая строка
    separator: разделитель
*/
vector<string> Explode(const string& what, const string& separator)
{
    string curstr;
    vector<string> retval;
    for (size_t i = 0; i < what.length(); i++)
    {
        if (what.find(separator, i) == i)
        {
            retval.push_back(curstr);
            curstr.assign("");
            i += separator.length() - 1;
            continue;
        }

        curstr += what[i];
    }
    retval.push_back(curstr);
    return retval;
}

/*
    utils::TrimLeft, utils::TrimRight, utils::Trim: обрезать незначимые символы в начале строки (TrimLeft), в конце (TrimRight)
        или и там, и там (Trim).

    Возвращает обрезанную строку.

    what: обрезаемая строка
    callback: функция, определяющая значимость символа. См. IsWhitespace
*/
string TrimLeft(const string& what, bool (callback)(char))
{
    string ret = what;
    for (string::iterator i = ret.begin(); i != ret.end(); ++i)
    {
        if (callback((*i)))
            continue;
        ret.erase(ret.begin(), i);
        return ret;
    }
    return "";
}

string TrimRight(const string& what, bool (callback)(char))
{
    string ret = what;
    for (string::reverse_iterator i = ret.rbegin(); i != ret.rend(); ++i)
    {
        if (callback((*i)))
            continue;
        ret.erase(i.base(), ret.end());
        return ret;
    }
    return "";
}

string Trim(const string& what, bool (callback)(char))
{
    return TrimRight(TrimLeft(what, callback));
}

/*
    utils::IsWhitespace: вспомогательная функция для Trim*

    Возвращает true, если символ является пробелом и его нужно обрезать.

    what: проверяемый символ.
*/
bool IsWhitespace(char what)
{
    switch ((unsigned char)what)
    {
    case ' ':
    case '\r':
    case '\n':
    case '\t':
    case 0xFF:
        return true;
    default:
        return false;
    }
}

/*
    utils::ToLower, utils::ToUpper: преобразование строки в верхний или нижний регистр.

    Возвращает преобразованную строку.

    what: Преобразовываемая строка.
*/
string ToLower(const string& what)
{
    string ret = what;
    for (string::iterator i = ret.begin(); i != ret.end(); ++i)
        (*i) = tolower((*i));
    return ret;
}

string ToUpper(const string& what)
{
    string ret = what;
    for (string::iterator i = ret.begin(); i != ret.end(); ++i)
        (*i) = toupper((*i));
    return ret;
}

#include <fstream>

/*
    utils::FileExists: проверка на существование файла.

    Возвращает true, если файл существует.

    filename: название проверяемого файла.
*/
bool FileExists(const string& filename)
{
    ifstream f_temp;
    f_temp.open(filename.c_str(), ios::in | ios::binary);
    if (!f_temp.is_open()) return false;
    f_temp.close();
    return true;
}

/*
    utils::Basename: срезать путь к файлу, оставив только его название.

    Возвращаемое значение: обрезанная строка.

    filename: старое название файла.
*/
string Basename(const string& filename)
{
    string ret = FixSlashes(filename);
    uint32_t where = ret.find_last_of('/');
    if (where == string::npos) return ret;
    ret.erase(0, where + 1);
    return ret;
}

/*
    utils::FixSlashes: сделать путь к файлу POSIX-совместимым (т.е. с человеческими слешами вместо backslash, используемого в DOS/Windows)

    Возвращаемое значение: исправленная строка.

    filename: старое название файла.
*/
string FixSlashes(const string& filename)
{
    string ret = filename;
    for (string::iterator i = ret.begin(); i != ret.end(); ++i)
        if ((*i) == '\\') (*i) = '/';
    return ret;
}

/*
    utils::TruncateSlashes: удалить повторяющиеся слеши (напр. main//graphics/mainmenu//menu_.bmp).

    Возвращает исправленную строку.

    filename: старое название файла.
*/
string TruncateSlashes(const string& filename)
{
    string ret = filename;
    char lastchar = 0;
    char thischar = 0;
    for (string::iterator i = ret.begin(); i != ret.end(); ++i)
    {
        thischar = (*i);
        if ((thischar == '/' || thischar == '\\') &&
            (lastchar == '/' || lastchar == '\\'))
        {
            ret.erase(i);
            i--;
        }
        lastchar = thischar;
    }
    return ret;
}

unsigned long StrToInt(const string& what)
{
    unsigned int retval;
    sscanf_s(what.c_str(), "%u", &retval);
    return retval;
}

unsigned long HexToInt(const string& what)
{
    unsigned int retval;
    sscanf_s(what.c_str(), "%X", &retval);
    return retval;
}

double StrToFloat(const string& what)
{
    float retval;
    sscanf_s(what.c_str(), "%f", &retval);
    return (double)retval;
}

bool CheckInt(const string& what)
{
    for (size_t i = 0; i < what.length(); i++)
        if (what[i] < 0x30 || 0x39 < what[i]) return false;
    return true;
}

bool CheckFloat(const string& what)
{
    for (size_t i = 0; i < what.length(); i++)
        if ((what[i] < 0x30 || 0x39 < what[i]) && what[i] != '.') return false;
    return true;
}

bool CheckHex(const string& what)
{
    for (size_t i = 0; i < what.length(); i++)
        if ((what[i] < 0x30 || 0x39 < what[i]) && !(what[i] >= 'A' && what[i] <= 'F') && !(what[i] >= 'a' && what[i] <= 'f')) return false;
    return true;
}

bool CheckBool(const string& what)
{
    string wh2 = ToLower(Trim(what));
    if (wh2 == "true" || wh2 == "false" || wh2 == "yes" || wh2 == "no" || wh2 == "y" || wh2 == "n" || wh2 == "0" || wh2 == "1")
        return true;
    return false;
}

bool StrToBool(const string& what)
{
    string cr = Trim(ToLower(what));
    if (cr == "yes" || cr == "true" || cr == "1" || cr == "y")
        return true;
    return false;
}

bool CheckIP(const std::string& addr)
{
    std::vector<std::string> addr2 = Explode(addr, ".");
    return ((addr2.size() == 4) && CheckInt(addr2[0]) && CheckInt(addr2[1]) && CheckInt(addr2[2]) && CheckInt(addr2[3]));
}

///
std::string Format(const char* s)
{
    return s;
}

std::string _impl_Printf(_Printf_flags& flags, int64_t p)
{
    // we cheat here
    std::string printf_real = "%";
    if (flags.flg_minus == 1)
        printf_real.push_back('-');
    if (flags.flg_plus == 1)
        printf_real.push_back('+');
    if (flags.flg_zero == 1)
        printf_real.push_back('0');
    if (flags.pad > 0)
        printf_real += __c_Format("%u", flags.pad);
    if (flags.type == _Printf_flags::Thex) printf_real.push_back('x');
    else if (flags.type == _Printf_flags::Tbighex) printf_real.push_back('X');
    else if (flags.type == _Printf_flags::Tpointer)
    {
        return __c_Format("0x%08X", (uint64_t)p);
    }
    else if (flags.type == _Printf_flags::Tbigint) printf_real.push_back('u');
    else printf_real.push_back('d');
    return __c_Format(printf_real.c_str(), p);
}

std::string _impl_Printf(_Printf_flags& flags, void* p)
{
    return __c_Format("0x%08X", (unsigned int)p);
}

std::string _impl_Printf(_Printf_flags& flags, std::string p)
{
    std::string printf_real = "%";
    if (flags.flg_minus == 1)
        printf_real.push_back('-');
    if (flags.flg_plus == 1)
        printf_real.push_back('+');
    if (flags.flg_zero == 1)
        printf_real.push_back('0');
    if (flags.pad > 0)
        printf_real += __c_Format("%u", flags.pad);
    if (flags.type == _Printf_flags::Tint ||
        flags.type == _Printf_flags::Thex ||
        flags.type == _Printf_flags::Tbighex)
    {
        int dp = StrToInt(p);
        return _impl_Printf(flags, dp);
    }
    else if (flags.type == _Printf_flags::Tpointer) // odd.
    {
        // let's write some arbitrary pointer
        return __c_Format("0x%08X", (unsigned int)p.c_str());
    }
    else printf_real.push_back('s');
    return __c_Format(printf_real.c_str(), p.c_str());
}

std::string _impl_Printf(_Printf_flags& flags, double_t p)
{
    std::string printf_real = "%";
    if (flags.flg_minus == 1)
        printf_real.push_back('-');
    if (flags.flg_plus == 1)
        printf_real.push_back('+');
    if (flags.flg_zero == 1)
        printf_real.push_back('0');
    if (flags.pad > 0)
        printf_real += __c_Format("%u", flags.pad);
    if (flags.prec != -1 && (flags.type == _Printf_flags::Tfloat ||
        flags.type == _Printf_flags::Tdouble))
        printf_real += __c_Format(".%u", flags.prec);
    if (flags.type == _Printf_flags::Tint ||
        flags.type == _Printf_flags::Tbigint)
    {
        printf_real.push_back(flags.type == _Printf_flags::Tint ? 'd' : 'u');
        int dp = p;
        return __c_Format(printf_real.c_str(), dp);
    }
    else if (flags.type == _Printf_flags::Thex ||
        flags.type == _Printf_flags::Tbighex)
    {
        printf_real.push_back(flags.type == _Printf_flags::Thex ? 'x' : 'X');
        // magic
        unsigned long dp = *(unsigned long*)(&p);
        return __c_Format(printf_real.c_str(), dp);
    }
    else if (flags.type == _Printf_flags::Tfloat ||
        flags.type == _Printf_flags::Tdouble)
    {
        printf_real.push_back('f');
        return __c_Format(printf_real.c_str(), p);
    }
    return printf_real;
}