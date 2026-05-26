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
import { MatSelectModule } from '@angular/material/select';
import { MatFormFieldModule } from '@angular/material/form-field';
import {
    FormControl,
    FormGroup, FormsModule,
    ReactiveFormsModule,
    Validators
} from '@angular/forms';
import { MatButtonModule } from '@angular/material/button';
import { MatIconModule } from '@angular/material/icon';
import { MatInputModule } from '@angular/material/input';

import { Global } from "../model/global";
import { ExitCode } from '../model/models';
import { CommonDialog } from '../components/common-dialog';
import { CommonLayoutComponent } from '../components/common-layout';
import { Logger } from "../model/logger";

@Component
({
    selector: "settings-page",
    templateUrl: "./settings-page.html",
    imports:
    [
        MatRadioModule,
        CommonLayoutComponent,
        MatSelectModule,
        MatFormFieldModule,
        FormsModule,
        MatButtonModule,
        MatIconModule,
        MatInputModule,
        ReactiveFormsModule
    ],
})

export class SettingsPage
{
    constructor()
    {
        this.logger = inject(Logger);
        this.logger.debug("SettingsPage.constructor");
    }

    // config btns
    onLoadClicked = () =>
    {
        this.logger.debug("SettingsPage.onLoadClicked");
        this.logger.debug("configCtl.value is: " +
            this.configCtl.value?.toString());

        if (this.configCtl.value === null ||
            this.configCtl.value === undefined ||
            this.configCtl.value === ""
        ) return;
    }

    // connect btns
    onSaveClicked = () =>
    {
        this.logger.debug("SettingsPage.onSaveClicked");

        if (this.profileForm.invalid) return;
        const profile = this.profileForm.value;
        this.logger.debug("profile is: " + JSON.stringify(profile));

        if (profile.host === null ||
            profile.host === undefined ||
            profile.host === ""
        ) return;
    }

    onClearClicked = () =>
    {
        this.logger.debug("SettingsPage.onClearClicked");
        this.profileForm.setValue({
            host: "",
            username: "",
            password: "",
            totp: "",
        });
    }

    onDisconnectClicked = async () =>
    {
        this.logger.debug("SettingsPage.onDisconnectClicked");
        var code: ExitCode = new ExitCode();
        if (code.hasError)
        {
            this.dialog.open(CommonDialog,
            {
                data:
                {
                    type: 'error',
                    title: 'Fatal error',
                    message: code.msg
                },
            });
        }
    }

    onConnectClicked = async () =>
    {
        this.logger.debug("SettingsPage.onConnectClicked");
        var code: ExitCode = new ExitCode();

        if (code.hasError)
        {
            this.dialog.open(CommonDialog,
            {
                data:
                {
                    type: 'error',
                    title: 'Error',
                    message: code.msg
                },
            });
        }
    }

    // config
    configCtl = new FormControl('');

    // profile
    profileForm = new FormGroup
    ({
        host: new FormControl('', [Validators.required, Validators.minLength(1)]),
        username: new FormControl('',
            [Validators.required, Validators.minLength(1)]
        ),
        password: new FormControl('',
            [Validators.required, Validators.minLength(1)]
        ),
        totp: new FormControl('',
            [Validators.required, Validators.minLength(1)]
        ),
    });

    global = inject(Global);
    private dialog = inject(MatDialog);
    private logger: Logger;
}
