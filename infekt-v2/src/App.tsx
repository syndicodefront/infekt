import React from 'react';
//import { invoke } from "@tauri-apps/api/primitives";
//import { message } from '@tauri-apps/plugin-dialog';
import { Layout, theme, ConfigProvider } from 'antd';
import SiderMenu from './components/SiderMenu';
import MainView from './components/MainView';
//import DialogMask from './components/DialogMask';
import { SiderCollapsedStatusProvider } from './context/SiderMenuContext';

const App = () => {

  /*async function greet() {
    await message('File not found', { title: 'Tauri', type: 'error' });
  }*/

  const {
    token: { colorBgContainer },
  } = theme.useToken();

  /*
  const [modalShowing, setModalShowing] = useState(false);
  <DialogMask masked={modalShowing}></DialogMask>

  const onMenuClick = async (): Promise<void> => {
    setModalShowing(true);

    const file = await dialogFileOpen({
      multiple: false,
      directory: false,
    });

    setModalShowing(false);

    console.log(file);
  }
  */

  return (
    <ConfigProvider theme={{ token: { motion: false } }}>
      <Layout hasSider style={{ backgroundColor: colorBgContainer }}>
        <SiderCollapsedStatusProvider>
          <SiderMenu></SiderMenu>
          <MainView></MainView>
        </SiderCollapsedStatusProvider>
      </Layout>
    </ConfigProvider>
  );
};

export default App;

/*
let greetInputEl: HTMLInputElement | null;
let greetMsgEl: HTMLElement | null;

async function greet() {
  if (greetMsgEl && greetInputEl) {
    // Learn more about Tauri commands at https://tauri.app/v1/guides/features/command
    greetMsgEl.textContent = await invoke("greet", {
      name: greetInputEl.value,
    });
  }
}

window.addEventListener("DOMContentLoaded", () => {
  greetInputEl = document.querySelector("#greet-input");
  greetMsgEl = document.querySelector("#greet-msg");
  document.querySelector("#greet-form")?.addEventListener("submit", (e) => {
    e.preventDefault();
    greet();
  });
});
*/
