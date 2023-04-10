<template>
  <el-dropdown trigger="click">
    <ToolButton label="文件">
      <FolderOpen />
    </ToolButton>
    <template #dropdown>
      <ElDropdownMenu>
        <ElDropdownItem @click="tabs.openFile" :icon="OpenFileIcon">打开文件</ElDropdownItem>
        <ElDropdownItem @click="tabs.openFolder" :icon="OpenFolderIcon">打开文件夹</ElDropdownItem>
        <ElDropdownItem @click="tabs.openProc" :icon="OpenProcIcon">打开外部程序</ElDropdownItem>
        <ElDropdownItem @click="tabs.openClipboard" :icon="OpenClipboardIcon">打开粘贴板</ElDropdownItem>
        <ElDropdownItem @click="tabs.openProject" :icon="OpenProjectIcon">打开项目</ElDropdownItem>
        <ElDropdownItem @click="userSaveProject" :icon="OpenProjectIcon">保存项目</ElDropdownItem>
        <ElDropdownItem @click="tabs.exportCurLogView" :icon="SaveOne">保存当前日志</ElDropdownItem>
      </ElDropdownMenu>
    </template>
  </el-dropdown>
  <el-dropdown trigger="click">
    <ToolButton label="最近文件">
      <FileDateOne />
    </ToolButton>
    <template #dropdown>
      <ElDropdownItem v-for="recent in recents" :key="recent.name" :icon="getRecentItemIcon(recent.type)"
        @click="tabs.openRecentItem(recent)">
        {{ recent.name }}
      </ElDropdownItem>
    </template>
  </el-dropdown>
</template>

<script lang="ts" setup>

import ToolButton from './ToolButton.vue';
import { FolderOpen, FileDateOne, FileText as OpenFileIcon, FileCabinet as OpenProjectIcon, FolderClose as OpenFolderIcon, Terminal as OpenProcIcon, Clipboard as OpenClipboardIcon, SaveOne } from '@icon-park/vue-next'
import { ElDivider, ElDropdownItem, ElDropdownMenu, ElMessage } from 'element-plus';
import { useTabsStore } from '../../stores/tabsStore'
import { RecentItem, useRecents, getRecentItemIcon } from '../../stores/recents';
import { toRaw } from 'vue';
import { LogTabData } from '../../stores/LogTabData';
import { saveProject, openProject } from '../../stores/project'
import { platform } from '../../ipc/platform';
import { useCollectStore } from '../../stores/collect';

const tabs = useTabsStore()

const { recents } = useRecents()

const {collect} = useCollectStore()

async function userSaveProject() {
  const { canceled, filePath } = await platform.showSaveDialog({
    properties: ['openFile'],
    filters: [
      { name: 'project file', extensions: ['liprj'] }
    ]
  })
  if (canceled)
    return

  const content = saveProject()
  collect('saveProject',{filePath})
  
  const errorMessage = await platform.writeFile(filePath!, JSON.stringify(content))

  if (errorMessage.length > 0) {
    ElMessage.error(errorMessage)
  }
}
</script>

<style></style>
