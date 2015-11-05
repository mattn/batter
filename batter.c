#include <windows.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>

BOOL WINAPI
ctrlhandler(DWORD t) {
  if (t == CTRL_C_EVENT || t == CTRL_BREAK_EVENT) {
    GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
    GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0);
    return FALSE;
  }
  return FALSE;
}

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
  char *p, *t, *e;
  t = p = strdup(GetCommandLine());
  if (!p) return -1;
  if (*p == '"')
    do p++; while (*p && *p != '"');
  else
    while (*p && *p != ' ') p++;
  if (!strncasecmp(p-4, ".exe", 4)) {
    e = p - 3;
    memcpy(e, "bat", 3);
  } else {
    char *comspec = getenv("COMSPEC"), *b;
    if (!comspec) comspec = "CMD";
    b = malloc(strlen(comspec) + 1 + 9 + strlen(t) + 5);
    *b = 0;
    strcat(b, comspec);
    strcat(b, " /c call ");
    strncat(b, t, (int) (p-t));
    e = b + strlen(b) + 1;
    strcat(b, ".bat");
    strcat(b, p);
    free(t);
    t = b;
  }
  if (GetFileAttributes(t) == -1)
    memcpy(e, "cmd", 3);
  si.cb = sizeof(si);
  if (!CreateProcess(
      NULL, t, NULL, NULL, TRUE,
      CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_PROCESS_GROUP,
      NULL, NULL, &si, &pi))
    r = emsg();
  else {
    SetConsoleCtrlHandler(ctrlhandler, TRUE);
    signal(SIGINT, SIG_IGN);
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
