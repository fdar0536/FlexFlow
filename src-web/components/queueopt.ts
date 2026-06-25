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

import {Component, inject, ChangeDetectionStrategy, input, signal, effect} from "@angular/core";
import { MatTabsModule } from '@angular/material/tabs';
import { MatIconModule } from '@angular/material/icon';
import { MatDialog } from "@angular/material/dialog";
import { MatButtonModule } from '@angular/material/button';
import { MatTooltipModule } from '@angular/material/tooltip';

import { Global } from "../model/global";
import { MessageDialog } from "./message-dialog";
import { CommonLayoutComponent } from "./common-layout";
import { TaskDetailsDialog } from "./taskdetails-dialog";
import { AddTaskDialog } from "./addtask-dialog";
import { TaskDetailsRes } from "./queuestatus";

interface IsRunningRes
{
    isRunning: boolean;
}

@Component
({
    selector: "QueueOpt",
    templateUrl: "./queueopt.html",
    changeDetection: ChangeDetectionStrategy.Eager,
    imports: [
        MatTabsModule,
        MatIconModule,
        CommonLayoutComponent,
        MatButtonModule,
        MatTooltipModule
    ],
    styleUrl: "../model/global.css",
})

export class QueueOptComponent
{
    onAddClicked = async() =>
    {
        this.global.logger.debug("QueueOptComponent.onAddClicked");
        const dialogRef = this.dialog.open(AddTaskDialog);
        dialogRef.afterClosed().subscribe(async(result) =>
        {
            if (result === undefined || result === null)
            {
                return;
            }

            // add task to server
            this.global.queueConnected.set(false);
            try
            {
                const res = await this.global.api.post('/queue/addtask',
                {
                    name: this.global.queueName(),
                    workDir: result.workDir,
                    execName: result.execName,
                    args: result.args.split(' ')
                });

                this.dialog.open(MessageDialog,
                {
                    data:
                    {
                        isInfo: true,
                        title: 'Success',
                        message: 'Task added successfully, ID is: ' + res.data.ID.toString()
                    }
                });
            }
            catch
            {
                this.dialog.open(MessageDialog,
                {
                    data:
                    {
                        isInfo: true,
                        title: 'Error',
                        message: 'Failed to add task'
                    }
                });
            }

            this.global.queueConnected.set(true);
        });
    } // onAddClicked

    onStatusClicked = async() =>
    {
        this.global.logger.debug("QueueOptComponent.onStatusClicked");
        this.global.queueConnected.set(false);

        try
        {
            const res = await this.global.api.get<IsRunningRes>('/queue/isrunning',
            {
                params:
                {
                    name: this.global.queueName()
                }
            });

            this.dialog.open(MessageDialog,
            {
                data:
                {
                    isInfo: true,
                    title: 'Status',
                    message: res.data.isRunning ? 'Queue is running' : 'Queue is not running'
                }
            });
        }
        catch
        {
            this.dialog.open(MessageDialog,
            {
                data:
                {
                    isInfo: true,
                    title: 'Error',
                    message: 'Failed to get status'
                }
            });
        }

        this.global.queueConnected.set(true);
    } // onStatusClicked

    onCurrentTaskClicked = async() =>
    {
        this.global.logger.debug("QueueOptComponent.onCurrentTaskClicked");
        this.global.queueConnected.set(false);
        try
        {
            const res = await this.global.api.get<TaskDetailsRes>('/queue/currenttask',
            {
                params:
                {
                    name: this.global.queueName()
                }
            });

            const data = res.data;
            this.dialog.open(TaskDetailsDialog,
            {
                data:
                {
                    title: 'Task Details',
                    workDir: data.workDir,
                    execName: data.execName,
                    args: data.args,
                    exitCode: 'Running',
                    ID: data.ID
                }
            });
        }
        catch
        {
            this.dialog.open(MessageDialog,
            {
                data:
                {
                    isInfo: true,
                    title: 'Error',
                    message: 'Failed to get current running task or queue is not running'
                }
            });
        }

        this.global.queueConnected.set(true);
    } // onCurrentTaskClicked

    onStartClicked = async() =>
    {
        this.global.logger.debug("QueueOptComponent.onStartClicked");
        this.global.queueConnected.set(false);
        try
        {
            await this.global.api.post('/queue/start',
            {
                name: this.global.queueName()
            });
        }
        catch
        {
            this.dialog.open(MessageDialog,
            {
                data:
                {
                    isInfo: true,
                    title: 'Error',
                    message: 'Failed to start queue or queue is running'
                }
            });
        }

        this.global.queueConnected.set(true);
    }

    onStopClicked = async() =>
    {
        this.global.logger.debug("QueueOptComponent.onStopClicked");
        this.global.queueConnected.set(false);
        try
        {
            await this.global.api.post('/queue/stop',
            {
                name: this.global.queueName()
            });
        }
        catch
        {
            this.dialog.open(MessageDialog,
            {
                data:
                {
                    isInfo: true,
                    title: 'Error',
                    message: 'Failed to stop queue'
                }
            });
        }

        this.global.queueConnected.set(true);
    }

    global = inject(Global);
    private dialog = inject(MatDialog);
}
