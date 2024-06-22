import { createApp } from 'vue'
import App from './App.vue'
//@ts-ignore
import { dispatcher } from './ipc/dispatcher'

import { createPinia } from 'pinia'

import ElementPlus from 'element-plus'
import 'element-plus/dist/index.css'

import '@imengyu/vue3-context-menu/lib/vue3-context-menu.css'
import ContextMenu from '@imengyu/vue3-context-menu'

import '@vscode/codicons/dist/codicon.css'
import { useDialogStore } from './stores/dialogs'

const pinia = createPinia()

window.host.ipcCallback(dispatcher.dispatch.bind(dispatcher))

dispatcher.register('menu', (item: string) => {
  const {showSettingsDlg, showShortcutDlg, showBuyDlg} = useDialogStore()
  switch (item) {
    case'settings':
      showSettingsDlg()
      break
    case'shortcuts':
      showShortcutDlg()
      break
    case 'buy':
      showBuyDlg()
      break
    default:
      break
  }
})

createApp(App)
  .use(ElementPlus)
  .use(pinia)
  .use(ContextMenu)
  .mount('#app')
  .$nextTick(() => {
    postMessage({ payload: 'removeLoading' }, '*')
  })
