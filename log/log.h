
#ifdef LOG_EXPORTS
#define LOG_API __declspec(dllexport)
#else
#define LOG_API __declspec(dllimport)
#endif

LOG_API int LogStartA(size_t index, const char *file_name);
LOG_API int LogStartW(size_t index, const wchar_t *file_name);
LOG_API VOID LogWriteA(size_t index, const char *message);
LOG_API VOID LogWriteW(size_t index, const wchar_t *message);
LOG_API VOID LogStop(size_t index);

#ifdef UNICODE
#define LogStart LogStartW
#define LogWrite LogWriteW
#else
#define LogStart LogStartA
#define LogWrite LogWriteA
#endif