import React from 'react';
import { Layout, Segmented, theme } from 'antd';
import { AppstoreOutlined, BlockOutlined, AlignCenterOutlined } from '@ant-design/icons';
import { useSiderCollapsed } from '../context/SiderMenuContext';

const { Content } = Layout;

const MainView = () => {
  const {
    token: { colorBgContainer },
  } = theme.useToken();

  const viewModeOptions = [
    { label: 'Rendered', value: 'rendered', icon: <BlockOutlined /> },
    { label: 'Classic', value: 'classic', icon: <AppstoreOutlined /> },
    { label: 'Text-only', value: 'text', icon: <AlignCenterOutlined /> }
  ];

  const siderCollapsedStatus = useSiderCollapsed();

  return (
    <Layout style={{ marginLeft: siderCollapsedStatus?.currentWidth, backgroundColor: colorBgContainer }}>
      <Segmented style={{ margin: '0 auto' }} options={viewModeOptions} />

      <Content style={{ margin: 0, overflowX: 'auto', overflowY: 'scroll' }}>
        <div>
          <p>long content</p>
          {
            // indicates very long content
            Array.from({ length: 100 }, (_, index) => (
              <React.Fragment key={index}>
                {index % 20 === 0 && index ? 'more' : '...'}
                <br />
              </React.Fragment>
            ))
          }
        </div>
      </Content>
    </Layout>
  );
};

export default MainView;
