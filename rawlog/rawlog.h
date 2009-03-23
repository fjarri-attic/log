#ifndef _RAWLOG_H
#define _RAWLOG_H

// Not a thread-safe
int RawLogStartA(size_t index, const char *file_name);
int RawLogStartW(size_t index, const wchar_t *file_name);

// Thread-safe, if log is started
void RawLogWriteA(size_t index, const char *message);
void RawLogWriteW(size_t index, const wchar_t *message);

// Not a thread-safe
void RawLogStop(size_t index);

#ifdef UNICODE
#define RawLogStart RawLogStartW
#define RawLogWrite RawLogWriteW
#else
#define RawLogStart RawLogStartA
#define RawLogWrite RawLogWriteA
#endif

#endif
