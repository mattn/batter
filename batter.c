#include <windows.h>
#include <string.h>
#include <stdio.h>

int
emsg() {
  char* p = NULL;
  DWORD err = GetLastError();
  if (err == 0) return 0;
  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, err, 0, (LPTSTR)(&p), 0, NULL);
  fputs(p, stderr);
  LocalFree((LPVOID)p);
  return err;
}

int
main(int argc, char* argv[]) {
  STARTUPINFO si = {0};
  PROCESS_INFORMATION pi = {0};
  int r;
  char *p, *t;
  t = p = strdup(GetCommandLine());
  if (!p) return -1;
  if (*p == '"')
    do p++; while (*p && *p != '"');
  else
    while (*p && *p != ' ') p++;
  if (!strncasecmp(p-4, ".exe", 4))
    memcpy(p-4, ".bat", 4);
  else {
    char *comspec = getenv("COMSPEC"), *b;
    if (!comspec) comspec = "CMD";
    b = malloc(strlen(comspec) + 1 + 9 + strlen(t) + 5);
    *b = 0;
    strcat(b, comspec);
    strcat(b, " /c call ");
    strncat(b, t, (int) (p-t));
    strcat(b, ".bat");
    strcat(b, p);
    free(t);
    t = b;
  }
  si.cb = sizeof(si);
  if (!CreateProcess(
      NULL, t, NULL, NULL, TRUE,
      CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_PROCESS_GROUP,
      NULL, NULL, &si, &pi))
    r = emsg();
  else {
    CloseHandle(pi.hThread);
    if (WaitForSingleObject(pi.hProcess, INFINITE) == WAIT_FAILED)
      r = emsg();
    else {
      DWORD code = 0;
      if (!GetExitCodeProcess(pi.hProcess, &code))
        r = emsg();
      else {
        r = (int) code;
        CloseHandle(pi.hProcess);
      }
    }
  }
  free(t);
  return r;
}