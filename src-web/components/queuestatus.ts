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

import {
    Component,
    inject,
    ChangeDetectionStrategy,
    input,
    signal,
    viewChild,
    ElementRef,
    computed,
    AfterViewInit,
    OnDestroy
} from "@angular/core";

import {
    interval,
    startWith,
    Subscription,
    switchMap
} from "rxjs";

import { MatTabsModule } from '@angular/material/tabs';
import { MatIconModule } from '@angular/material/icon';
import { MatDialog } from "@angular/material/dialog";
import { MatButtonModule } from '@angular/material/button';

import { Global } from "../model/global";
import { MessageDialog } from "./message-dialog";
import { CommonLayoutComponent } from "./common-layout";
import { TaskDetailsDialog } from "./taskdetails-dialog";

export interface TaskDetailsRes
{
    workDir: string;
    execName: string;
    args: string[];
    exitCode: number;
    ID: number;
}

@Component
({
    selector: "QueueStatus",
    templateUrl: "./queuestatus.html",
    changeDetection: ChangeDetectionStrategy.Eager,
    imports: [
        MatTabsModule,
        MatIconModule,
        CommonLayoutComponent,
        MatButtonModule
    ],
    styleUrl: "../model/global.css",
})

export class QueueStatusComponent implements AfterViewInit, OnDestroy
{
    // for angular
    ngAfterViewInit()
    {
        this.global.logger.debug("QueueStatusComponent.ngAfterViewInit");

        this.dataSubscription = interval(5000).pipe(
            startWith(0),
            switchMap(() => this.onRefreshClicked()
            )).subscribe(
            {
                error: (err) => this.global.logger.error('Update Failed', err),
            });
    }

    ngOnDestroy()
    {
        this.global.logger.debug("QueueStatusComponent.ngOnDestroy");
        this.dataSubscription.unsubscribe();
    }

    // Signals used to manage selection status and dragging
    selectedIds = signal<Set<number>>(new Set());
    isDragging = signal(false);
    dragBoxStyle = signal<Record<string, string>>({});

    boxContainer = viewChild<ElementRef>('boxContainer');

    onItemClicked = async(event: MouseEvent, id: number) =>
    {
        this.global.logger.debug("QueueStatusComponent.onItemClicked");
        if (!this.isPending)
        {
            return;
        }

        event.stopPropagation();

        const currentSelected = new Set(this.selectedIds());
        if (event.ctrlKey || event.metaKey)
        {
            // 如果按著 Ctrl，切換選取狀態
            if (currentSelected.has(id))
            {
                currentSelected.delete(id);
            }
            else
            {
                currentSelected.add(id);
            }
        }
        else
        {
            currentSelected.clear();
            currentSelected.add(id);
        }

        this.selectedIds.set(currentSelected);
    }

    onItemDblClicked = async(item: number) =>
    {
        this.global.queueConnected.set(false);
        try
        {
            const res = await this.global.api.get<TaskDetailsRes>(this.detailsApi(),
            {
                params:
                {
                    name: this.global.queueName(),
                    ID: item
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
                    exitCode: this.isPending() ? 'Pending' : data.exitCode,
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
                    message: 'Failed to get task details'
                }
            });
        }

        this.global.queueConnected.set(true);
    }

    onMouseDown = (event: MouseEvent) =>
    {
        this.global.logger.debug("QueueStatusComponent.onMouseDown");
        if (!this.isPending)
        {
            return;
        }

        if ((event.target as HTMLElement).closest('button')) return;
        if (event.button !== 0) return;

        if (!event.ctrlKey)
        {
            this.selectedIds.set(new Set());
        }

        const container = this.boxContainer()?.nativeElement;
        if (!container) return;

        const rect = container.getBoundingClientRect();
        this.startX = event.clientX - rect.left;
        this.startY = event.clientY - rect.top;

        this.isDragging.set(true);

        const mouseMoveHandler = (e: MouseEvent) => this.onMouseMove(e, rect, container);
        const mouseUpHandler = () =>
        {
            this.isDragging.set(false);
            window.removeEventListener('mousemove', mouseMoveHandler);
            window.removeEventListener('mouseup', mouseUpHandler);
        };

        window.addEventListener('mousemove', mouseMoveHandler);
        window.addEventListener('mouseup', mouseUpHandler);
    }

