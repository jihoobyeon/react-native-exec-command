#pragma once

#include "pch.h"
#include "resource.h"

#if __has_include("codegen/NativeExecCommandDataTypes.g.h")
  #include "codegen/NativeExecCommandDataTypes.g.h"
#endif
// Note: The following lines use Mustache template syntax which will be processed during
// project generation to produce standard C++ code. If existing codegen spec files are found,
// use the actual filename; otherwise use conditional includes.
#if __has_include("codegen/NativeExecCommandSpec.g.h")
  #include "codegen/NativeExecCommandSpec.g.h"
#endif

#include "NativeModules.h"

namespace winrt::ExecCommand
{

// See https://microsoft.github.io/react-native-windows/docs/native-platform for help writing native modules

  REACT_TURBO_MODULE(ExecCommand);
  struct ExecCommand
  {
    using ModuleSpec = ExecCommandCodegen::ExecCommandSpec;

    REACT_METHOD(exec);
    void exec(
      std::string cwd,
      std::string command,
      std::vector<std::string> args,
      winrt::Microsoft::ReactNative::ReactPromise<std::string> const& promise
    ) noexcept;
    
  private:
    React::ReactContext m_context;
  };

} // namespace winrt::ExecCommand