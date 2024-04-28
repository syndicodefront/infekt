import React from 'react';
import { Layout, theme, ConfigProvider } from 'antd';
import SiderMenu from './components/SiderMenu';
import MainView from './components/MainView';
import DialogMask from './components/DialogMask';
import { SiderCollapsedStatusProvider } from './context/SiderMenuContext';
import { DialogMaskProvider } from './context/DialogMaskContext';

const App = () => {
  const {
    token: { colorBgContainer }, // XXX: this will go away with NFO themes support
  } = theme.useToken();

  return (
    <ConfigProvider theme={{ token: { motion: false } }}>
      <DialogMaskProvider>
        <Layout hasSider style={{ backgroundColor: colorBgContainer }}>
          <SiderCollapsedStatusProvider>
            <SiderMenu></SiderMenu>
            <MainView></MainView>
          </SiderCollapsedStatusProvider>
        </Layout>
        <DialogMask />
      </DialogMaskProvider>
    </ConfigProvider>
  );
};

export default App;
