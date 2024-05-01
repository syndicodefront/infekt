import React, { PropsWithChildren, createContext, useContext, useReducer } from 'react';

export const SIDER_WIDTH = 200;
export const SIDER_WIDTH_COLLAPSED = 80;

interface SiderCollapsedStatus {
  isCollapsed: boolean;
  currentWidth: number;
};

const SiderCollapsedContext = createContext<SiderCollapsedStatus | undefined>(undefined);
const SiderCollapsedDispatchContext = createContext<React.Dispatch<boolean> | undefined>(undefined);

export const SiderCollapsedStatusProvider = ({ children }: PropsWithChildren) => {
  const [collapsedStatus, dispatch] = useReducer(
    isCollapsedReducer,
    { isCollapsed: false, currentWidth: SIDER_WIDTH }
  );

  return (
    <SiderCollapsedContext.Provider value={collapsedStatus}>
      <SiderCollapsedDispatchContext.Provider value={dispatch}>
        {children}
      </SiderCollapsedDispatchContext.Provider>
    </SiderCollapsedContext.Provider>
  );
}

export const useSiderCollapsed = () => {
  return useContext(SiderCollapsedContext);
}

export const useSiderCollapsedDispatch = () => {
  return useContext(SiderCollapsedDispatchContext);
}

function isCollapsedReducer(_: SiderCollapsedStatus, action: boolean) {
  return {
    isCollapsed: action,
    currentWidth: action ? SIDER_WIDTH_COLLAPSED : SIDER_WIDTH
  }
}
