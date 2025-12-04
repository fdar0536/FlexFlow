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

import React from "react";

// ui
import Box from '@mui/material/Box';
import BottomNavigation from '@mui/material/BottomNavigation';
import BottomNavigationAction from '@mui/material/BottomNavigationAction';

// icon
import { AddLink, History, Info, List, Settings } from '@mui/icons-material';
import useGlobal from "../model/store";
import { pages } from "../model/pages";

interface HeaderProps
{
    page: pages;
    setPage: (page: pages) => void;
}

class HeaderBase extends React.Component<HeaderProps>
{
    onSettingsClick = (e: React.MouseEvent<HTMLButtonElement>): void =>
    {
        e.preventDefault();
        this.props.setPage(pages.settings);
    }

    onConnectClick = (e: React.MouseEvent<HTMLButtonElement>): void =>
    {
        e.preventDefault();
        this.props.setPage(pages.connect);
    }

    onAboutClick = (e: React.MouseEvent<HTMLButtonElement>): void =>
    {
        e.preventDefault();
        this.props.setPage(pages.about);
    }

    render = (): React.ReactNode =>
    {
        return (
            <Box sx={{ width: window.innerWidth }}>
                <BottomNavigation
                    showLabels
                >
                    <BottomNavigationAction label="Setings"
                        icon={<Settings />}
                        onClick={ this.onSettingsClick }
                    />
                    <BottomNavigationAction label="Connect"
                        icon={<AddLink />}
                        onClick={ this.onConnectClick }
                    />
                    <BottomNavigationAction label="Queue List"
                        icon={<List />}
                    />
                    <BottomNavigationAction label="Queue"
                        icon={<List />}
                    />
                    <BottomNavigationAction label="Log"
                        icon={<History />}
                    />
                    <BottomNavigationAction label="About"
                        icon={<Info />}
                        onClick={ this.onAboutClick }
                    />
                </BottomNavigation>
            </Box>
        )    
    }
}

function Header() {
    const { page, setPage } = useGlobal();

    return (
        <HeaderBase
            page = { page }
            setPage = { setPage }
        />
    );
}

export default Header;
