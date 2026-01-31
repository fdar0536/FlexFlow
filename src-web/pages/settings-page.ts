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
import { ConnectMode, ExitCode } from '../model/models';
import { CommonDialog } from '../components/common-dialog';
import { CommonLayoutComponent } from '../components/common-layout';
import { Logger } from "../model/logger";
import { invoke } from "@tauri-apps/api/core";

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
        this.logger.debug("current name is: " +
            this.global.config.current_name());
        this.configCtl.setValue(this.global.config.current_name());

        const current_profile =
        this.global.config.current_profile();
        this.logger.debug("current profile is: " +
            JSON.stringify(current_profile));
        
        this.profileForm.setValue({
            name: this.global.config.current_name(),
            target: current_profile.target,
            port: current_profile.port
        });
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

        this.global.config.current_name.set(this.configCtl.value);
        const current_profile = this.global.config.current_profile();
        this.logger.debug("current profile is: " +
            JSON.stringify(current_profile));

        this.profileForm.setValue({
            name: this.global.config.current_name(),
            target: current_profile.target,
            port: current_profile.port
        });
    }

    onDeleteClicked = () =>
    {
        this.logger.debug("SettingsPage.onDeleteClicked");
        this.logger.debug("configCtl.value is: " +
            this.configCtl.value?.toString());

        if (this.configCtl.value === null) return;
        if (this.configCtl.value === undefined) return;
        if (this.configCtl.value === "") return;

        this.global.config.remove(this.configCtl.value);

        this.logger.debug("current name is: " +
            this.global.config.current_name());
        this.configCtl.setValue(this.global.config.current_name());
    }

    // radio btn
    onRadioChanged = (value: number) =>
    {
        this.logger.debug("SettingsPage.onRadioChanged");
        this.logger.debug("value is: " + value);

        if (this.global.isTauri &&
            value === ConnectMode.Web)
        {
            this.dialog.open(CommonDialog,
            {
                data:
                {
                    type: 'info',
                    title: 'Performance Warning',
                    message: 'This feature is only for debug purpose.'
                },
            });
        }
    } // onRadioChanged = (value: number)

    // connect btns
    onSaveClicked = () =>
    {
        this.logger.debug("SettingsPage.onSaveClicked");

        if (this.profileForm.invalid) return;
        const profile = this.profileForm.value;
        this.logger.debug("profile is: " + JSON.stringify(profile));

        if (profile.name === "" ||
            profile.name === null ||
            profile.name === undefined ||
            profile.target === null ||
            profile.target === undefined ||
            profile.port === null ||
            profile.port === undefined
        ) return;

        this.logger.debug
        ("selectedMode is: " + this.selectedMode.toString());

        this.global.config.add(profile.name,
        {
            mode: this.selectedMode,
            target: profile.target,
            port: profile.port
        });

        this.global.config.current_name.set(profile.name);
    }

    onClearClicked = () =>
    {
        this.logger.debug("SettingsPage.onClearClicked");
        this.profileForm.setValue({
            name: "",
            target: "",
            port: 0
        });
    }

    onDisconnectClicked = async () =>
    {
        this.logger.debug("SettingsPage.onDisconnectClicked");
        var code: ExitCode = new ExitCode();
        await this.global.connectInfo.disconnect(this.selectedMode, code);
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
            
            if (this.global.isTauri)
            {
                await invoke("quit", { code: 1 });
            }
        }
    }

    onConnectClicked = async () =>
    {
        this.logger.debug("SettingsPage.onConnectClicked");
        var code: ExitCode = new ExitCode();
        if (this.profileForm.invalid) return;
        if (this.profileForm.value.target === null ||
            this.profileForm.value.target === undefined
        ) return;
        if (this.profileForm.value.port === null ||
            this.profileForm.value.port === undefined
        ) return;

        await this.global.connectInfo.connect({
            mode: this.selectedMode,
            target: this.profileForm.value.target,
            port: this.profileForm.value.port
        }, code);

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
        name: new FormControl(''),
        target: new FormControl('',
            [Validators.required, Validators.minLength(1)]
        ),
        port: new FormControl(0,
            [
                Validators.max(65535),
                Validators.min(0)
            ]
        )
    });

    global = inject(Global);
    selectedMode = this.global.config.current_profile().mode;
    private dialog = inject(MatDialog);
    private logger: Logger;
}
