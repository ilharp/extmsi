#include "extmsi/main.hpp"

int main(int argc, char **argv) {
  if (!SetConsoleCP(CP_UTF8))
    LogAndFailWithLastError(L"Failed to set console codepage.");
  if (!SetConsoleOutputCP(CP_UTF8))
    LogAndFailWithLastError(L"Failed to set console output codepage.");

  if (argc != 2) LogAndFail(L"extmsi needs exactly 1 argument.");

  std::filesystem::path exePath = std::filesystem::path(argv[1]);
  std::filesystem::path wd = exePath.parent_path();
  std::string name = exePath.stem().string();

  HINSTANCE hExe = LoadLibraryExW(
      exePath.wstring().c_str(),
      nullptr,
      LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
  if (!hExe) LogAndFailWithLastError(L"Failed to load exe.");

  std::ofstream meta;
  meta.open(wd / (name + "_meta.tsv"), std::ios::out | std::ios::trunc);

  ExtMsiContext ctx(&exePath, &wd, &name, hExe, &meta);

  if (!EnumResourceNamesExW(
          hExe,
          EXTMSI_MSI,
          EnumNames,
          reinterpret_cast<long long int>(&ctx),
          RESOURCE_ENUM_LN,
          0))
    LogAndFailWithLastError(L"Failed to enumerate resources.");

  FreeLibrary(hExe);
  meta.close();

  return 0;
}

int EnumNames(
    HINSTANCE hExe, const wchar_t *type, wchar_t *wname, long long int rctx) {
  ExtMsiContext *ctx = (ExtMsiContext *)rctx;

  std::string name;

  if (IS_INTRESOURCE(wname))
    name =
        std::to_string(static_cast<short>(reinterpret_cast<intptr_t>(wname)));
  else
    name = std::string(WideCharToUTF8(wname).get());

  std::string targetName = *(ctx->name) + "_" + name + ".msi";
  std::filesystem::path targetPath = *(ctx->wd) / targetName;

  std::cerr << "Extracting: " << targetName << std::endl;

  HRSRC hRsrc = FindResourceW(hExe, wname, EXTMSI_MSI);
  if (!hRsrc) LogAndFailWithLastError(L"Failed to find resource.");
  HGLOBAL hGlobal = LoadResource(hExe, hRsrc);
  if (!hGlobal) LogAndFailWithLastError(L"Failed to load resource.");
  unsigned long const size = SizeofResource(hExe, hRsrc);
  if (!size) LogAndFailWithLastError(L"Failed to get size of resource.");
  void *p = LockResource(hGlobal);
  if (!p) LogAndFailWithLastError(L"Failed to lock resource.");

  std::ofstream file;
  file.open(targetPath, std::ios::out | std::ios::binary | std::ios::trunc);
  file.write(static_cast<const char *>(p), size);
  file.close();

  *(ctx->meta) << name << '\t' << size << '\t' << targetName << '\t'
               << std::endl;

  return 1;
}

void FailWithLastError() {
  wchar_t *message;
  unsigned long const err = GetLastError();
  FormatMessageW(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr,
      err,
      0,
      reinterpret_cast<wchar_t *>(&message),
      0,
      nullptr);
  std::wcerr << "[WinError " << err << "] " << message << std::endl;
  ExitProcess(1);
}

void LogAndFailWithLastError(const wchar_t *messages) {
  std::wcerr << messages << L" Last error:" << std::endl;
  FailWithLastError();
}

void LogAndFail(const wchar_t *messages) {
  std::wcerr << messages << std::endl;
  ExitProcess(1);
}

void LogAndFail(const std::wstring &messages) {
  std::wcerr << messages << std::endl;
  ExitProcess(1);
}

void CheckFailure(HRESULT hr, const std::wstring &message) {
  if (FAILED(hr)) {
    std::wcerr << "[WinError HRESULT " << hr << "] " << message << std::endl;
    ExitProcess(1);
  }
}

std::unique_ptr<char[]> WideCharToUTF8(_In_ wchar_t *w) {
  int len = WideCharToMultiByte(
      CP_UTF8, WC_ERR_INVALID_CHARS, w, -1, nullptr, 0, nullptr, nullptr);
  if (!len) return nullptr;
  std::unique_ptr<char[]> s = std::make_unique<char[]>(len + 1);
  s[len] = 0;
  WideCharToMultiByte(
      CP_UTF8, WC_ERR_INVALID_CHARS, w, -1, s.get(), len, nullptr, nullptr);
  return s;
}

std::unique_ptr<wchar_t[]> UTF8ToWideChar(const char *s) {
  int len =
      MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, nullptr, 0);
  if (!len) return nullptr;
  std::unique_ptr<wchar_t[]> w = std::make_unique<wchar_t[]>(len + 1);
  w[len] = 0;
  MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, w.get(), len);
  return w;
}
