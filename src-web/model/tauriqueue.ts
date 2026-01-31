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

import { invoke } from "@tauri-apps/api/core";

import { IQueue, Handle, ProcTask } from "./models";

export class TauriQueue implements IQueue
{
    constructor(queue: Handle)
    {
        this.queue = queue;
    }

    handle(): Handle
    {
        return this.queue;  
    }

    listPending = async(): Promise<number[]> =>
    {
        return await invoke<number[]>("queue_list_pending", { h: this.queue });
    }

    listFinished = async(): Promise<number[]> =>
    {
        return await invoke<number[]>("queue_list_finished", { h: this.queue });
    }

    pendingDetails = async(id: number): Promise<ProcTask> =>
    {
        return await invoke<ProcTask>
        ("queue_pending_details", { h: this.queue, id });
    }

    finishedDetails = async(id: number): Promise<ProcTask> =>
    {
        return await invoke<ProcTask>
        ("queue_finished_details", { h: this.queue, id });
    }

    currentTask = async(): Promise<ProcTask> =>
    {
        return await invoke<ProcTask>
        ("queue_current_task", { h: this.queue });
    }

    addTask = async(task: ProcTask): Promise<void> =>
    {
        return await invoke<void>
        ("queue_add_task", { h: this.queue, task });
    }

    readCurrentOutput = async(): Promise<string[]> =>
    {
        return await invoke<string[]>
        ("queue_read_current_output", { h: this.queue });
    }

    start = async(): Promise<void> =>
    {
        return await invoke<void>
        ("queue_start", { h: this.queue });
    }

    stop = async(): Promise<void> =>
    {
        return await invoke<void>
        ("queue_stop", { h: this.queue });
    }

    isRunning = async(): Promise<boolean> =>
    {
        return await invoke<boolean>
        ("queue_is_running", { h: this.queue });
    }

    clearPending = async(): Promise<void> =>
    {
        return await invoke<void>
        ("queue_clear_pending", { h: this.queue });
    }

    clearFinished = async(): Promise<void> =>
    {
        return await invoke<void>
        ("queue_clear_finished", { h: this.queue });
    }

    removeTask = async(id: number): Promise<void> =>
    {
        return await invoke<void>
        ("queue_remove_task", { h: this.queue, id });
    }

    private queue: Handle = 0;
}
