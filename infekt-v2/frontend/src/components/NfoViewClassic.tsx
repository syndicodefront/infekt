import React, { useEffect, useRef } from 'react';
import { useCurrentNfo } from '../context/CurrentNfoContext';
import { invoke } from '@tauri-apps/api/core';

const NfoViewClassic = () => {
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

  return (
    <pre ref={myRef}></pre>
  );
};

export default NfoViewClassic;
