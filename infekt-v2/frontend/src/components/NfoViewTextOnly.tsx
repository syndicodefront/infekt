import React from 'react';
import { useCurrentNfo } from '../context/CurrentNfoContext';

const NfoViewTextOnly = () => {
  const currentNfo = useCurrentNfo();

  return (
    <>
      <p>content TEXT ONLY [ {currentNfo.filePath} ]</p>
      {
        Array.from({ length: 50 }, (_, index) => (
          <React.Fragment key={index}>
            {index % 5 === 0 && index ? 'TEXT ONLY' : '...'}
            <br />
          </React.Fragment>
        ))
      }
    </>
  );
};

export default NfoViewTextOnly;
