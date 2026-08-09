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
    "context"
    "fmt"
    "net/http"
    "strings"

    "github.com/alexflint/go-arg"
    "github.com/grpc-ecosystem/grpc-gateway/v2/runtime"
    "google.golang.org/grpc"
    "google.golang.org/grpc/credentials/insecure"
    "google.golang.org/protobuf/proto"

    pb "github.com/fdar0536/FlexFlowGateway/protos"
)

var(
    serverAddr = "localhost:9090"
    listenAddr = "localhost:8080"
    frontendDir = ""

	Version = "development"
    Branch = "main"
    Commit = "HEAD"
)

func run() error {
    ctx := context.Background()
    ctx, cancel := context.WithCancel(ctx)
    defer cancel()

    // 1. add new grpc ServeMux
    mux := runtime.NewServeMux(
        runtime.WithForwardResponseOption(ffHttpResponseModifier),
    )

    // 2. add regular http mux
    mainMux := http.NewServeMux()

    // 3. setup
    opts := []grpc.DialOption{grpc.WithTransportCredentials(insecure.NewCredentials())}

    // 4. register Handler, let http request forward to grpc server
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

    var fs http.Handler
    if (frontendDir != "") {
        mainMux.Handle("/api/", mux)
        fs = http.FileServer(http.Dir(frontendDir))
    }

    if (frontendDir == "") {
        mainMux.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
            mux.ServeHTTP(w, r)
        })
    } else {
        mainMux.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
            // ban path "API" and let Angular SPA frontend handle it
            if strings.HasPrefix(r.URL.Path, "/api/") {
                mux.ServeHTTP(w, r)
                return
            }

            // Redirect all missing static files to index.html
            // (to maintain proper Angular routing)
            fs.ServeHTTP(w, r)
        })
    }

    fmt.Printf("FlexFlow Gateway start successfully, listen on: %s\n", listenAddr)
    return http.ListenAndServe(listenAddr, cookieToMetadataMiddleware(mainMux))
}

func ffHttpResponseModifier(ctx context.Context,
                            w http.ResponseWriter,
                            resp proto.Message) error {
    md, ok := runtime.ServerMetadataFromContext(ctx)
    if !ok {
        return nil
    }

    tokens := md.HeaderMD.Get("x-auth-token")
    tokenTimeOutStr := md.HeaderMD.Get("x-auth-token-timeout")
    if (len(tokenTimeOutStr) <= 0 || len(tokens) <= 0) {
        return nil
    }

    var tokenTimeout int = 0
    _, err := fmt.Sscan(tokenTimeOutStr[0], &tokenTimeout);
    if (err != nil) {
        fmt.Println(err)
        return nil
    }

    token := tokens[0]
    // build HttpOnly cookie
    cookie := &http.Cookie{
        Name:     "x-auth-token",
        Value:    token,
        Path:     "/",
        HttpOnly: true, // for prevent xss
        SameSite: http.SameSiteLaxMode,
        MaxAge:   tokenTimeout,
    }

    // write HTTP Response
    http.SetCookie(w, cookie)

    // remove x-auth-token amd x-auth-token-timeout from http header
    w.Header().Del("Grpc-Metadata-X-Auth-Token")
    w.Header().Del("Grpc-Metadata-X-Auth-Token-Timeout")

    return nil
}

func cookieToMetadataMiddleware(next http.Handler) http.Handler {
    return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
        // 1. try to get "x-auth-token" Cookie from browser data
        if cookie, err := r.Cookie("x-auth-token"); err == nil {
            
            // 2. write to HTTP Header directly, and add "Grpc-Metadata-" prefix
            // gRPC-Gateway will convert "Grpc-Metadata-" automatically to gRPC Metadata
            r.Header.Set("Grpc-Metadata-x-auth-token", cookie.Value)
        }
        
        // to next step（sent to gRPC-Gateway and convert to gRPC request）
        next.ServeHTTP(w, r)
    })
}

func main() {
    fmt.Printf("Flex Flow %s HTTP Gateway\n", Version)
    fmt.Println("")

    var args struct {
        ServerPort  uint16 `arg:"-p,--server-port" help:"gRPC server port"`
        ListenPort  uint16 `arg:"-P,--listen-port" help:"HTTP gateway listening port"`
        FrontendDir string `arg:"-f,--frontend-dir" help:"Path to the Web UI directory. If omitted, only the HTTP gateway is started."`
        PrintVersion bool  `arg:"-v,--version" help:"Print version and exit."`
    }

    // set default
    args.ServerPort = 12345
    args.ListenPort = 8080
    args.FrontendDir = ""
    args.PrintVersion = false

    arg.MustParse(&args)
    if (args.PrintVersion) {
        fmt.Printf("Branch: %s\n", Branch)
        fmt.Printf("Commit: %s\n", Commit)
        return
    }

    serverAddr = "127.0.0.1:" + fmt.Sprint(args.ServerPort)
    listenAddr = "127.0.0.1:" + fmt.Sprint(args.ListenPort)
    frontendDir = args.FrontendDir

    if err := run(); err != nil {
        panic(err)
    }
}
