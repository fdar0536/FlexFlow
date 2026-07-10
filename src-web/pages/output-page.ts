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
    ChangeDetectionStrategy
} from "@angular/core";
import { AsyncPipe } from '@angular/common';
import { ScrollingModule } from '@angular/cdk/scrolling';
import {
    Observable,
    interval,
    startWith,
    switchMap,
    from,
    map,
    catchError,
    of,
    scan
} from 'rxjs';

import { CommonLayoutComponent } from "../components/common-layout";
import { Global } from "../model/global";
import { MAX_LOG_COUNT } from "../model/config";

interface LogItem
{
    id: number;
    value: string;
}

class CircularArray
{
    enqueue = (items: string[]): void =>
    {
        const itemsLen = items.length;
        if (itemsLen === 0) return;

        const newLogItems: LogItem[] = items.map(item => ({ id: this.m_nextId++, value: item }));

        // 1.
        // if the length of imported data exceeds the total capacity,
        // the final m_capacity written data are directly imported.
        if (itemsLen >= this.m_capacity)
        {
            const newItemsToInsert = newLogItems.slice(-this.m_capacity);
            this.m_arr.splice(0, this.m_capacity, ...newItemsToInsert);
            this.m_front = 0;
            this.m_rear = 0;
            this.m_size = this.m_capacity;
            return;
        }

        // 2.
        // If space is not enough,
        // discard the oldest data at the front (simply move the front pointer).
        const neededSpace = itemsLen;
        const availableSpace = this.m_capacity - this.m_size;
        if (neededSpace > availableSpace)
        {
            const toDelete = neededSpace - availableSpace;
            this.m_front = (this.m_front + toDelete) % this.m_capacity;
            this.m_size -= toDelete;
        }

        // 3. insert new data
        const spaceToEnd = this.m_capacity - this.m_rear;
        if (itemsLen <= spaceToEnd)
        {
            // If there is enough space remaining at the tail, insert it all at once.
            this.m_arr.splice(this.m_rear, itemsLen, ...newLogItems);
        }
        else
        {
            // If there is not enough space at the tail, insert it in two sections.
            const part1 = newLogItems.slice(0, spaceToEnd);
            const part2 = newLogItems.slice(spaceToEnd);
            this.m_arr.splice(this.m_rear, part1.length, ...part1);
            this.m_arr.splice(0, part2.length, ...part2);
        }

        this.m_rear = (this.m_rear + itemsLen) % this.m_capacity;
        this.m_size += itemsLen;
    }

    getItems = (): LogItem[] =>
    {
        const result: LogItem[] = [];
        if (this.m_size === 0)
        {
            return result;
        }

        for (let i = 0; i < this.m_size; i++)
        {
            const index = (this.m_front + i) % this.m_capacity;
            result.push(this.m_arr[index]);
        }

        return result;
    }

    private m_arr: LogItem[] = new Array<LogItem>(MAX_LOG_COUNT);
    private m_capacity: number = MAX_LOG_COUNT;
    private m_size: number = 0;
    private m_front: number = 0;
    private m_rear: number = 0;
    private m_nextId: number = 0;
} // class CircularArray

@Component
({
    selector: "output-page",
    templateUrl: "./output-page.html",
    changeDetection: ChangeDetectionStrategy.OnPush,
    imports:
    [
        AsyncPipe,
        CommonLayoutComponent,
        ScrollingModule
    ],
    styleUrl: "../model/global.css",
})

export class OutputPage
{
    logTrackBy = (_: number, item: LogItem): number =>
    {
        return item.id;
    }

    readonly global = inject(Global);
    readonly logs = new CircularArray();

    readonly logItems$: Observable<LogItem[]> = interval(5000).pipe(
        startWith(0),
        switchMap(() => from(this.global.api.get<string[]>('/queue/readcurrentoutput',
        {
            responseType: 'text',
            params: { name: this.global.queueName() }
        }))
        .pipe(
            map(response =>
            {
                /*
                the data structure of response.data is:
                {"result":{"msg":"Hello world!\r\n"}}
                {"result":{"msg":"Hello world!\r\n"}}
                {"result":{"msg":"Hello world!\r\n"}}
                {"result":{"msg":"Hello world!\r\n"}}
                {"result":{"msg":"Hello world!\r\n"}}
                */
                const rawText = response.data as unknown as string;
                if (!rawText) return [];

                const lines = rawText.split(/\r?\n/);
                const newLogStrings: string[] = [];

                for (const line of lines)
                {
                    if (line.trim() === '') continue;

                    try
                    {
                        const jsonStandardLine = line.replace(/'/g, '"');
                        const parsed = JSON.parse(jsonStandardLine);
                        
                        if (parsed?.result?.msg !== undefined &&
                            parsed?.result?.msg !== null &&
                            parsed?.result?.msg !== '')
                        {
                            newLogStrings.push(parsed.result.msg.replace(/\r?\n$/, ''));
                        }
                    }
                    catch (error)
                    {
                        this.global.logger.error('parse Chunk failed:', error, 'raw data:', line);
                    }
                }

                return newLogStrings;
            }),
            catchError(() =>
            {
                this.global.logger.error('Update Failed');
                return of(["Failed to get output or not connected to queue"]);
            })
        )),
        scan((circularArray, newLogStrings: string[]) =>
        {
            circularArray.enqueue(newLogStrings);
            return circularArray;
        }, this.logs),
        map(circularArray => circularArray.getItems())
    );
}
