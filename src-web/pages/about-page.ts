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

import {AfterViewInit, Component, inject, signal, ChangeDetectionStrategy} from "@angular/core";

import { CommonLayoutComponent } from "../components/common-layout";
import { FF_VERSION, FF_COMMIT, FF_BRANCH } from "../model/config";
import { Global } from "../model/global";


interface ServerInfo
{
    branch: string;
    commit: string;
    version: string;
}

@Component
({
    selector: "about-page",
    template: `
        <h1>Flex Flow Web UI</h1>
        <common-layout
            direction="row"
            align_items="center"
            justify_content="center"
            gap="10"
        >
            <p class="CommonText">Version: {{ version }}</p>
            <p class="CommonText">Commit: {{ commit }}</p>
            <p class="CommonText">Branch: {{ branch }}</p>
            <p class="CommonText">License: <a class="CommonText" href="https://opensource.org/licenses/MIT" target="_blank">MIT License</a></p>
        </common-layout>
        <h1>Server Info</h1>
        <common-layout
            direction="row"
            align_items="center"
            justify_content="center"
            gap="10"
        >
            <p class="CommonText">Version: {{ this.serverVersion() }}</p>
            <p class="CommonText">Commit: {{ this.serverCommit() }}</p>
            <p class="CommonText">Branch: {{ this.serverBranch() }}</p>
        </common-layout>  
    `,
    changeDetection: ChangeDetectionStrategy.Eager,
    imports: [CommonLayoutComponent],
    styleUrl: "../model/global.css",
})

export class AboutPage implements AfterViewInit
{
    ngAfterViewInit()
    {
        setTimeout(() =>
        {
            this.getServerInfo();
        }, 0);
    }

    version = FF_VERSION;
    commit = FF_COMMIT;
    branch = FF_BRANCH;

    // server info
    serverVersion = signal<string>("Not connected");
    serverCommit = signal<string>("Not connected");
    serverBranch = signal<string>("Not connected");

    private global = inject(Global);

    private getServerInfo = async () =>
    {
        try
        {
            const res = await this.global.api.get<ServerInfo>('/info');
            this.serverVersion.set(res.data.version);
            this.serverCommit.set(res.data.commit);
            this.serverBranch.set(res.data.branch);
        }
        catch
        {
            const err = "Failed to get server info"
            this.serverVersion.set(err);
            this.serverCommit.set(err);
            this.serverBranch.set(err);
        }
    }
}
