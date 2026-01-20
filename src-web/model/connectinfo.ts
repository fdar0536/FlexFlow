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

import { Mutex } from 'async-mutex';
import { signal, DestroyRef, inject } from '@angular/core';

import { IConnect, IQueueList, IQueue, ConnectMode } from './imodel';
import { Global } from './global';

export class ConnectInfo
{
    constructor(global: Global)
    {
        this.global = global;
        this.destroyRef.onDestroy(() =>
        {
            this.clean();
        });
    }

    connect: IConnect | null = null;
    queueList: IQueueList | null = null;
    queue: IQueue | null = null;
    mode = signal<ConnectMode>(ConnectMode.GRPC);
    target = signal<string>("");
    port = signal<number>(0);
    isConnected = signal<boolean>(false);

    clean = async() =>
    {
        const release = await this.mutex.acquire();
        if (this.queueList)
        {
            if (this.queue)
            {
                var handle = this.queue.handle();
                try
                {
                    await this.queueList.returnQueue(handle);
                }
                catch (e)
                {
                    console.error(e);
                }
                this.queue = null;
            }

            await this.queueList.destroy();
            this.queueList = null;
        }

        if (this.connect)
        {
            await this.connect.destroy();
            this.connect = null;
        }
        
        this.isConnected.set(false);
        this.global.status.set("Not connected");
        release();
    }

    private mutex = new Mutex();
    private global: Global;
    private destroyRef = inject(DestroyRef);
}
