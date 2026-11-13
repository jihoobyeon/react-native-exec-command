import { TurboModuleRegistry, type TurboModule } from 'react-native';

export interface Spec extends TurboModule {
	exec(cwd: string, command: string, args: string[]): Promise<string>;
}

export default TurboModuleRegistry.getEnforcing<Spec>('ExecCommand');
