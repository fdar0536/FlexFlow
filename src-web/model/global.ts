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
import { listen } from '@tauri-apps/api/event';

import { Injectable, signal, PLATFORM_ID, inject, DestroyRef } from '@angular/core';
import { isPlatformBrowser } from '@angular/common';
import { MatDialog } from '@angular/material/dialog';

import { Mutex } from 'async-mutex';

import { CommonDialog } from '../components/common-dialog';
import { IConnect, IQueueList, IQueue, ConnectMode } from './imodel';

@Injectable
({
    providedIn: 'root'
})

export class Global
{
    private destroyRef = inject(DestroyRef);
    constructor()
    {
        if (isPlatformBrowser(this.platformId))
        {
            const hasTauriInternal = !!(window as any).__TAURI_INTERNALS__;
            this.isTauri = hasTauriInternal;
            this.isBrowser = !hasTauriInternal;
            this.setupExit();
        }

        if (this.isBrowser)
        {
            this.connectMode.set(ConnectMode.Web);
        }

        this.destroyRef.onDestroy(() =>
        {
            this.cleanModel();
        });
    }

    // tauri
    private platformId = inject(PLATFORM_ID);
    readonly isTauri: boolean = true;
    readonly isBrowser: boolean = false;

    status = signal<string>('Not connected');

    // model
    // todo: optimize it
    private mutex = new Mutex();
    connect: IConnect | null = null;
    queueList: IQueueList | null = null;
    queue: IQueue | null = null;
    connectMode = signal<ConnectMode>(ConnectMode.GRPC);
    connectTarget = signal<string>("");
    connectPort = signal<number>(0);
    isConnected = signal<boolean>(false);

    cleanModel = async() =>
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
        this.status.set("Not connected");
        release();
    }

    // handle exit
    setupExit = async() =>
    {
        if (this.isTauri)
        {
            await listen('request-exit', (event) =>
            {
                this.onExitEvent();
            });
        }
    }

    private dialog = inject(MatDialog);
    onExitEvent = async() =>
    {
        if (this.isTauri)
        {
            const dialogRef = this.dialog.open(CommonDialog,
            {
                data:
                {
                    type: 'confirm',
                    title: 'Exit',
                    message: 'Are you sure you want to exit?'
                },
            });
            
            dialogRef.afterClosed().subscribe(result =>
            {
                if (result === true)
                {
                    invoke("quit");
                }
            });
        }
        else
        {
            this.dialog.open(CommonDialog, {
                data:
                {
                    type: 'info',
                    title: 'Oops!',
                    message: 'Browser is not supported.'
                },
            });
        }
    } // onExitEvent
}
