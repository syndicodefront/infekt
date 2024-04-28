import React, { PropsWithChildren, createContext, useContext, useReducer } from 'react';

const ShowDialogMaskContext = createContext<boolean>(false);
const ShowDialogMaskDispatchContext = createContext<React.Dispatch<boolean> | undefined>(undefined);

export function DialogMaskProvider({ children }: PropsWithChildren) {
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

export function useShowDialogMaskContext() {
  return useContext(ShowDialogMaskContext);
}

export function useShowDialogMaskDispatchContext() {
  return useContext(ShowDialogMaskDispatchContext);
}

function showDialogMaskReducer(_: boolean, action: boolean) {
  return action
}
