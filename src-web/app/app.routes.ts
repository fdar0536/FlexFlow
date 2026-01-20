import { Routes } from "@angular/router";

import { SettingsPage } from "../pages/settings-page";
import { QueueListPage } from "../pages/queuelist-page";
import { AboutPage } from "../pages/about-page";

export const routes: Routes =
[
    {
        path: "",
        component: SettingsPage,
    },
    {
        path: "QueueList",
        component: QueueListPage,
    },
    {
        path: "About",
        component: AboutPage,
    },
];
