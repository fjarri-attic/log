
#ifdef LOG_EXPORTS
#define LOG_API __declspec(dllexport)
#else
#define LOG_API __declspec(dllimport)
#endif

DWORD WINAPI LogThreadProc(PVOID context);
LOG_API int LogInitA(size_t index, const char *file_name);
LOG_API int LogInitW(size_t index, const wchar_t *file_name);
LOG_API VOID LogWriteA(size_t index, const char *message);
LOG_API VOID LogWriteW(size_t index, const wchar_t *message);
LOG_API VOID LogStop(size_t index);

#ifdef UNICODE
#define LogInit LogInitW
#define LogWrite LogWriteW
#else
#define LogInit LogInitA
#define LogWrite LogWriteA
#endif