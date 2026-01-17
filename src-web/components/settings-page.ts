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

import { Component, inject } from "@angular/core";
import { MatRadioModule } from '@angular/material/radio';
import { MatDialog } from '@angular/material/dialog';

import { Global } from "../model/global";
import { ConnectMode } from '../model/imodel';
import { CommonDialog } from '../components/common-dialog';

@Component
({
    selector: "settings-page",
    templateUrl: "./settings-page.html",
    imports: [MatRadioModule],
})

export class SettingsPage
{
    global = inject(Global);
    private dialog = inject(MatDialog);

    constructor()
    {
        this.global.cleanModel();
    }
    
    onRadioChanged = (value: number) =>
    {
        if (this.global.isTauri &&
            value === ConnectMode.Web)
        {
            this.dialog.open(CommonDialog,
            {
                data:
                {
                    type: 'info',
                    title: 'Performance Warning',
                    message: 'This feature is only for debug purpose.'
                },
            });
        }

        this.global.connectMode.set(value);
    }
}
