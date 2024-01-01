import React from 'react';

export interface DialogMaskProps {
  masked: boolean;
};

const DialogMask = ({ masked }: DialogMaskProps) => {
  return (<>
    {masked && <div style={{
      backgroundColor: 'rgba(100, 100, 100, 0.5)',
      WebkitBackdropFilter: 'blur(2px)',
      backdropFilter: 'blur(2px)',
      position: 'fixed',
      height: '100vh',
      width: '100vw',
      left: 0,
      top: 0,
      bottom: 0,
      zIndex: 99999,
    }}></div>}
  </>);
};

export default DialogMask;
