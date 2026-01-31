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

import { computed, inject, signal } from '@angular/core';

import
{
    IConnect,
    IQueueList,
    IQueue,
    ConnectMode,
    ExitCode
} from './models';
import { Global } from './global';
import { Logger } from './logger';
import { ConnectionProfile } from './appconfig';
import { TauriConnect } from './tauriconnect';
import { TauriQueueList } from './tauriqueuelist';
import { WebConnect } from './webconnect';
import { WebQueueList } from './webqueuelist';

export class ConnectUnit
{
    constructor(logger: Logger)
    {
        this.logger = logger;
    }

    clean = async(code: ExitCode) =>
    {
        this.logger.debug("ConnectUnit.clean");
        code.clear();
        if (this.queueList)
        {
            this.logger.debug("queueList is not null");
            if (this.queue)
            {
                this.logger.debug("queue is not null");
                var handle = this.queue.handle();
                try
                {
                    await this.queueList.returnQueue(handle);
                }
                catch (e: any)
                {
                    this.logger.error(e.toString());
                }

                this.queue = null;
            }

            await this.queueList.destroy();
            this.queueList = null;
        }

        if (this.connect)
        {
            this.logger.debug("connect is not null");
            await this.connect.destroy(code);
            if (code.hasError)
            {
                return;
            }

            this.connect = null;
        }
    }

    connect: IConnect | null = null;
    queueList: IQueueList | null = null;
    queue: IQueue | null = null;
    private logger: Logger;
}

export class ConnectInfo
{
    constructor(global: Global)
    {
        this.logger.debug("ConnectInfo.constructor");
        this.global = global;
        this.global.status.set("Not connected");
    }

    connect = async(profile: ConnectionProfile, code: ExitCode) =>
    {
        code.clear();
        this.logger.debug("ConnectInfo.connect");
        this.logger.debug("profile is: " + JSON.stringify(profile));

        if (this.unit[profile.mode] !== null)
        {
            code.hasError = true;
            code.msg = "Already connected or disconnect/reset it first";
            this.global.status.set(
                "Already connected or disconnect/reset it first");
            return;
        }

        var unit = new ConnectUnit(this.logger);
        if (profile.mode === ConnectMode.Web)
        {
            unit.connect = new WebConnect();
            await unit.connect.startConnect(profile.target, profile.port, code);
            unit.queueList = new WebQueueList();
        }
        else
        {
            unit.connect = await TauriConnect.create(profile.mode, this.logger, code);
            if (code.hasError)
            {
                return;
            }

            if (unit.connect === null)
            {
                code.hasError = true;
                code.msg = "Connect failed";
                return;
            }

            await unit.connect.startConnect(profile.target, profile.port, code);
            if (code.hasError)
            {
                return;
            }

            // connect success
            // connect to queue list
            var handle = unit.connect.handle();
            unit.queueList = await TauriQueueList.create(handle);
        }

        this.global.status.set("Connected");
        this.isConnected.set(true);
        this.unit[profile.mode] = unit;
    }

    disconnect = async(mode: ConnectMode, code: ExitCode) =>
    {
        this.logger.debug("ConnectInfo.disconnect");
        this.logger.debug("mode is: " + mode);
        code.clear();
        if (this.unit[mode] === null) return;

        this.unit[mode].clean(code);
        if (code.hasError)
        {
            return;
        }

        this.unit[mode] = null;
        if (this.unit[ConnectMode.GRPC] === null &&
            this.unit[ConnectMode.Local] === null &&
            this.unit[ConnectMode.Web] === null
        )
        {
            this.isConnected.set(false);
            this.global.status.set("Not connected");
        }
    }

    readonly currentUnit = computed((): ConnectUnit | null =>
    {
        this.logger.trace("ConnectInfo.currentUnit");
        return this.unit[this.global.config.current_profile().mode];
    });

    readonly connected = computed((): boolean =>
    {
        this.logger.trace("ConnectInfo.connected");
        return this.isConnected();
    });

    private unit: Record<ConnectMode, ConnectUnit | null> =
    {
        [ConnectMode.GRPC]: null,
        [ConnectMode.Local]: null,
        [ConnectMode.Web]: null
    }

    private global: Global;
    private isConnected = signal<boolean>(false);
    private logger: Logger = inject(Logger);
}
