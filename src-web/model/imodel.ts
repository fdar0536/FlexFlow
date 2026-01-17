/*
 * Flex Flow
 * Copyright (c) 2025-present fdar0536
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

export type Handle = number;

export interface IConnect
{
    destroy(): Promise<void>;
    handle(): Handle;
    startConnect(target: string, port: number): Promise<void>;
    targetPath(): Promise<string>;
}

export interface IQueueList
{
    destroy(): Promise<void>;
    createQueue(name: string): Promise<void>;
    listQueue(): Promise<string[]>;
    deleteQueue(name: string): Promise<void>;
    renameQueue(old_name: string, new_name: string): Promise<void>;
    getQueue(name: string): Promise<Handle>;
    returnQueue(queue: Handle): Promise<void>;
}

export interface ProcTask
{
    execName: string;
    args: string[];
    workDir: string | null;
    id: number;
    exitCode: number;
    isSuccess: boolean;
}

export interface IQueue
{
    handle(): Handle;
    listPending(): Promise<number[]>;
    listFinished(): Promise<number[]>;
    pendingDetails(id: number): Promise<ProcTask>;
    finishedDetails(id: number): Promise<ProcTask>;
    currentTask(): Promise<ProcTask>;
    addTask(task: ProcTask): Promise<void>;
    readCurrentOutput(): Promise<string[]>;
    start(): Promise<void>;
    stop(): Promise<void>;
    isRunning(): Promise<boolean>;
    clearPending(): Promise<void>;
    clearFinished(): Promise<void>;
    removeTask(id: number): Promise<void>;
}

export enum ConnectMode
{
    GRPC,
    Local,
    Web
}
