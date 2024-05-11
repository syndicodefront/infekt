import React, { useEffect, useState } from 'react';
import { useCurrentNfo } from '../context/CurrentNfoContext';
import { invoke } from '@tauri-apps/api/core';
import { NfoRendererGrid } from '../api/rendergrid';

const NfoViewRendered = ({ viewIsActive }: { readonly viewIsActive: boolean }) => {
  const currentNfo = useCurrentNfo();
  const [dummy, setDummy] = useState<string>('');

  useEffect(() => {
    if (!currentNfo.isLoaded) {
      return;
    }

    let isSubscribed = true;

    const fetchData = async () => {
      const grid: NfoRendererGrid = await invoke('get_nfo_renderer_grid');

      if (isSubscribed) {
        setDummy(JSON.stringify(grid, null, 2));
      }
    }

    fetchData()
      .catch(console.error);

    return function () { isSubscribed = false; }
  }, [currentNfo, setDummy])

  // TODO: canvas ...

  return (
    <pre style={{ display: viewIsActive ? 'block' : 'none' }}>
      {dummy}
    </pre>
  );
};

export default NfoViewRendered;
