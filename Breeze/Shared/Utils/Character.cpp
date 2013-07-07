/**
 * @file   Character.cpp
 * @author Xuzhou
 */

#include <stdio.h>
#include <time.h>
#include <locale.h>

#ifdef WIN32

#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CODEC_UTF8 "UTF-8"
#define CODEC_GBK  "GBK"
#endif

#include "Character.h"

#define GBK_LENGTH  1
#define UTF8_LENGTH 3

CharacterConverter::CharacterConverter(void)
{
#ifndef WIN32
    _encoder = iconv_open(CODEC_UTF8, "GBK");
    if ((iconv_t)-1 == _encoder)
        printf("CharacterConverter::Encoder error\n");

    _decoder = iconv_open("GBK", CODEC_UTF8);
    if ((iconv_t)-1 == _decoder)
        printf("CharacterConverter::Decoder error\n");
#endif
}
CharacterConverter::~CharacterConverter(void)
{
#ifndef WIN32
    iconv_close(_encoder);
    iconv_close(_decoder);
#endif
}

char* 
CharacterConverter::ConvertToUtf8(char* src)
{
#ifdef WIN32
    ::setlocale(LC_ALL, "chs");

    size_t len = strlen(src) + GBK_LENGTH;
    wchar_t* temp = (wchar_t*)malloc(len * sizeof(wchar_t));

    ::wmemset(temp, 0, len);
    ::mbstowcs(temp, src, len * sizeof(wchar_t));

    ::setlocale(LC_ALL, NULL);

    char* dst = (char*)malloc(len * UTF8_LENGTH);
    ::memset(dst, 0, len * UTF8_LENGTH);

    ::WideCharToMultiByte (CP_UTF8, 0, temp, -1, dst, len * UTF8_LENGTH, 0, 0);

    free(temp);

    return dst;
#else
    size_t srcLen = strlen(src);
    size_t dstLen = srcLen * UTF8_LENGTH + UTF8_LENGTH;
    char *dst = (char*)malloc(dstLen);
    ::memset(dst, 0, dstLen);

    char *p = dst;

    iconv(_encoder, &src, &srcLen, &p, &dstLen);

    return dst;
#endif
}

char* 
CharacterConverter::ConvertFromUtf8(char* src)
{
#ifdef WIN32
    size_t len = strlen(src) + GBK_LENGTH;
    wchar_t* temp = (wchar_t*)malloc(len * sizeof(wchar_t));

    ::wmemset(temp, 0, len);
    ::mbstowcs(temp, src, len);
    ::MultiByteToWideChar(CP_UTF8, 0, src, -1, temp, len * UTF8_LENGTH);

    setlocale(LC_ALL, "chs");

    char* dst = (char*)malloc(len * UTF8_LENGTH);
    ::memset(dst, 0, len * UTF8_LENGTH);
    ::wcstombs(dst, temp, len * UTF8_LENGTH);

    ::setlocale(LC_ALL, NULL);

    free(temp);

    return dst;
#else
    size_t srcLen = strlen(src);
    size_t dstLen = srcLen + UTF8_LENGTH;
    char *dst = (char*)malloc(dstLen);
    ::memset(dst, 0, dstLen);

    char *p = dst;
    iconv(_decoder, &src, &srcLen, &p, &dstLen);

    return dst;
#endif
}

