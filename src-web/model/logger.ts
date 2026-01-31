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
 * copies or s`ubstantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

import pino from 'pino';
import { Injectable, isDevMode } from "@angular/core";
import { info, debug, error, warn, trace } from "@tauri-apps/plugin-log";

@Injectable
({
    providedIn: 'root'
})

export class Logger
{
    constructor()
    {
        console.log('Logger initialized. isTauri:', this.isTauri);
    }    

    trace(msg: string, ...args: any[])
    {
        this.logger.trace(args.length ? { detail: args } : {}, msg);
    }

    debug(msg: string, ...args: any[])
    {
        this.logger.debug(args.length ? { detail: args } : {}, msg);
    }

    info(msg: string, ...args: any[])
    {
        this.logger.info(args.length ? { detail: args } : {}, msg);
    }

    warn(msg: string, ...args: any[])
    {
        this.logger.warn(args.length ? { detail: args } : {}, msg);
    }

    error(msg: string, ...args: any[])
    {
        this.logger.error(args.length ? { detail: args } : {}, msg);
    }

    fatal(msg: string, ...args: any[])
    {
        this.logger.fatal(args.length ? { detail: args } : {}, msg);
    }

    private isTauri = !!(window as any).__TAURI_INTERNALS__;
    private logger = pino
    ({
        level: isDevMode() ? 'debug' : 'info',
        browser:
        {
            asObject: true,
            write: (o) =>
            {
                console.log(o);
            },
            transmit:
            {
                level: isDevMode() ? 'debug' : 'info',
                send: async (level, logEvent) =>
                {
                    if (this.isTauri)
                    {
                        try
                        {   
                            const msg = logEvent.messages.map(m => 
                            typeof m === 'object' ? JSON.stringify(m) : m
                            ).join(' ');

                            // 對應 Pino 的 Level 到 Tauri Plugin Log
                            switch (level)
                            {
                                case 'debug': await debug(msg); break;
                                case 'info':  await info(msg);  break;
                                case 'warn':  await warn(msg);  break;
                                case 'error': await error(msg); break;
                                case 'fatal': await error(`[FATAL] ${msg}`); break;
                                default: await trace(msg);
                            }
                        } // try
                        catch (err)
                        {
                            // 如果插件沒裝好，至少還有原生 console
                            console.warn('Tauri logger plugin failed:', err);
                        } // catch
                    } // if (this.global.isTauri)
                } // send: async (level, logEvent) =>
            } // transmit:
        } // browser:
    });
}
