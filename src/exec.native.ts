import ExecCommand from './NativeExecCommand';

export function exec(cwd: string, command: string, args?: string[]): Promise<string> {
	return ExecCommand.exec(cwd, command, args ?? []);
}
