import { Routes } from "@angular/router";

import { SettingsPage } from "../components/settings-page";
import { QueueListPage } from "../components/queuelist-page";

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
];