    onRefreshClicked = async() =>
    {
        this.global.logger.debug("QueueStatusComponent.onRefreshClicked");

        if (this.global.queueDisabled() ||
            this.isRefreshing())
        {
            return;
        }

        if (this.global.queueName() === "")
        {
            this.list.set([]);
            return;
        }

        this.isRefreshing.set(true);

        // list the tasks in queue
        this.list.set([]);
        try
        {
            const res = await this.global.api.get<string>(
                this.listApi(),
                {
                    responseType: 'text',
                    params:
                    {
                        name: this.global.queueName()
                    }
                }
            );
            const rawData = res.data;
            if (rawData)
            {
                // split string
                const lines = rawData.split(/\r?\n/);
                var outList: number[] = [];
                for (const line of lines)
                {
                    // ensure its not empty
                    if (line.trim() === '') continue;

                    try
                    {
                        // Mistake prevention
                        // Replace single quotes with standard JSON double quotes
                        const jsonStandardLine = line.replace(/'/g, '"');
                        
                        // string as JSON
                        const parsed = JSON.parse(jsonStandardLine);
                        if (parsed?.result?.ID !== undefined &&
                            parsed?.result?.ID !== null)
                        {
                            outList.push(parsed.result.ID);
                        }
                    }
                    catch (error)
                    {
                        // avoid error to crash whole app
                        this.global.logger.error('parse Chunk failed:', error, 'raw data:', line);
                    }
                } // for (const line of lines)

                this.list.set(outList);
            } // if (rawData)
        }
        catch
        {
            this.global.queueConnected.set(false);
        }

        this.isRefreshing.set(false);
    }

    onRemoveClicked = async() =>
    {
        this.global.logger.debug("QueueStatusComponent.onRemoveClicked");
        if (!this.isPending)
        {
            return;
        }

        if (this.selectedIds().size === 0)
        {
            return;
        }

        // remove selected ids
        this.isRefreshing.set(true);
        try
        {
            for (const id of this.selectedIds())
            {
                this.global.api.post('/queue/removetask',
                {
                    name: this.global.queueName(),
                    ID: id
                });
            }
        }
        catch
        {
            this.dialog.open(MessageDialog,
            {
                data:
                {
                    isInfo: true,
                    title: 'Error',
                    message: 'Error when remove all selected tasks'
                }
            });
        }

        this.isRefreshing.set(false);
        this.selectedIds.set(new Set());
        this.onRefreshClicked();
    }

    onClearClicked = async() =>
    {
        this.global.logger.debug("QueueStatusComponent.onClearClicked");

        if (this.list().length === 0)
        {
            return;
        }
        
        // clear the queue
        const dialogRef = this.dialog.open(MessageDialog,
        {
            data:
            {
                isInfo: false,
                title: 'Clear Queue',
                message: 'Are you sure you want to clear the list?'
            },
        });

        dialogRef.afterClosed().subscribe(async(result) =>
        {
            if (!result)
            {
                return;
            }
            
            this.isRefreshing.set(true);
            try
            {
                await this.global.api.delete(this.clearApi(),
                {
                    params:
                    {
                        name: this.global.queueName()
                    }
                });

                this.list.set([]);
            }
            catch
            {
                this.dialog.open(MessageDialog,
                {
                    data:
                    {
                        isInfo: true,
                        title: 'Error',
                        message: 'Failed to clear list'
                    }
                });
            }
            
            this.isRefreshing.set(false);
        });
    }

    title = input<string>("");
    listApi = input<string>("");
    detailsApi = input<string>("");
    clearApi = input<string>("");
    isPending = input<boolean>(true);

    list = signal<number[]>([]);
    isRefreshing = signal<boolean>(false);
    disabled = computed(() => this.global.queueDisabled() || this.isRefreshing());

    global = inject(Global);
    private dialog = inject(MatDialog);

    private dataSubscription!: Subscription;
    
    // for mouse event
    private startX = 0;
    private startY = 0;
    private onMouseMove = (
        event: MouseEvent,
        containerRect: DOMRect,
        container: HTMLElement) =>
    {
        if (!this.isDragging()) return;

        const currentX = event.clientX - containerRect.left;
        const currentY = event.clientY - containerRect.top;

        const left = Math.min(this.startX, currentX);
        const top = Math.min(this.startY, currentY);
        const width = Math.abs(this.startX - currentX);

        this.dragBoxStyle.set({
            position: 'absolute',
            left: `${left}px`,
            top: `${top}px`,
            width: `${width}px`,
            height: `${Math.abs(this.startY - currentY)}px`
        });

        const items = container.querySelectorAll('.my-item');
        const newSelected = event.ctrlKey ? new Set(this.selectedIds()) : new Set<number>();

        items.forEach((item: any) =>
        {
            const itemRect = item.getBoundingClientRect();
            const itemLeft = itemRect.left - containerRect.left;
            const itemTop = itemRect.top - containerRect.top;
            const id = Number(item.getAttribute('data-id'));

            const isOverlapping = 
                itemLeft < left + width &&
                itemLeft + itemRect.width > left &&
                itemTop < top + (Math.abs(this.startY - currentY)) &&
                itemTop + itemRect.height > top;

            if (isOverlapping)
            {
                newSelected.add(id);
            }
            else if (!event.ctrlKey)
            {
                newSelected.delete(id);
            }
        });

        this.selectedIds.set(newSelected);
    }

} // class QueueStatusComponent
