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

pub mod loadlib;
pub mod ffmodeldef;
pub mod connect;
pub mod queue_list;
pub mod queue;

use tauri::{
    menu::{Menu, MenuItem},
    tray::TrayIconBuilder,
};

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run()
{
    let res = loadlib::load_api();
    if res.is_err()
    {
        println!("Fail to load FFModel");
        return;
    }

    // create menu
    

    tauri::Builder::default()
        .setup(|app| {
            let tray = TrayIconBuilder::new().build(app)?;
            let quit_i =
            MenuItem::with_id(app, "quit",
            "Quit", true, None::<&str>)?;
            let show_i = MenuItem::with_id(app, "",
            "Show", true, None::<&str>)?;

            let menu =
            Menu::with_items(app, &[&quit_i, &show_i])?;
            let tray = TrayIconBuilder::new()
            .menu(&menu)
            .on_menu_event(|app, event| match event.id.as_ref() {
                "Quit" => {
                  
                  app.exit(0);
                }
                _ => {
                  println!("menu item {:?} not handled", event.id);
                }
              })
            .build(app)?;
            Ok(())
        })
        .plugin(tauri_plugin_opener::init())
        .invoke_handler(tauri::generate_handler![
            // connect
            connect::connect_init,
            connect::connect_destroy,
            connect::connect_start_connect,
            connect::connect_target_path,

            // queue list
            queue_list::queue_list_init,
            queue_list::queue_list_destroy,
            queue_list::queue_list_create_queue,
            queue_list::queue_list_list_queue,
            queue_list::queue_list_delete_queue,
            queue_list::queue_list_rename_queue,
            queue_list::queue_list_get_queue,
            queue_list::queue_list_return_queue,

            // queue
            queue::queue_list_pending,
            queue::queue_list_finished,
            queue::queue_pending_details,
            queue::queue_finished_details,
            queue::queue_clear_pending,
            queue::queue_clear_finished,
            queue::queue_current_task,
            queue::queue_add_task,
            queue::queue_remove_task,
            queue::queue_is_running,
            queue::queue_read_current_output,
            queue::queue_start,
            queue::queue_stop
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
} // pub fn run()
