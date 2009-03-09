
#ifdef LOG_EXPORTS
#define LOG_API __declspec(dllexport)
#else
#define LOG_API __declspec(dllimport)
#endif

LOG_API DWORD LogInit(const TCHAR *file_name);
LOG_API VOID LogWrite(const char *message);
LOG_API VOID LogWrite(const wchar_t *message);

