import React, { PropsWithChildren, createContext, useReducer } from 'react';
import { wrapUseContextGuaranteed } from '../util/useContextGuaranteed';

const ShowDialogMaskContext = createContext<boolean>(false);
const ShowDialogMaskDispatchContext = createContext<React.Dispatch<boolean> | undefined>(undefined);

export const DialogMaskProvider = ({ children }: PropsWithChildren) => {
  const [isMasked, dispatch] = useReducer(showDialogMaskReducer, false);

  return (
    <ShowDialogMaskContext.Provider value={isMasked}>
      <ShowDialogMaskDispatchContext.Provider value={dispatch}>
        {children}
      </ShowDialogMaskDispatchContext.Provider>
    </ShowDialogMaskContext.Provider>
  );
}

export const useShowDialogMaskContext = wrapUseContextGuaranteed(ShowDialogMaskContext);
export const useShowDialogMaskDispatchContext = wrapUseContextGuaranteed(ShowDialogMaskDispatchContext);

function showDialogMaskReducer(_: boolean, action: boolean) {
  return action
}
