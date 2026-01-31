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

import { computed, inject, signal } from "@angular/core";
import { Mutex } from "async-mutex";
import { Global } from "./global";
import { invoke } from "@tauri-apps/api/core";
import { ConnectMode } from "./models";
import { Logger } from "./logger";

export type ConnectionProfile =
{
    mode: ConnectMode;
    target: string;
    port: number;
}

export type AppConfigData = 
{
    profiles: Record<string, ConnectionProfile>;
    current_profile: string;
}

function isEmptyObj(obj: Object): boolean
{
    return Object.keys(obj).length === 0;
}

export class AppConfig
{
    constructor(global: Global)
    {
        this.global = global;
    }

    // load the config to buffer from filesystem via tauri
    load = async() =>
    {
        this.logger.debug("AppConfig.load");
        const release = await this.mutex.acquire();
        if (this.global.isTauri)
        {
            try
            {
                const data =
                await invoke<AppConfigData>("load_config");
                this.logger.debug("data is " + data.toString());
                if (isEmptyObj(data.profiles) ||
                    data.current_profile === undefined ||
                    data.current_profile === null ||
                    data.current_profile === "" ||
                    !data.profiles.hasOwnProperty(data.current_profile)
                )
                {
                    this.loadDefaultProfile();
                    this.loaded.set(true);
                    release();
                    return;
                }

                this.profiles.set(data.profiles);
                this.current_name.set(data.current_profile);
            }
            catch (e: any)
            {
                this.logger.error(e.toString());
                this.loadDefaultProfile();
                this.loaded.set(true);
                release();
                return;
            }
        }
        else
        {
            this.loadDefaultProfile();
        }

        this.loaded.set(true);
        release();
    }

    // save the config in buffer to filesystem via tauri
    save = async() =>
    {
        this.logger.debug("AppConfig.save");
        const release = await this.mutex.acquire();
        if (this.global.isTauri &&
            !isEmptyObj(this.profiles()) &&
            this.current_name() !== "")
        {
            const buffer =
            {
                profiles: this.profiles(),
                current_profile: this.current_name()
            }

            this.logger.debug("buffer is " + buffer.toString());

            try
            {
                await invoke("save_config", { config: buffer });
            }
            catch (e: any)
            {
                this.logger.error(e.toString());
            }
        }

        release();
    }

    // add the profile to buffer
    add = async (name: string, profile: ConnectionProfile) =>
    {
        this.logger.debug("AppConfig.add");
        this.logger.debug("name is " + name);
        this.logger.debug("profile is " + profile.toString());

        const release = await this.mutex.acquire();
        if (name === "" ||
            name === "default" ||
            isEmptyObj(profile))
        {
            this.logger.error("Invalid profile");
            release();
            return;
        }

        if (Object.hasOwn(this.profiles(), name))
        {
            this.logger.info
            ("Profile already exists, switch to \"update\"");
            release();
            this.update(name, profile);
            return;
        }

        this.profiles.update(currentProfiles =>
        ({
            ...currentProfiles,
            [name]: profile
        }));
        
        release();
    }

    // remove the profile from buffer
    remove = async (name: string) =>
    {
        this.logger.debug("AppConfig.remove");
        this.logger.debug("name is " + name);

        const release = await this.mutex.acquire();
        const currentProfiles = this.profiles();
        this.logger.debug("currentProfiles is " + currentProfiles.toString());

        if (name === "" ||
            name === "default" ||
            isEmptyObj(currentProfiles) ||
            !currentProfiles.hasOwnProperty(name)
        )
        {
            this.logger.error("Invalid profile");
            release();
            return;
        }

        this.profiles.update(prev =>
        {
            const next = { ...prev };
            delete next[name];
            return next;
        });

        if (name === this.current_name())
        {
            this.logger.debug("Current name is cleaned");
            this.current_name.set("");
        }

        release();
    }

    // update the profile in buffer
    update = async(name: string, profile: ConnectionProfile) =>
    {
        this.logger.debug("AppConfig.update");
        this.logger.debug("name is " + name);
        this.logger.debug("profile is " + profile.toString());

        const release = await this.mutex.acquire();
        if (name === "" || isEmptyObj(profile))
        {
            this.global
            release();
            return;
        }

        const currentProfiles = this.profiles();
        this.logger.debug("currentProfiles is " + currentProfiles.toString());

        if (!currentProfiles.hasOwnProperty(name))
        {
            this.logger.info
            ("Invalid profile, switch to \"add\"");
            release();
            this.add(name, profile);
            return;
        }

        this.profiles.update(prev =>
        {
            const next = { ...prev };
            next[name] = profile;
            return next;
        });

        release();
    }

    // it will set true when buffer is ready
    readonly ready = computed((): boolean =>
    {
        this.logger.trace("AppConfig.ready");
        return this.loaded()
    });

    readonly current_profile =  computed((): ConnectionProfile =>
    {
        this.logger.trace("current_profile");
        return this.profiles()[this.current_name()];
    })

    readonly profileList = computed((): string[] =>
    {
        this.logger.trace("AppConfig.ready");
        return Object.keys(this.profiles());
    });

    private global: Global;
    private mutex = new Mutex();
    private profiles = signal<Record<string, ConnectionProfile>>({});
    current_name = signal<string>("");
    private loaded = signal<boolean>(false);

    // if it has any reason cannot load the profile
    // this function is fallback
    private loadDefaultProfile = () =>
    {
        this.logger.debug("AppConfig.loadDefaultProfile");
        var data = {};
        if (this.global.isTauri)
        {
            data =
            {
                "default":
                {
                    mode: ConnectMode.GRPC,
                    target: "localhost",
                    port: 12345
                }
            }
        }
        else
        {
            data =
            {
                "default":
                {
                    mode: ConnectMode.Web,
                    target: window.location.hostname,
                    port: 12345
                }
            }
        }

        this.logger.debug("data is " + data.toString());
        this.profiles.set(data);
        this.current_name.set("default");
    } // defaultProfile

    private logger: Logger = inject(Logger);
}
