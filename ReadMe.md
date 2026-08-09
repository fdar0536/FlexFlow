# Flex Flow

<p align="center">
  <img src="src-cpp/original-icon.png" alt="Flex Flow icon" width="200" height="200">
</p>

## Introduction

Flex Flow is a lightweight task queue and workflow management system for local and remote execution.

It provides a C++ gRPC server, a command-line interface (CLI), an HTTP gRPC Gateway, and an optional web UI.  

Tasks are executed sequentially and can be monitored through the CLI or the web UI.

> [!WARNING]
> Do not run Flex Flow as `root`, with `sudo`, or as a Windows administrator.
>
> Flex Flow is intended for local development and trusted environments. Running it with elevated privileges may expose the host system to serious security risks.

## Architecture

```  
                              +----------------+
                              | Browser        |
                              +----------------+
                                      |
                                      | HTTP
                                      v
                              +----------------+
                              | HTTP Server /  |
                              | Reverse Proxy  |
                              +----------------+
                                |            |
                    Static Web  |            | API requests
                    UI files    |            |
                                |            v
                                |      +-----------+
                                |      | Gateway   |
                                |      +-----------+
                                |            |
                                |            | gRPC
                                |            v
+-----------+                   |      +-----------+
| CLI       |------------------------->| gRPC      |
+-----------+                          | Server    |
                                      +-----------+
                                            |
                                            v
                                      +-----------+
                                      | Model     |
                                      +-----------+
```

The Web UI is an independent static front-end component.  
It may be served by a general-purpose HTTP server such as Nginx, or by the HTTP server built into the gRPC Gateway.  
During development, a reverse proxy can expose both the Web UI development server and the Gateway through a single HTTP endpoint.  

## Extensibility

Flex Flow defines its public API using Protocol Buffers and exposes it through gRPC.

The server is intentionally independent of any specific user interface. Any language with official gRPC support can generate client stubs directly from the `.proto` files and communicate with the server.

This makes it easy to build custom clients and integrations, including:

- Desktop applications
- Mobile applications
- Web front ends
- Command-line tools
- Automation scripts
- Third-party integrations

This project currently provides:

- C++ gRPC server
- CLI
- Web UI
- HTTP gRPC Gateway

## Network Behavior

The server and gateway listen **only on `127.0.0.1`**.  
This is an intentional security design. By default, Flex Flow does **not** accept direct connections from other machines on the network.  
Remote access should be provided through SSH tunneling, a VPN, or a reverse proxy.  
**Directly exposing Flex Flow to the public Internet is not supported.**  

## Security

Current security assumptions:  

- Run Flex Flow as a regular user.
- Do not use root or administrator privileges.
- Only use Flex Flow in trusted environments.
- Remote access should be provided through a secure tunnel or reverse proxy.

## Platform Support

### Prebuilt binaries

Prebuilt binaries are provided for:

* Windows 11 x86_64 with **"Use Unicode UTF-8 for worldwide language support"** enabled
* macOS arm64 (Apple silicon)  

#### Note for Windows requirements

The Windows prebuilt binaries require the **"Use Unicode UTF-8 for worldwide language support"** system option to be enabled.  

Without this setting, Unicode file paths and console I/O may not work correctly.  

### Source builds

Source builds are currently tested on:  

* Arch Linux x86_64

Other Linux distributions and platforms may work but are currently untested and are not supported.  

Users who require support for other architectures or operating system versions may build or port Flex Flow from source.  

## Building from Source

### Required tools

* [Task](https://taskfile.dev)  
* [CMake](https://cmake.org)  
* [Git](https://git-scm.com)  
* A compiler with C++23 support
  * Tested compilers
    * Windows: MSVC
    * Linux: GCC
    * macOS: Clang
* [Buf](https://buf.build)
* [pnpm](https://pnpm.io)
* [Go](https://go.dev)

### C++ dependencies

Dependencies are installed using:  
* Windows: [conan](https://conan.io)
* Arch Linux: [yay](https://github.com/jguer/yay)
* macOS: [homebrew](https://brew.sh)

#### Core components

* C++23  
* Protocol Buffers  
* [SQLite](https://www.sqlite.org)  
* [gRPC](https://grpc.io)  
* [spdlog](https://github.com/gabime/spdlog)  
* [OpenSSL](https://www.openssl.org)

#### Server and CLI additionally require

* [CLI11](https://github.com/CLIUtils/CLI11)  
* [yaml-cpp](https://github.com/jbeder/yaml-cpp)  

## Build Configuration

Build settings are loaded from the `.env` file in the project root.  
Copy `.env.default` to `.env` before building:  
```sh  
cp .env.default .env  
```  

Review and update the values in `.env` for your environment.  
See .env.default for all available build options.  

## Build Commands

Minimum build:  
```sh  
task build:cpp
```  
Optional:  
```sh  
task build:gateway
task build:webui
```

#### Available build commands

Build the C++ model, gRPC server and CLI:  
```sh  
task build:cpp  
```  
Build the web UI:  
```sh  
task build:webui  
```  
Build the gateway:  
```sh  
task build:gateway  
```  
Clean generated build files:  
```sh  
task clean:all  
```  

## Usage

- Each executable provides built-in command-line help:  
```sh  
FlexFlowCLI --help
FlexFlowServer --help
FlexFlowGateway --help
```  
- For more detailed usage instructions, refer to the [User Guide](docs/UserGuide.md).

### Example

```sh
FlexFlowServer -c server.yaml

FlexFlowCLI create -n build

FlexFlowCLI queue \
    -n build \
    add-task \
    -e cmake \
    -a "--build build" \
    -w .

FlexFlowCLI queue \
    -n build \
    start
```

## Binary Releases

Prebuilt binaries for Windows and macOS are available on the GitHub Releases page.  
Linux users should build Flex Flow from source.  

### macOS

Prebuilt binaries are not code signed or notarized.  
  
After extracting the release package, remove the macOS quarantine attribute before running the executables:  
  
```sh  
xattr -dr com.apple.quarantine .  
```  

## Release Verification

Every release includes:  

- Platform archives (`*.tar.zst`)
- `SHA256SUMS`
- `SHA256SUMS.asc`

The checksum file is signed with the Flex Flow release signing key.  
  
The corresponding public key is available in this repository as:  
  
```text  
release-signing-key.asc  
```  
  
Verify the signature before checking the archive checksums.  

## Documentation

- [User Guide](docs/UserGuide.md)
- Configuration examples
- [Protocol Buffers definitions](protos/)
- [C++ source code with Doxygen-style comments](src-cpp/)

## License

Flex Flow is licensed under the [MIT License](https://opensource.org/licenses/MIT).  
