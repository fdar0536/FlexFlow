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

import {Component, inject, ChangeDetectionStrategy} from "@angular/core";
import {Router, RouterLink} from '@angular/router';
import {MatTabsModule} from '@angular/material/tabs';
import {MatIconModule} from '@angular/material/icon';
import { MatDialog } from "@angular/material/dialog";

import { Global } from "../model/global";
import { MessageDialog } from "../components/message-dialog";

@Component
({
    selector: "header-component",
    templateUrl: "./header-component.html",
    changeDetection: ChangeDetectionStrategy.Eager,
    imports: [
        MatTabsModule,
        MatIconModule,
        RouterLink
    ],
})

export class HeaderComponent
{
    onLogoutClicked = (e: Event) =>
    {
        this.global.logger.debug("HeaderComponent.onLogoutClicked");
        e.preventDefault();
        const dialogRef = this.dialog.open(MessageDialog,
        {
            data:
            {
                isInfo: false,
                title: 'Logout',
                message: 'Are you sure you want to Logout?'
            },
        });
        
        dialogRef.afterClosed().subscribe(result =>
        {
            if (!result)
            {
                return;
            }

            // log out the api
            try
            {
                this.global.api.post('/logout');
            }
            catch
            {
                this.dialog.open(MessageDialog,
                {
                    data:
                    {
                        isInfo: true,
                        title: 'Error',
                        message: 'Failed to logout'
                    }
                });

                return;
            }

            this.global.connected.set(false);
            this.router.navigate(['/login']);
        });
    }

    global = inject(Global);
    
    private dialog = inject(MatDialog);
    private router = inject(Router);
}
