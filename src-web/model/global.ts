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

import { CommonDialog } from '../components/common-dialog';
import { ConnectInfo } from "./connectinfo";
import { ConnectMode } from "./imodel";

export type ConnectionProfile =
{
    name: string;
    target: string;
    port: number;
}

export type AppConfig = 
{
    profiles: ConnectionProfile[];
    current_profile: string | null;
}

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

        this.connectInfo = new ConnectInfo(this);
        if (this.isBrowser)
        {
            this.connectInfo.mode.set(ConnectMode.Web);
        }

        this.destroyRef.onDestroy(() =>
        {
            if (this.isTauri)
            {
                invoke("save_config", { config: this.config });
            } 
        });
    }

    // tauri
    private platformId = inject(PLATFORM_ID);
    readonly isTauri: boolean = true;
    readonly isBrowser: boolean = false;

    status = signal<string>('Not connected');

    // model
    connectInfo: ConnectInfo;

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

    // status
    config: AppConfig | null = null;
    isConfigReady = signal<boolean>(false, { equal: () => false });
    loadConfig = async() =>
    {
        if (this.isTauri)
        {
            try
            {
                this.config = await invoke<AppConfig>("get_config");
            }
            catch (e)
            {
                this.config = null;
                console.log(e)
            }
        }
        else
        {
            this.config = null;
        }

        this.isConfigReady.set(true);
    }
}
