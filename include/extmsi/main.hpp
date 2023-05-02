#ifndef _EXTMSI_MAIN_
#define _EXTMSI_MAIN_

#include "extmsi/stdafx.hpp"

const wchar_t *EXTMSI_MSI = L"MSI";

class ExtMsiContext {
public:
  std::filesystem::path *exePath;
  std::filesystem::path *wd;
  std::string *name;

  HINSTANCE hExe;

  std::ofstream *meta;

  ExtMsiContext(
      std::filesystem::path *exePath,
      std::filesystem::path *wd,
      std::string *name,
      HINSTANCE hExe,
      std::ofstream *meta)
      : exePath(exePath), wd(wd), name(name), hExe(hExe), meta(meta) {
  }
};

int main(int argc, char **argv);

int EnumNames(
    HINSTANCE hExe, const wchar_t *type, wchar_t *wname, long long int rctx);

void FailWithLastError();

void LogAndFailWithLastError(const wchar_t *messages);

void LogAndFail(const wchar_t *messages);

std::unique_ptr<char[]> WideCharToUTF8(wchar_t *w);

#endif /* _EXTMSI_MAIN_ */
