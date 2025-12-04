import { create } from 'zustand';

import { IConnect, IQueueList, IQueue } from './imodel';

interface GlobalState
{
    status: string;
    conn: IConnect | null;
    list: IQueueList | null;

    setStatus: (status: string) => void;
    setConnect: (conn: IConnect) => void;
    setQueueList: (list: IQueueList) => void;
}

const useGlobal = create<GlobalState>((set) => ({
    status: "",
    conn: null,
    list: null,

    setStatus: (status: string) => set(
        {
            status: status
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
