import React, { useState } from 'react';
import { Layout, Segmented, theme } from 'antd';
import { AppstoreOutlined, BlockOutlined, AlignCenterOutlined } from '@ant-design/icons';
import { useSiderCollapsed } from '../context/SiderMenuContext';
import NfoViewRendered from './NfoViewRendered';
import NfoViewTextOnly from './NfoViewTextOnly';
import NfoViewClassic from './NfoViewClassic';

const { Content } = Layout;

enum ViewMode {
  Rendered = 1,
  Classic = 2,
  TextOnly = 3,
}

const DefaultViewMode = ViewMode.Rendered;

const viewModeOptions = [
  { label: 'Rendered', value: ViewMode.Rendered, icon: <BlockOutlined /> },
  { label: 'Classic', value: ViewMode.Classic, icon: <AppstoreOutlined /> },
  { label: 'Text-only', value: ViewMode.TextOnly, icon: <AlignCenterOutlined /> }
];

const MainView = () => {
  const {
    token: { colorBgContainer },
  } = theme.useToken();

  const siderCollapsedStatus = useSiderCollapsed();
  const [viewMode, setViewMode] = useState<ViewMode>(DefaultViewMode);

  return (
    <Layout style={{ marginLeft: siderCollapsedStatus?.currentWidth, backgroundColor: colorBgContainer }}>
      <Segmented<ViewMode> style={{ margin: '0 auto' }} defaultValue={DefaultViewMode} options={viewModeOptions} onChange={setViewMode} />

      <Content style={{ margin: 0, overflowX: 'auto', overflowY: 'scroll' }}>
        {viewMode === ViewMode.Rendered && <NfoViewRendered />}
        {viewMode === ViewMode.Classic && <NfoViewClassic />}
        {viewMode === ViewMode.TextOnly && <NfoViewTextOnly />}
      </Content>
    </Layout>
  );
};

export default MainView;
