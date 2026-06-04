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

import { Component, inject, ChangeDetectionStrategy } from "@angular/core";
import { FormsModule } from '@angular/forms';
import { MatFormFieldModule } from "@angular/material/form-field";
import { MatSelectModule } from "@angular/material/select";
import { MatButtonModule } from '@angular/material/button';
import { MatDialog } from "@angular/material/dialog";

import { Global } from "../model/global";
import { CommonLayoutComponent } from "./common-layout";
import { Logger } from "../model/logger";
import { InputDialog } from "./input-dialog";
import { MessageDialog } from "./message-dialog";

@Component
({
    selector: "queuelist-component",
    templateUrl: "./queuelist-component.html",
    imports: [
        CommonLayoutComponent,
        MatFormFieldModule,
        MatSelectModule,
        MatButtonModule,
        FormsModule
    ],
    changeDetection: ChangeDetectionStrategy.Eager,
    styleUrl: "../model/disable.css",
})

export class QueueListComponent
{
    constructor()
    {
        this.logger.debug("QueueListComponent.constructor");
        this.onRefreshClicked();
    }

    onRefreshClicked = async() =>
    {
        this.logger.debug("QueueListComponent.onRefreshClicked");
        this.global.disabled.set(true);
        this.list = [];
        try
        {
            // the result is chunked, handle it as string
            const res = await this.global.api.get<string>(
                '/queuelist/list',
                {
                    responseType: 'text' 
                }
            );
            
            // the data structure of 'res' is
            /*
            {"result":{"name":"test2"}}
            {"result":{"name":"test"}}
            {"result":{"name":"Cache"}}
            */
            const rawData = res.data;
            if (rawData)
            {
                // split string
                const lines = rawData.split(/\r?\n/);
                for (const line of lines)
                {
                    // ensure its not empty
                    if (line.trim() === '') continue;

                    try
                    {
                        // string as JSON
                        const parsed = JSON.parse(line);
                        if (parsed?.result?.name)
                        {
                            this.list.push(parsed.result.name);
                        }
                    }
                    catch (error)
                    {
                        // avoid error to crash whole app
                        this.logger.error('parse Chunk failed:', error, 'raw data:', line);
                    }
                } // for (const line of lines)
            } // if (rawData)
        }
        catch
        {
            // do nothing
        }

        this.global.disabled.set(false);
    } // onRefreshClicked

    onLoadClicked = async() =>
    {
        this.logger.debug("QueueListComponent.onLoadClicked");
        if (this.selectedItem === '')
        {
            this.logger.error("QueueListComponent.onLoadClicked: selectedItem is empty");
            return;
        }

        this.global.disabled.set(true);
        this.global.queueConnected.set(false);
        try
        {
            await this.global.api.get('/queuelist/get', {
                params:
                {
                    name: this.selectedItem
                }
            });

            this.global.queueConnected.set(true);
            this.global.queueName.set(this.selectedItem);
        }
        catch
        {
            this.dialog.open(MessageDialog,
            {
                data:
                {
                    type: 'info',
                    title: 'Error',
                    message: 'Failed to load queue'
                }        
            });

            this.global.queueConnected.set(false);
            this.global.queueName.set('');
        }

        this.global.disabled.set(false);
    }

    onCreateClicked = async() =>
    {
        this.logger.debug("QueueListComponent.onAddClicked");
        const dialogRef = this.dialog.open(InputDialog,
        {
            data:
            {
                title: 'New Queue',
                name: 'Name'
            },
        });

        dialogRef.afterClosed().subscribe(async(result) =>
        {
            if (result === undefined || result === null)
            {
                return;
            }

            if (result === '')
            {
                return;
            }

            // create new queue
            this.global.disabled.set(true);
            try
            {
                await this.global.api.post('/queuelist/create',
                {
                    name: result
                });
            }
            catch
            {
                this.dialog.open(MessageDialog,
                {
                    data:
                    {
                        type: 'info',
                        title: 'Error',
                        message: 'Failed to create queue'
                    }        
                });
            }

            this.global.disabled.set(false);
            this.onRefreshClicked();
        });
    } // onCreateClicked

    onDeleteClicked = async() =>
    {
        this.logger.debug("QueueListComponent.onDeleteClicked");
        if (this.selectedItem === '')
        {
            this.logger.error("QueueListComponent.onDeleteClicked: selectedItem is empty");
            return;
        }

        // delete the queue
        this.global.disabled.set(true);
        try
        {
            await this.global.api.delete('/queuelist/delete',
            {
                params:
                {
                    name: this.selectedItem
                }
            });

            this.global.disabled.set(false);
            this.onRefreshClicked();
        }
        catch
        {
            this.dialog.open(MessageDialog,
            {
                data:
                {
                    type: 'info',
                    title: 'Error',
                    message: 'Failed to delete queue'
                }        
            });

            this.global.disabled.set(false);
        }
    } // onDeleteClicked

    onRenameClicked = async() =>
    {
        this.logger.debug("QueueListComponent.onRenameClicked");
        if (this.selectedItem === '')
        {
            this.logger.error("QueueListComponent.onRenameClicked: selectedItem is empty");
            return;
        }

        // rename the queue
        const dialogRef = this.dialog.open(InputDialog,
        {
            data:
            {
                title: 'Rename Queue',
                name: 'New Name'
            },
        });

        dialogRef.afterClosed().subscribe(async(result) =>
        {
            if (result === undefined || result === null)
            {
                return;
            }

            if (result === '')
            {
                return;
            }

            this.global.disabled.set(true);
            try
            {
                await this.global.api.post('/queuelist/rename',
                {
                    oldName: this.selectedItem,
                    newName: result
                });
                this.global.disabled.set(false);
                this.onRefreshClicked();
            }
            catch
            {
                this.dialog.open(MessageDialog,
                {
                    data:
                    {
                        type: 'info',
                        title: 'Error',
                        message: 'Failed to rename queue'
                    }        
                });

                this.global.disabled.set(false);
            }
        }); // dialogRef.afterClosed().subscribe
    } // onRenameClicked

    global = inject(Global);
    logger = inject(Logger);
    dialog = inject(MatDialog);
    list: string[] = [];
    selectedItem = '';
}