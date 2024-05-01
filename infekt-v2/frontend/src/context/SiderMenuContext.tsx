import React, { PropsWithChildren, createContext, useReducer } from 'react';
import { wrapUseContextGuaranteed } from '../util/useContextGuaranteed';

export const SIDER_WIDTH = 200;
export const SIDER_WIDTH_COLLAPSED = 80;

export interface SiderCollapsedStatus {
  isCollapsed: boolean;
  currentWidth: number;
};

const SiderCollapsedContext = createContext<SiderCollapsedStatus | undefined>(undefined);
const SiderCollapsedDispatchContext = createContext<React.Dispatch<boolean> | undefined>(undefined);

function createInitialState(): SiderCollapsedStatus {
  return { isCollapsed: false, currentWidth: SIDER_WIDTH }
}

export const SiderCollapsedStatusProvider = ({ children }: PropsWithChildren) => {
  const [collapsedStatus, dispatch] = useReducer(isCollapsedReducer, null, createInitialState);

  return (
    <SiderCollapsedContext.Provider value={collapsedStatus}>
      <SiderCollapsedDispatchContext.Provider value={dispatch}>
        {children}
      </SiderCollapsedDispatchContext.Provider>
    </SiderCollapsedContext.Provider>
  );
}

export const useSiderCollapsed = wrapUseContextGuaranteed(SiderCollapsedContext);
export const useSiderCollapsedDispatch = wrapUseContextGuaranteed(SiderCollapsedDispatchContext);

function isCollapsedReducer(_: SiderCollapsedStatus, action: boolean) {
  return {
    isCollapsed: action,
    currentWidth: action ? SIDER_WIDTH_COLLAPSED : SIDER_WIDTH
  }
}
