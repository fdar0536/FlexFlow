import { create } from 'zustand';

import { IConnect, IQueueList } from './imodel';
import { pages } from './pages';

interface GlobalState
{
    status: string;
    page: pages;
    conn: IConnect | null;
    list: IQueueList | null;

    setStatus: (status: string) => void;
    setPage: (page: pages) => void;
    setConnect: (conn: IConnect) => void;
    setQueueList: (list: IQueueList) => void;
}

const useGlobal = create<GlobalState>((set) => ({
    status: "",
    page: pages.connect,
    conn: null,
    list: null,

    setStatus: (status: string) => set(
        {
            status: status
        }
    ),

    setPage: (page: pages) => set(
        {
            page: page
        }
    ),

    setConnect: (conn: IConnect) => set(
        {
            conn: conn
        }
    ),

    setQueueList: (list: IQueueList) => set(
        {
            list: list
        }
    )
}));

export default useGlobal;
