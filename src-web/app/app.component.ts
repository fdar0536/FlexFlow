import { Component, inject } from "@angular/core";
import { RouterOutlet } from "@angular/router";
import { HeaderComponent } from "../components/header-component";
import { Global } from "../model/global";

@Component
({
    selector: "app-root",
    imports: [RouterOutlet, HeaderComponent],
    templateUrl: "./app.component.html",
    styleUrl: "./app.component.css",
})

export class AppComponent
{
    global = inject(Global);
    ngOnInit()
    {
        this.global.isConfigReady.set(false);
        this.global.loadConfig();
    }
}
