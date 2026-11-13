# react-native-exec-command

Exec CLI commands in React Native desktop app

## Installation


```sh
yarn add react-native-exec-command

// Windows
npx react-native autolink-windows
// macOS
npx pod-install macos
```


## Usage


`exec` function gets three arguments:\
first one is string of absolute path which the terminal(Windows: cmd, macOS: zsh) would open.\
second string is the command. and last, optional array of string is arguments for the command.
```js
import exec from 'react-native-exec-command';

export default function App() {
	return (
		<View>
			<Button title='Windows' onPress={ async () => await exec('C:\\Users\\user\\Desktop', 'dir') } />
			<Button title='macOS' onPress={ async () => await exec('/Users/user/Desktop', 'ls', ['-a']) } />
		</View>
	);
};
```


## Contributing

- [Development workflow](CONTRIBUTING.md#development-workflow)
- [Sending a pull request](CONTRIBUTING.md#sending-a-pull-request)
- [Code of conduct](CODE_OF_CONDUCT.md)

## License

MIT

---

Made with [create-react-native-library](https://github.com/callstack/react-native-builder-bob)
