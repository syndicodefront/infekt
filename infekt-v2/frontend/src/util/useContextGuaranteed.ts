import React from 'react';

export const wrapUseContextGuaranteed = <T>(context: React.Context<T>) => {
  return () => {
    const result = React.useContext(context);

    if (result === undefined) {
      throw new Error(`useContextGuaranteed - missing context`);
    }

    return result;
  }
}
