import React, { PropsWithChildren, createContext, useReducer } from 'react';
import { wrapUseContextGuaranteed } from '../util/useContextGuaranteed';

export type NfoFilePath = string | null;

export interface CurrentNfo {
  isLoaded: boolean;
  filePath: NfoFilePath;
};

export interface CurrentNfoChangedAction {
  type: 'loaded' | 'unloaded';
  filePath: NfoFilePath;
}

const CurrentNfoContext = createContext<CurrentNfo | undefined>(undefined);
const CurrentNfoDispatchContext = createContext<React.Dispatch<CurrentNfoChangedAction> | undefined>(undefined);

function createInitialState(): CurrentNfo {
  return { isLoaded: false, filePath: null }
}

export const CurrentNfoProvider = ({ children }: PropsWithChildren) => {
  const [currentNfo, dispatch] = useReducer(currentNfoChangedReducer, null, createInitialState);

  return (
    <CurrentNfoContext.Provider value={currentNfo}>
      <CurrentNfoDispatchContext.Provider value={dispatch}>
        {children}
      </CurrentNfoDispatchContext.Provider>
    </CurrentNfoContext.Provider>
  );
}

export const useCurrentNfo = wrapUseContextGuaranteed(CurrentNfoContext);
export const useCurrentNfoDispatch = wrapUseContextGuaranteed(CurrentNfoDispatchContext);

function currentNfoChangedReducer(_: CurrentNfo, action: CurrentNfoChangedAction) {
  return {
    isLoaded: action.type === 'loaded',
    filePath: action.type === 'unloaded' ? null : action.filePath,
  }
}
