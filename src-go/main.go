/*
 * Flex Flow
 * Copyright (c) 2026-present fdar0536
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package main

import (
	"fmt"
	"context"
	"net/http"

	"github.com/grpc-ecosystem/grpc-gateway/v2/runtime"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
	"github.com/alexflint/go-arg"

	pb "github.com/fdar0536/FlexFlowGateway/protos"
)

var(
	serverAddr = "localhost:9090"
	listenAddr = "localhost:8080"
)

func run() error {
	ctx := context.Background()
	ctx, cancel := context.WithCancel(ctx)
	defer cancel()

	// 1. add new ServeMux
	mux := runtime.NewServeMux()

	// 2. setup
	opts := []grpc.DialOption{grpc.WithTransportCredentials(insecure.NewCredentials())}

	// 3. register Handler, let http request forward to grpc server
	err := pb.RegisterAccessHandlerFromEndpoint(ctx, mux, serverAddr, opts)
	if err != nil {
		return err
	}

	err = pb.RegisterQueueListHandlerFromEndpoint(ctx, mux, serverAddr, opts)
	if err != nil {
		return err
	}

	err = pb.RegisterQueueHandlerFromEndpoint(ctx, mux, serverAddr, opts)
	if err != nil {
		return err
	}

	fmt.Printf("FlexFlow Gateway start successfully, listen on: %s\n", listenAddr)
	return http.ListenAndServe(listenAddr, mux)
}

func main() {

	var args struct {
		ServerIp    string `arg:"-s,--server-ip" help:"grpc server ip"`
		ServerPort  uint16 `arg:"-p,--server-port" help:"grpc server port"`
		ListenIp    string `arg:"-l,--listen-ip" help:"http gateway listen ip"`
		ListenPort  uint16 `arg:"-P,--listen-port" help:"http gateway listen port"`
	}

	// 賦予預設值
	args.ServerIp = "127.0.0.1"
	args.ServerPort = 9090
	args.ListenIp = "127.0.0.1"
	args.ListenPort = 8080

	arg.MustParse(&args)
	serverAddr = args.ServerIp + ":" + fmt.Sprint(args.ServerPort)
	listenAddr = args.ListenIp + ":" + fmt.Sprint(args.ListenPort)

	if err := run(); err != nil {
		panic(err)
	}
}
