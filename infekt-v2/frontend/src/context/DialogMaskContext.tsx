import React, { PropsWithChildren, createContext, useContext, useReducer } from 'react';

const ShowDialogMaskContext = createContext<boolean>(false);
const ShowDialogMaskDispatchContext = createContext<React.Dispatch<boolean> | undefined>(undefined);

export const DialogMaskProvider = ({ children }: PropsWithChildren) => {
  const [isMasked, dispatch] = useReducer(
    showDialogMaskReducer,
    false
  );

  return (
    <ShowDialogMaskContext.Provider value={isMasked}>
      <ShowDialogMaskDispatchContext.Provider value={dispatch}>
        {children}
      </ShowDialogMaskDispatchContext.Provider>
    </ShowDialogMaskContext.Provider>
  );
}

export const useShowDialogMaskContext = () => {
  return useContext(ShowDialogMaskContext);
}

export const useShowDialogMaskDispatchContext = () => {
  return useContext(ShowDialogMaskDispatchContext);
}

function showDialogMaskReducer(_: boolean, action: boolean) {
  return action
}
