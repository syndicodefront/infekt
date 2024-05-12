import React, { useEffect, useState } from 'react';
import { useCurrentNfo } from '../context/CurrentNfoContext';
import { invoke } from '@tauri-apps/api/core';
import { NfoRendererGrid } from '../api/rendergrid';
import { useDevicePixelRatio } from 'use-device-pixel-ratio';

const NfoViewRendered = ({ viewIsActive }: { readonly viewIsActive: boolean }) => {
  const currentNfo = useCurrentNfo();
  const devicePixelRatio = useDevicePixelRatio({ maxDpr: 4 });
  const [nfoRendererGrid, setNfoRendererGrid] = useState<NfoRendererGrid>('');

  useEffect(() => {
    if (!currentNfo.isLoaded) {
      return;
    }

    let isSubscribed = true;

    const fetchData = async () => {
      const grid: NfoRendererGrid = await invoke('get_nfo_renderer_grid');

      if (isSubscribed) {
        setNfoRendererGrid(grid);
      }
    }

    fetchData()
      .catch(console.error);

    return function () { isSubscribed = false; }
  }, [currentNfo, setNfoRendererGrid])

  useEffect(() => {
    console.count('re-render');
  }, [nfoRendererGrid, devicePixelRatio]);

  // TODO: canvas ...

  return (
    <div style={{ display: viewIsActive ? 'block' : 'none' }}>
      ok
    </div>
  );
};

export default NfoViewRendered;
