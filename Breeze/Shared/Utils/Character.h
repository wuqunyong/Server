
#ifndef __SHARED_UTILS_CHARACTER_H__
#define __SHARED_UTILS_CHARACTER_H__

#ifndef WIN32
#include <iconv.h>
#endif

#include "Singleton.h"

class CharacterConverter
{
public:
    CharacterConverter(void);
    ~CharacterConverter(void);

public:
    char* ConvertToUtf8(char* src);
    char* ConvertFromUtf8(char* src);

protected:
#ifndef WIN32
    iconv_t _encoder;
    iconv_t _decoder;
#endif
};

#define SINGLETON_CHARACTER_CONVERTER Singleton<CharacterConverter>::Instance()

#endif 
