import React, { useEffect, useState } from 'react';
import { useCurrentNfo } from '../context/CurrentNfoContext';
import { invoke } from '@tauri-apps/api/core';

const NfoViewClassic = ({ viewIsActive }: { readonly viewIsActive: boolean }) => {
  const currentNfo = useCurrentNfo();
  const [nfoHtml, setNfoHtml] = useState<string>('');

  // TODO: delay calling get_nfo_html_classic until
  // the view becomes active for the first time...

  useEffect(() => {
    if (!currentNfo.isLoaded) {
      return;
    }

    let isSubscribed = true;

    const fetchData = async () => {
      const html: string = await invoke('get_nfo_html_classic');

      if (isSubscribed) {
        setNfoHtml(html);
      }
    }

    fetchData()
      .catch(console.error);

    return function () { isSubscribed = false; }
  }, [currentNfo, setNfoHtml])

  return (
    <pre
      style={{ display: viewIsActive ? 'block' : 'none' }}
      // eslint-disable-next-line react/no-danger
      dangerouslySetInnerHTML={{ __html: nfoHtml }}
    />
  );
};

export default NfoViewClassic;
