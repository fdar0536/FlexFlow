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

import { Injectable, signal, inject, computed } from '@angular/core';
import { Router } from '@angular/router';

import axios from 'axios';

import { API_BASE } from './config';
import { Logger } from "./logger";

@Injectable
({
    providedIn: 'root'
})

export class Global
{
    constructor()
    {
        this.logger.debug("Global.constructor");

        this.api = axios.create(
        {
            baseURL: window.location.protocol + "//" + window.location.host + API_BASE,
            timeout: 30000, // 30 secs
            // set Cookie
            withCredentials: true 
        });
    }

    init = async() =>
    {
        this.logger.debug("Global.init");
        this.echoTest();
        if (this.connected())
        {
            this.router.navigate(['/']);
        }
        else
        {
            this.router.navigate(['/login']);
        }
    }

    disabled = signal<boolean>(false);

    api = axios.create();
    connected = signal<boolean>(false);
    queueConnected = signal<boolean>(false);
    queueDisabled = computed(() => this.disabled() || !this.queueConnected());
    queueName = signal<string>('');

    private logger = inject(Logger);
    private router = inject(Router);
    private echoTest = async () =>
    {
        this.logger.debug("SettingsPage.echoTest");
        this.connected.set(false);
        this.disabled.set(true);

        // start echo test
        try
        {
            await this.api.get("/echo");
            this.connected.set(true);
        }
        catch
        {
            this.connected.set(false);
        }

        this.disabled.set(false);
    } // end echoTest
} // end export class Global
