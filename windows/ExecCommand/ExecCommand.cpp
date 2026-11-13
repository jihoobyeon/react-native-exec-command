#include "pch.h"
#include "ExecCommand.h"

namespace
{
  // UTF-8 -> UTF-16
  std::wstring ToWide(const std::string& value)
  {
    if (value.empty())
      return L"";

    int size = MultiByteToWideChar(
      CP_UTF8,
      MB_ERR_INVALID_CHARS,
      value.data(),
      static_cast<int>(value.size()),
      nullptr,
      0
    );

    if (size <= 0)
      throw std::runtime_error("Invalid UTF-8 string.");

    std::wstring result(size, L'\0');

    MultiByteToWideChar(
      CP_UTF8,
      MB_ERR_INVALID_CHARS,
      value.data(),
      static_cast<int>(value.size()),
      result.data(),
      size
    );

    return result;
  }

  // UTF-16 -> UTF-8
  std::string ToUtf8(const std::wstring& value)
  {
    if (value.empty())
      return "";

    int size = WideCharToMultiByte(
      CP_UTF8,
      0,
      value.data(),
      static_cast<int>(value.size()),
      nullptr,
      0,
      nullptr,
      nullptr
    );

    if (size <= 0)
      throw std::runtime_error("UTF-8 conversion failed.");

    std::string result(size, '\0');

    WideCharToMultiByte(
      CP_UTF8,
      0,
      value.data(),
      static_cast<int>(value.size()),
      result.data(),
      size,
      nullptr,
      nullptr
    );

    return result;
  }

  // Windows CRT command-line argument quoting.
  std::wstring QuoteArgument(const std::wstring& value)
  {
    std::wstring result = L"\"";
    size_t backslashes = 0;

    for (wchar_t ch : value)
    {
      if (ch == L'\\')
      {
        ++backslashes;
      }
      else if (ch == L'"')
      {
        result.append(backslashes * 2 + 1, L'\\');
        result += L'"';
        backslashes = 0;
      }
      else
      {
        result.append(backslashes, L'\\');
        backslashes = 0;
        result += ch;
      }
    }

    // Backslashes immediately before the closing quote
    // must be doubled.
    result.append(backslashes * 2, L'\\');
    result += L'"';

    return result;
  }

  std::wstring BuildCommandLine(
    const std::wstring& command,
    const std::vector<std::wstring>& args)
  {
    std::wstring result = QuoteArgument(command);

    for (const auto& arg : args)
    {
      result += L" ";
      result += QuoteArgument(arg);
    }

    return result;
  }

  std::wstring GetWindowsError(DWORD error)
  {
    LPWSTR buffer = nullptr;

    DWORD size = FormatMessageW(
      FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr,
      error,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPWSTR>(&buffer),
      0,
      nullptr
    );

    std::wstring message =
      size && buffer ? buffer : L"Unknown Windows error";

    if (buffer)
      LocalFree(buffer);

    return message;
  }

  // Escape a value for use as a cmd.exe command argument.
  std::wstring EscapeCmdArgument(const std::wstring& value)
  {
    std::wstring result = L"\"";

    for (wchar_t ch : value)
    {
      switch (ch)
      {
      case L'%':
        // Prevent environment-variable expansion.
        result += L"%%";
        break;

      case L'^':
        result += L"^^";
        break;

      case L'&':
      case L'|':
      case L'<':
      case L'>':
      case L'(':
      case L')':
        result += L'^';
        result += ch;
        break;

      case L'"':
        // cmd.exe quoting is not equivalent to CRT quoting.
        // Reject embedded quotes instead of silently
        // changing the command's meaning.
        throw std::runtime_error(
          "Double quotes inside cmd arguments are not supported."
        );

      default:
        result += ch;
        break;
      }
    }

    result += L"\"";

    return result;
  }

  std::wstring BuildCmdCommand(
    const std::wstring& command,
    const std::vector<std::wstring>& args)
  {
    std::wstring result = command;

    for (const auto& arg : args)
    {
      result += L" ";
      result += EscapeCmdArgument(arg);
    }

    return result;
  }

  void RunVisible(
    const std::wstring& cwd,
    const std::wstring& command,
    const std::vector<std::wstring>& args)
  {
    std::wstring commandLine = BuildCmdCommand(command, args);

    // cmd.exe command:
    // cmd.exe /D /V:OFF /S /C "command arguments"
    std::wstring shellCommand =
      L"\"" + commandLine + L"\"";

    std::wstring fullCommandLine =
      L"cmd.exe /D /V:OFF /S /C " + shellCommand;

    STARTUPINFOW startupInfo{};
    startupInfo.cb = sizeof(STARTUPINFOW);
    startupInfo.dwFlags = STARTF_USESHOWWINDOW;
    startupInfo.wShowWindow = SW_SHOW;

    PROCESS_INFORMATION processInfo{};

    std::vector<wchar_t> mutableCommandLine(
      fullCommandLine.begin(),
      fullCommandLine.end()
    );

    mutableCommandLine.push_back(L'\0');

    BOOL created = CreateProcessW(
      nullptr,
      mutableCommandLine.data(),
      nullptr,
      nullptr,
      FALSE,
      CREATE_NEW_CONSOLE,
      nullptr,
      cwd.empty() ? nullptr : cwd.c_str(),
      &startupInfo,
      &processInfo
    );

    if (!created)
    {
      DWORD error = GetLastError();

      throw std::runtime_error(
        ToUtf8(GetWindowsError(error))
      );
    }

    // cmd.exe exits after /C finishes.
    // Its console window closes automatically.
    WaitForSingleObject(
      processInfo.hProcess,
      INFINITE
    );

    DWORD exitCode = 0;

    GetExitCodeProcess(
      processInfo.hProcess,
      &exitCode
    );

    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);

    if (exitCode != 0)
    {
      throw std::runtime_error(
        "Command failed with exit code: " +
        std::to_string(exitCode)
      );
    }
  }
}

namespace winrt::ExecCommand
{
  void ExecCommand::exec(
    std::string cwd,
    std::string command,
    std::vector<std::string> args,
    winrt::Microsoft::ReactNative::ReactPromise<std::string> const& promise
  ) noexcept
  {
    // Copy the promise and arguments before leaving this method.
    auto capturedPromise = promise;

    std::thread(
      [cwd = std::move(cwd),
      command = std::move(command),
      args = std::move(args),
      capturedPromise]() mutable
      {
        try
        {
          std::wstring wideCwd = ToWide(cwd);
          std::wstring wideCommand = ToWide(command);

          std::vector<std::wstring> wideArgs;
          wideArgs.reserve(args.size());

          for (const auto& arg : args)
          {
            wideArgs.push_back(ToWide(arg));
          }

          RunVisible(
            wideCwd,
            wideCommand,
            wideArgs
          );

          capturedPromise.Resolve("");
        }
        catch (const std::exception& error)
        {
          winrt::Microsoft::ReactNative::ReactError reactError{};

          reactError.Code = "EXEC_FAILED";
          reactError.Message = error.what();

          capturedPromise.Reject(reactError);
        }
        catch (...)
        {
          winrt::Microsoft::ReactNative::ReactError reactError{};

          reactError.Code = "EXEC_FAILED";
          reactError.Message = "Unknown execution error.";

          capturedPromise.Reject(reactError);
        }
      }
    ).detach();
  }
} // namespace winrt::ExecCommand