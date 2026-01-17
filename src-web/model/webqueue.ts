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

import { IQueue, ProcTask } from "./imodel";

export class WebQueue implements IQueue
{
    handle(): number {
        throw new Error("Method not implemented.");
    }

    listPending(): Promise<number[]> {
        throw new Error("Method not implemented.");
    }
    listFinished(): Promise<number[]> {
        throw new Error("Method not implemented.");
    }
    pendingDetails(id: number): Promise<ProcTask> {
        throw new Error("Method not implemented.");
    }
    finishedDetails(id: number): Promise<ProcTask> {
        throw new Error("Method not implemented.");
    }
    currentTask(): Promise<ProcTask> {
        throw new Error("Method not implemented.");
    }
    addTask(task: ProcTask): Promise<void> {
        throw new Error("Method not implemented.");
    }
    readCurrentOutput(): Promise<string[]> {
        throw new Error("Method not implemented.");
    }
    start(): Promise<void> {
        throw new Error("Method not implemented.");
    }
    stop(): Promise<void> {
        throw new Error("Method not implemented.");
    }
    isRunning(): Promise<boolean> {
        throw new Error("Method not implemented.");
    }
    clearPending(): Promise<void> {
        throw new Error("Method not implemented.");
    }
    clearFinished(): Promise<void> {
        throw new Error("Method not implemented.");
    }
    removeTask(id: number): Promise<void> {
        throw new Error("Method not implemented.");
    }
}
