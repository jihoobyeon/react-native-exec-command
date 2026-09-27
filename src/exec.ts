export default function exec(cwd: string, command: string, args?: string[], silent?: boolean): Promise<string> {
	throw new Error("'react-native-exec-command' is only supported on native platforms.");
}
