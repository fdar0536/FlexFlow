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

import { Injectable, signal, PLATFORM_ID, inject } from '@angular/core';
import { isPlatformBrowser } from '@angular/common';
import { MatDialog } from '@angular/material/dialog';

import { CommonDialog } from '../components/common-dialog';
import { ConnectInfo } from "./connectinfo";
import { Logger } from "./logger";
import { AppConfig } from "./appconfig";

@Injectable
({
    providedIn: 'root'
})

export class Global
{
    constructor()
    {
        this.logger = inject(Logger);
        this.logger.debug("Global.constructor");
        if (isPlatformBrowser(this.platformId))
        {
            const hasTauriInternal = !!(window as any).__TAURI_INTERNALS__;
            this.isTauri = hasTauriInternal;
            this.isBrowser = !hasTauriInternal;
            this.setupExit();
        }

        this.connectInfo = new ConnectInfo(this);
        this.config = new AppConfig(this);
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
        this.logger.debug("Global.setupExit");
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
        this.logger.debug("Global.onExitEvent");
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
            
            dialogRef.afterClosed().subscribe(async result =>
            {
                if (result === true)
                {
                    this.logger.info("Saving configuration before exit...");
                    await this.config.save();
                    await invoke("quit", { code: 0 });
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

    config: AppConfig;
    private logger: Logger;
}
