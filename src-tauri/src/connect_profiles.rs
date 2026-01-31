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

use std::fs;
use std::path::PathBuf;
use std::collections::HashMap;
use tauri::{AppHandle, Manager, Runtime};
use serde::{Serialize, Deserialize};

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct ConnectionProfile
{
    pub mode: u8,
    pub target: String,
    pub port: u16,
}

#[derive(Debug, Serialize, Deserialize, Default)]
pub struct AppConfig
{
    // 使用 HashMap，Key 是 Profile 的名稱 (String)
    pub profiles: HashMap<String, ConnectionProfile>,
    pub current_profile: Option<String>,
}

fn get_config_path<R: Runtime>(app: &AppHandle<R>) -> PathBuf
{
    app.path().app_config_dir()
        .expect("Cannot get app config dir")
        .join("config.yaml")
}

#[tauri::command]
pub fn save_config(app_handle: tauri::AppHandle, config: AppConfig) -> Result<(), String>
{
    let path = get_config_path(&app_handle);
    
    if let Some(parent) = path.parent()
    {
        fs::create_dir_all(parent).map_err(|e| e.to_string())?;
    }

    // 將資料轉為 YAML 字串
    let yaml = serde_yaml::to_string(&config).map_err(|e| e.to_string())?;
    
    // 寫入檔案
    fs::write(path, yaml).map_err(|e| e.to_string())?;
    Ok(())
}

#[tauri::command]
pub fn load_config(app_handle: tauri::AppHandle) -> Result<AppConfig, String>
{
    let path = get_config_path(&app_handle);
    
    if !path.exists()
    {
        return Ok(AppConfig::default());
    }

    // 讀取檔案
    let content = fs::read_to_string(path).map_err(|e| e.to_string())?;
    
    // 解析 YAML
    let config: AppConfig = serde_yaml::from_str(&content).map_err(|e| e.to_string())?;
    Ok(config)
}
