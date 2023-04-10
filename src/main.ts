import { createApp } from 'vue'
import App from './App.vue'
//@ts-ignore
import { dispatcher } from './ipc/dispatcher'

import { createPinia } from 'pinia'

import ElementPlus from 'element-plus'
import 'element-plus/dist/index.css'

import '@imengyu/vue3-context-menu/lib/vue3-context-menu.css'
import ContextMenu from '@imengyu/vue3-context-menu'

const pinia = createPinia()

window.host.ipcCallback(dispatcher.dispatch.bind(dispatcher))

createApp(App)
  .use(ElementPlus)
  .use(pinia)
  .use(ContextMenu)
  .mount('#app')
  .$nextTick(() => {
    postMessage({ payload: 'removeLoading' }, '*')
  })
