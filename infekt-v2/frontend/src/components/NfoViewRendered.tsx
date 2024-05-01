import React from 'react';
import { useCurrentNfo } from '../context/CurrentNfoContext';

const NfoViewRendered = () => {
  //const rendererGrid: NfoRendererGrid = await invoke('get_nfo_renderer_grid');
  //console.log(rendererGrid);

  const currentNfo = useCurrentNfo();

  return (
    <>
      <p>RENDERED {currentNfo.filePath}</p>
      {
        Array.from({ length: 100 }, (_, index) => (
          <React.Fragment key={index}>
            {index % 10 === 0 && index ? 'RENDERED' : '...'}
            <br />
          </React.Fragment>
        ))
      }
    </>
  );
};

export default NfoViewRendered;
