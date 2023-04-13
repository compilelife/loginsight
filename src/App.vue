<template>
  <div class="drag-layer" v-show="isOverDropZone">
    <p>释放以打开文件/文件夹/工程</p>
  </div>
  <el-tabs 
    type="border-card"
    v-model="tabsStore.currentTab"
    >
    <el-tab-pane v-for="(tab, index) of tabs" 
      class="tabContent"
      :name="index"
      :key="tab.name"
    >
      <template #label>
        <span>
          <ElTooltip v-if="tab.desc" :show-after="1000" :content="tab.desc">
            <EditableLabel v-model="tab.title"/>
          </ElTooltip>
          <EditableLabel v-else v-model="tab.title"/>
          <el-icon 
            class="tabCloseBtn"
            @click.stop="rmTab(tab.name)">
            <Close />
          </el-icon>
        </span>
      </template>
      <component :is="componentForTab(tab)" :data="tab"/>
    </el-tab-pane>
  </el-tabs>
  <OpenCmdDialog ref="refToOpenProcDlg"/>
  <OpenFolderDialog ref="refToOpenFolderDlg"/>
  <SettingsDlg ref="refToSettingsDlg"/>
  <SyntaxPreviewDialog ref="refToSyntaxDlg"/>
  <ShortcutDialog ref="refToShortcutDlg"/>
</template>
  
<script setup lang="ts">

import { Close } from '@element-plus/icons-vue';
import { ElTabs, ElTooltip } from 'element-plus';
import { storeToRefs } from 'pinia';
import { onMounted, ref } from 'vue';
import { useDialogStore } from './stores/dialogs';
import { Tab, useTabsStore } from './stores/tabsStore';
import { measureTextWidth, nextTabId } from './stores/util';
import LogTab from './views/LogTab.vue';
import WelcomeTab from './views/WelcomeTab.vue';
import OpenCmdDialog from './views/OpenProcDialog.vue';
import OpenFolderDialog from './views/OpenFolderDialog.vue';
import SettingsDlg from './views/SettingsDlg.vue';
import { registerShortcuts } from './stores/shortcuts';
import SyntaxPreviewDialog from './views/SyntaxPreviewDialog.vue';
import { useDropZone } from '@vueuse/core';
import { platform } from './ipc/platform';
import ShortcutDialog from './views/ShortcutDialog.vue';
import EditableLabel from './components/EditableLabel.vue';

const tabsStore = useTabsStore()
const {tabs, rmTab, smartOpen} = tabsStore

const dlgStore = useDialogStore()
const {refToOpenProcDlg, refToOpenFolderDlg, refToSettingsDlg, refToSyntaxDlg, refToShortcutDlg} = storeToRefs(dlgStore)

function componentForTab(tab: Tab) {
  return tab.type === 'log' ? LogTab : WelcomeTab
}

registerShortcuts()

function handleDrop(files: File[]|null) {
  if (!files || files.length === 0) return
  return smartOpen((files[0] as any).path)
}
const {isOverDropZone} = useDropZone(document.getElementById('app'), handleDrop)

onMounted(async()=>{
  const args = await platform.getCmdlineArgs()
  if (args.length > 0) {
    const path = args[0]
    return smartOpen(path)
  }
})

</script>

<style>

.el-tabs {
  box-sizing: border-box;
  height: 100%;
}

.tabContent {
  height: 100%;
}

body {
  margin: 0;
  height: 100vh;
  width: 100vw;
}

#app {
  width: 100%;
  height: 100%;
}

.tabCloseBtn {
  vertical-align: middle;
  padding-left: 10px;
}

.tabCloseBtn:hover {
  color: #f00;
  scale: 1.2;
}

.el-tabs {
  --el-tabs-header-height: 33px !important;
}

.el-tabs__content {
  padding: 0 !important;
  height: calc(100% - var(--el-tabs-header-height));
}

.splitpanes.default-theme .splitpanes__pane {
  background-color: white;
}

.splitpanes.default-theme .splitpanes__splitter {
  background-color: var(--el-fill-color-light);
}

.drag-layer {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background-color: rgba(0, 0, 0, 0.5);
  display: flex;
  justify-content: center;
  align-items: center;
  font-size: 24px;
  color: white;
  z-index: 999;
}

#mx-menu-default-container{
  z-index: 3000 !important;
}

</style>
