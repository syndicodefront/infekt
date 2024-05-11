import React from 'react';
import { useCurrentNfo } from '../context/CurrentNfoContext';

const NfoViewRendered = ({ viewIsActive }: { viewIsActive: boolean }) => {
  //const rendererGrid: NfoRendererGrid = await invoke('get_nfo_renderer_grid');
  //console.log(rendererGrid);

  const currentNfo = useCurrentNfo();

  // TODO: replace style/display with some logic that only calls get_nfo_html_stripped
  // on the first render, and after NFO has changed.

  return (
    <div style={{ display: viewIsActive ? 'block' : 'none' }}>
      <p>RENDERED {currentNfo.filePath}</p>
      {
        Array.from({ length: 100 }, (_, index) => (
          <React.Fragment key={index}>
            {index % 10 === 0 && index ? 'RENDERED' : '...'}
            <br />
          </React.Fragment>
        ))
      }
    </div>
  );
};

export default NfoViewRendered;
