#import "ExecCommand.h"

static NSString *ShellQuote(NSString *value)
{
  NSString *escaped =
    [value stringByReplacingOccurrencesOfString:@"'"
                                    withString:@"'\\''"];

  return [NSString stringWithFormat:@"'%@'", escaped];
}

@implementation ExecCommand
- (void)exec:(NSString *)cwd
     command:(NSString *)command
        args:(NSArray *)args
     resolve:(RCTPromiseResolveBlock)resolve
      reject:(RCTPromiseRejectBlock)reject
{
    NSMutableArray<NSString *> *parts = [NSMutableArray array];
    [parts addObject:ShellQuote(command)];

    for (NSString *arg in args) {
      [parts addObject:ShellQuote(arg)];
    }

    NSString *shellCommand =
      [NSString stringWithFormat:@"cd %@ && %@",
        ShellQuote(cwd),
        [parts componentsJoinedByString:@" "]];

    NSString *escapedForAppleScript =
      [[shellCommand stringByReplacingOccurrencesOfString:@"\\"
                                               withString:@"\\\\"]
        stringByReplacingOccurrencesOfString:@"\""
                                  withString:@"\\\""];

    NSString *script = [NSString stringWithFormat:
      @"tell application \"Terminal\"\n"
       "activate\n"
       "do script \"%@\"\n"
       "end tell",
      escapedForAppleScript];

    NSTask *task = [[NSTask alloc] init];
    task.executableURL = [NSURL fileURLWithPath:@"/usr/bin/osascript"];

    task.arguments = @[
      @"-e",
      script
    ];

    NSError *error = nil;

    if (![task launchAndReturnError:&error]) {
      reject(@"TERMINAL_FAILED", error.localizedDescription, error);
      return;
    }

    [task waitUntilExit];

    if (task.terminationStatus != 0) {
      reject(@"TERMINAL_FAILED", @"Failed to open Terminal.", nil);
      return;
    }

    resolve(@"");
}

- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:
    (const facebook::react::ObjCTurboModule::InitParams &)params
{
  return std::make_shared<facebook::react::NativeExecCommandSpecJSI>(params);
}

+ (NSString *)moduleName
{
  return @"ExecCommand";
}

@end
