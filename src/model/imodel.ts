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

export interface IConnect
{
    startConnect(target: string, port: number): Promise<number>;
    targetPath(): string;
}

export interface IQueueList
{
    createQueue(name: string): Promise<number>;
    listQueue(): [Promise<string[]>, Promise<number>];
    deleteQueue(name: string): Promise<number>;
    renameQueue(old_name: string, new_name: string): Promise<number>;
    get_queue(name: string): [Promise<number>, Promise<number>];
    return_queue(queue: number): Promise<number>;
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
    listPending(): [Promise<number[]>, Promise<number>];
    listFinished(): [Promise<number[]>, Promise<number>];
    pendingDetails(id: number): [Promise<ProcTask>, Promise<number>];
    finishedDetails(id: number): [Promise<ProcTask>, Promise<number>];
    currentTask(): [Promise<ProcTask>, Promise<number>];
    add_task(task: ProcTask): Promise<number>;
}
