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

import { IQueueList, Handle } from "./imodel";

export class TauriQueueList implements IQueueList
{
    public static create = async(conn: Handle): Promise<TauriQueueList> =>
    {
        var list: Handle =
        await invoke<Handle>("queue_list_init", {connect_handle: conn});
        return new TauriQueueList(list);
    }

    public destroy = async(): Promise<void> =>
    {
        return await invoke<void>("queue_list_destroy", { h: this.list });
    }

    public createQueue = async(name: string): Promise<void> =>
    {
        return await invoke<void>("queue_list_create_queue",
            { h: this.list, name });
    }

    public listQueue = async(): Promise<string[]> =>
    {
        return await invoke<string[]>
        ("queue_list_list_queue", { h: this.list });
    }

    public deleteQueue = async(name: string): Promise<void> =>
    {
        return await invoke<void>
        ("queue_list_delete_queue", { h: this.list, name });
    }

    public renameQueue =
    async(old_name: string, new_name: string): Promise<void> =>
    {
        return await invoke<void>
        ("queue_list_rename_queue", { h: this.list, old_name, new_name });
    }

    public getQueue = async(name: string): Promise<Handle> =>
    {
        return await invoke<Handle>
        ("queue_list_get_queue", { h: this.list, name});
    }

    public returnQueue = async(queue: Handle): Promise<void> =>
    {
        return await invoke<void>
        ("queue_list_return_queue", { h: this.list, queue});
    }

    private constructor(list: Handle)
    {
        this.list = list;
    }

    private list: Handle = 0
}
