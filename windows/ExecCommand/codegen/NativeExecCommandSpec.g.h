
/*
 * This file is auto-generated from a NativeModule spec file in js.
 *
 * This is a C++ Spec class that should be used with MakeTurboModuleProvider to register native modules
 * in a way that also verifies at compile time that the native module matches the interface required
 * by the TurboModule JS spec.
 */
#pragma once
// clang-format off


#include <NativeModules.h>
#include <tuple>

namespace ExecCommandCodegen {

struct ExecCommandSpec : winrt::Microsoft::ReactNative::TurboModuleSpec {
  static constexpr auto methods = std::tuple{
      Method<void(std::string, std::string, std::vector<std::string>, Promise<std::string>) noexcept>{0, L"exec"},
  };

  template <class TModule>
  static constexpr void ValidateModule() noexcept {
    constexpr auto methodCheckResults = CheckMethods<TModule, ExecCommandSpec>();

    REACT_SHOW_METHOD_SPEC_ERRORS(
          0,
          "exec",
          "    REACT_METHOD(exec) void exec(std::string cwd, std::string command, std::vector<std::string> const & args, ::React::ReactPromise<std::string> &&result) noexcept { /* implementation */ }\n"
          "    REACT_METHOD(exec) static void exec(std::string cwd, std::string command, std::vector<std::string> const & args, ::React::ReactPromise<std::string> &&result) noexcept { /* implementation */ }\n");
  }
};

} // namespace ExecCommandCodegen
