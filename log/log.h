
#ifdef LOG_EXPORTS
#define LOG_API __declspec(dllexport)
#else
#define LOG_API __declspec(dllimport)
#endif

LOG_API DWORD LogInitA(const char *file_name);
LOG_API DWORD LogInitW(const wchar_t *file_name);
LOG_API VOID LogWriteA(const char *message);
LOG_API VOID LogWriteW(const wchar_t *message);
LOG_API VOID LogStop();

#ifdef UNICODE
#define LogInit LogInitW
#define LogWrite LogWriteW
#else
#define LogInit LogInitA
#define LogWrite LogWriteA
#endif