/*
 * Flex Flow
 * Copyright (c) 2026-present fdar0536
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

import {Component, inject} from "@angular/core";
import { Router } from '@angular/router';
import { MatDialog } from '@angular/material/dialog';

import {Global} from "../model/global";
import { CommonDialog } from '../components/common-dialog';

@Component
({
    selector: "queueList-page",
    templateUrl: "./queuelist-page.html",
    imports: [],
})

export class QueueListPage
{
    private router = inject(Router);
    private dialog = inject(MatDialog);
    global = inject(Global);

    constructor()
    {
        if (this.global.isConnected() === false)
        {
            this.dialog.open(CommonDialog,
            {
                data:
                {
                    type: 'info',
                    title: 'Oops!',
                    message: 'Not connected.'
                },
            });

            this.router.navigateByUrl("/", { replaceUrl: true });
            return;
        }

        if (this.global.queueList === null)
        {
            this.dialog.open(CommonDialog,
            {
                data:
                {
                    type: 'info',
                    title: 'Oops!',
                    message: 'Connect may be failed. Please try again.'
                }
            })

            this.router.navigateByUrl("/", { replaceUrl: true });
            return;
        }
    }
}
