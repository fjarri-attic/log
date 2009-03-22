#ifndef _RAWLOG_H
#define _RAWLOG_H

int RawLogStartA(size_t index, const char *file_name);
int RawLogStartW(size_t index, const wchar_t *file_name);
VOID RawLogWriteA(size_t index, const char *message);
VOID RawLogWriteW(size_t index, const wchar_t *message);
VOID RawLogStop(size_t index);

#ifdef UNICODE
#define RawLogStart RawLogStartW
#define RawLogWrite RawLogWriteW
#else
#define RawLogStart RawLogStartA
#define RawLogWrite RawLogWriteA
#endif

#endif
