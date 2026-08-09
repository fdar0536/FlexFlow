# User Guide

Flex Flow provides three executable components:  
- Server
- Command-Line Interface (CLI)
- HTTP gRPC Gateway

This guide explains how to use each component.  

For detailed command-line options, refer to each executable's `--help` output.  
Configuration options are documented in the corresponding example configuration files.  

## Getting Started

- Start the Flex Flow server before using the CLI or HTTP gRPC Gateway. 
- Refer to the [README](../ReadMe.md) for installation and initial setup.

## Server

- To start the server
```sh
cp server-config.example.yaml server-config.yaml
FlexFlowServer --config server-config.yaml
```
- For all available command-line options: `FlexFlowServer --help`
- For configuration options, refer to: [server-config.example.yaml](../src-cpp/server-config.example.yaml).

## CLI

- For all available command-line options: `FlexFlowCLI --help`
- Command-specific help is also available: `FlexFlowCLI <command> --help`
- For client profiles and connection settings, refer to: [client-profile.example.yaml](../src-cpp/client-profile.example.yaml).

## HTTP gRPC Gateway

- The gateway translates HTTP requests into gRPC requests and forwards them to a running Flex Flow server.
- For all available command-line options: `FlexFlowGateway --help`
- For API definitions and HTTP mappings, refer to the Protocol Buffer files: [protos](../protos/).

## Authentication

The command-line client authenticates through gRPC metadata.  
  
The HTTP gateway stores authentication information in an HTTP-only cookie.  
Web clients do not directly read or modify the authentication token.  

For detailed authentication behavior, refer to: [Authentication API](../protos/access.proto)
