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

import {Component} from "@angular/core";
import { CommonLayoutComponent } from "../components/common-layout";
import { FF_VERSION, FF_COMMIT, FF_BRANCH } from "../model/version";

@Component
({
    selector: "about-page",
    template: `
        <common-layout
            direction="column"
            align_items="center"
            justify_content="center"
        >
            <h1>Flex Flow</h1>
            <p class="CommonText">Version: {{ version }}</p>
            <p class="CommonText">Commit: {{ commit }}</p>
            <p class="CommonText">Branch: {{ branch }}</p>
            <p class="CommonText">License: <a class="CommonText" href="https://opensource.org/licenses/MIT" target="_blank">MIT License</a></p>
        </common-layout>
    `,
    imports: [CommonLayoutComponent],
})

export class AboutPage
{
    version = FF_VERSION;
    commit = FF_COMMIT;
    branch = FF_BRANCH;
}
