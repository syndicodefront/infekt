import React, { useCallback } from 'react';
import { Layout, Menu } from 'antd';
import type { MenuProps } from 'antd';
import { MenuInfo } from 'rc-menu/es/interface';
import { FolderOpenOutlined, SettingOutlined, InfoCircleOutlined } from '@ant-design/icons';
import { SIDER_WIDTH, SIDER_WIDTH_COLLAPSED, useSiderCollapsedDispatch } from '../context/SiderMenuContext';
import { useShowDialogMaskDispatchContext } from '../context/DialogMaskContext';
import { open as dialogFileOpen } from '@tauri-apps/plugin-dialog';
import { invoke } from '@tauri-apps/api/core';
import { LoadNfoRequest, LoadNfoResponse } from '../api/loadnfo';
import { useCurrentNfoDispatch } from '../context/CurrentNfoContext';

const { Sider } = Layout;
type MenuItem = Required<MenuProps>['items'][number];

const menuItems: MenuItem[] = [
  { label: 'Open File...', key: 'OPEN', icon: <FolderOpenOutlined /> },
  { label: 'Preferences', key: 'PREF', icon: <SettingOutlined /> },
  { label: 'About', key: 'ABOUT', icon: <InfoCircleOutlined /> },
  { type: 'divider' },
];

const SiderMenu = () => {
  const toggleDialogMask = useShowDialogMaskDispatchContext();
  const updateCurrentNfo = useCurrentNfoDispatch();

  const onMenuClick = useCallback(async ({ key }: MenuInfo) => {
    switch (key) {
      case 'OPEN':
        toggleDialogMask(true);

        try {
          const file = await dialogFileOpen({
            multiple: false,
            directory: false,
            filters: [
              { name: 'NFO Files', extensions: ['nfo', 'diz', 'asc'] },
              { name: 'Text Files', extensions: ['txt'] },
              { name: 'All Files', extensions: ['*'] },
            ]
          });

          if (file) {
            const loadNfoRequest: LoadNfoRequest = {
              req: {
                filePath: file.path,
              }
            };

            const loadNfoResponse: LoadNfoResponse = await invoke('load_nfo', loadNfoRequest);

            if (loadNfoResponse.success) {
              updateCurrentNfo({ type: 'loaded', filePath: file.path });
            }
          }
        } finally {
          toggleDialogMask(false);
        }
        break;
      case 'PREF':
        break;
      case 'ABOUT':
        break;
    }
  }, [toggleDialogMask, updateCurrentNfo]);

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
      />
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