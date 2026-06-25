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
import { Router } from "@angular/router";
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
import { MessageDialog } from '../components/message-dialog';
import { CommonLayoutComponent } from '../components/common-layout';
import { FF_BRANCH, FF_COMMIT, FF_VERSION } from "../model/config";

@Component
({
    selector: "login-page",
    templateUrl: "./login-page.html",
    styleUrl: "../model/global.css",
    changeDetection: ChangeDetectionStrategy.Eager,
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

export class LoginPage
{
    onClearClicked = () =>
    {
        this.global.logger.debug("SettingsPage.onClearClicked");
        this.profileForm.setValue({
            username: "",
            password: "",
            totp: "",
            tokenTimeOut: 86400
        });
    }

    onLoginClicked = async () =>
    {
        this.global.logger.debug("SettingsPage.onConnectClicked");
        if (this.global.connected())
        {
            this.dialog.open(MessageDialog,
            {
                data:
                {
                    isInfo: true,
                    title: 'Info',
                    message: 'You are already connected'
                
                },
            }); 

            return;
        }

        // check form data
        if (this.profileForm.invalid)
        {
            this.dialog.open(MessageDialog,
            {
                data:
                {
                    isInfo: true,
                    title: 'Error',
                    message: 'Invalid login information'
                
                },
            });

            return;
        }

        this.global.disabled.set(true);
        const value = this.profileForm.getRawValue();
        this.global.connected.set(false);

        // login the api
        try
        {
            await this.global.api.post('/login',
                {
                    username: value.username,
                    password: value.password,
                    otp: value.totp,
                    tokentimeout: value.tokenTimeOut
                }
            );

            this.global.connected.set(true);
            this.router.navigate(['/']);
        }
        catch
        {
            this.dialog.open(MessageDialog,
            {
                data:
                {
                    isInfo: true,
                    title: 'Error',
                    message: 'Failed to login'
                },
            });
        }

        this.global.disabled.set(false);
    }

    // profile
    profileForm = new FormGroup
    ({
        username: new FormControl('',
            {
                validators: [Validators.required, Validators.minLength(1)],
                nonNullable: true
            }
        ),
        password: new FormControl('',
            {
                validators: [Validators.required, Validators.minLength(1)],
                nonNullable: true
            }
        ),
        totp: new FormControl('',
            {
                validators: [Validators.required, Validators.minLength(1)],
                nonNullable: true
            }
        ),
        tokenTimeOut: new FormControl(86400,
            {
                validators: [Validators.required, Validators.min(3600), Validators.max(2592000)],
                nonNullable: true
            }
        )
    });

    global = inject(Global);

    version = FF_VERSION;
    commit = FF_COMMIT;
    branch = FF_BRANCH;

    private dialog = inject(MatDialog);
    
    private router = inject(Router);
}
