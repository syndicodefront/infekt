import React from 'react';
import { Layout, Menu } from 'antd';
import type { MenuProps } from 'antd';
import { MenuInfo } from 'rc-menu/es/interface';
import { FolderOpenOutlined, SettingOutlined, InfoCircleOutlined } from '@ant-design/icons';
import { SIDER_WIDTH, SIDER_WIDTH_COLLAPSED, useSiderCollapsedDispatch } from '../context/SiderMenuContext';

const { Sider } = Layout;
type MenuItem = Required<MenuProps>['items'][number];

const SiderMenu = () => {
  const menuItems: Array<MenuItem> = [
    { label: 'Open File...', key: 'OPEN', icon: <FolderOpenOutlined /> },
    { label: 'Preferences', key: 'PREF', icon: <SettingOutlined /> },
    { label: 'About', key: 'ABOUT', icon: <InfoCircleOutlined /> },
    { type: 'divider' },
  ];

  const onMenuClick = ({ key }: MenuInfo): void => {
    switch (key) {
      case 'OPEN':
        break;
      case 'PREF':
        break;
      case 'ABOUT':
        break;
    }
  };

  return (
    <Sider
      theme='dark'
      style={{
        overflow: 'auto',
        height: '100vh',
        position: 'fixed',
        left: 0,
        top: 0,
        bottom: 0,
      }}
      collapsible
      onCollapse={useSiderCollapsedDispatch()}
      width={SIDER_WIDTH}
      collapsedWidth={SIDER_WIDTH_COLLAPSED}
    >
      <Menu
        selectable={false}
        items={menuItems}
        mode='vertical'
        theme='dark'
        onClick={onMenuClick}
      >
      </Menu>
    </Sider>
  );

  /*
  Settings:
  - font (including size)
  - background color
  - text color
  - art color
  - glow color
  */
};

export default SiderMenu;
