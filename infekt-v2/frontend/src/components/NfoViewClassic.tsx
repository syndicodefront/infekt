import React from 'react';
import { useCurrentNfo } from '../context/CurrentNfoContext';

const NfoViewClassic = () => {
  const currentNfo = useCurrentNfo();

  return (
    <>
      <p>{currentNfo.filePath} - CLASSIC</p>
      {
        Array.from({ length: 100 }, (_, index) => (
          <React.Fragment key={index}>
            {index % 20 === 0 && index ? 'CLASSIC' : '...'}
            <br />
          </React.Fragment>
        ))
      }
    </>
  );
};

export default NfoViewClassic;
