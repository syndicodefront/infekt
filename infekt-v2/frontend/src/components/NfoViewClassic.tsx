import React, { useEffect, useRef } from 'react';
import { useCurrentNfo } from '../context/CurrentNfoContext';
import { invoke } from '@tauri-apps/api/core';

const NfoViewClassic = ({ viewIsActive }: { viewIsActive: boolean }) => {
  const currentNfo = useCurrentNfo();
  const myRef = useRef<HTMLPreElement>(null);

  useEffect(() => {
    let isSubscribed = true;

    const fetchData = async () => {
      const html: string = await invoke('get_nfo_html_classic');

      if (isSubscribed && myRef.current) {
        myRef.current.innerHTML = html;
      }
    }

    fetchData()
      .catch(console.error);

    return function () { isSubscribed = false; }
  }, [myRef, currentNfo])

  // TODO: replace style/display with some logic that only calls get_nfo_html_classic
  // on the first render, and after NFO has changed.

  return (
    <pre ref={myRef} style={{ display: viewIsActive ? 'block' : 'none' }}></pre>
  );
};

export default NfoViewClassic;
