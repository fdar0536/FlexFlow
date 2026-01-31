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

import { IConnect, Handle, ExitCode } from "./models";
import { Logger } from "./logger";

export class TauriConnect implements IConnect
{
    static create = async(backend: number, logger: Logger, code: ExitCode): Promise<TauriConnect | null> =>
    {
        code.clear();
        var h: Handle;
        try
        {
            h = await invoke<Handle>("connect_init", { backend });
        }
        catch (e: any)
        {
            code.hasError = true;
            code.msg = e.toString();
            logger.error(e.toString());
            return null;
        }
        
        return new TauriConnect(h, logger);
    }

    destroy = async(code: ExitCode): Promise<void> =>
    {
        code.clear();
        try
        {
            await invoke<void>("connect_destroy", { h: this.conn });
        }
        catch (e: any)
        {
            code.hasError = true;
            code.msg = e.toString();
            this.logger.error(e.toString());    
        }
    }
    
    handle(): Handle
    {
        return this.conn;
    }

    startConnect =
    async(target: string, port: number, code: ExitCode): Promise<void> =>
    {
        code.clear();
        this.logger.debug("TauriConnect.startConnect");
        this.logger.debug("target is: " + target);
        this.logger.debug("port is: " + port);
        try
        {
            await invoke<void>("connect_start_connect",
            {h: this.conn, target, port});
        }
        catch (e: any)
        {
            code.hasError = true;
            code.msg = e.toString();
            this.logger.error(e.toString());
        }
    }

    targetPath = async(code: ExitCode): Promise<string> =>
    {
        code.clear();
        try
        {
            return await invoke<string>("connect_target_path", { h: this.conn });
        }
        catch (e: any)
        {
            code.hasError = true;
            code.msg = e.toString();
            this.logger.error(e.toString());
            return "";
        }
    }

    private constructor(conn: Handle, logger: Logger)
    {
        this.conn = conn;
        this.logger = logger;
    }

    private conn: Handle = 0;
    private logger: Logger;
};
