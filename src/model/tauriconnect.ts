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

import { IConnect, Handle } from "./imodel";

export class TauriConnect implements IConnect
{
    static create = async(backend: number): Promise<TauriConnect> =>
    {
        var h: Handle = await invoke<Handle>("connect_init", { backend });
        return new TauriConnect(h);
    }

    destroy = async(): Promise<void> =>
    {
        return await invoke<void>("connect_destroy", { h: this.conn });;
    }

    startConnect =
    async(target: string, port: number): Promise<void> =>
    {
        return await invoke<void>("connect_start_connect",
            {h: this.conn, target, port});
    }

    targetPath = async(): Promise<string> =>
    {
        return await invoke<string>("connect_target_path", { h: this.conn });
    }

    private constructor(conn: Handle)
    {
        this.conn = conn;
    }

    private conn: Handle = 0;
};
