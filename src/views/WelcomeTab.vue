<template>
  <div class="welcome">
    <h1>Loginsight</h1>
    <ElRow>
      <ElCol :span="12">
        <h2>启动</h2>
        <ul>
          <li>
            <ElButton link type="primary" @click="tabs.openFile">打开文件</ElButton>
          </li>
          <li>
            <ElButton link type="primary" @click="tabs.openFolder">打开文件夹</ElButton>
          </li>
          <li>
            <ElButton link type="primary" @click="tabs.openProc">打开外部程序</ElButton>
          </li>
          <li>
            <ElButton link type="primary" @click="tabs.openClipboard">打开粘贴板</ElButton>
          </li>
          <li>
            <ElButton link type="primary" @click="tabs.openProject">打开项目</ElButton>
          </li>
        </ul>
      </ElCol>
      <ElCol :span="12">
        <h2>帮助</h2>
        <ul>
          <li>
            <ElButton link type="primary" @click="showSettingsDlg">设置</ElButton>
          </li>
          <li>
            <ElButton link type="primary" @click="showShortcutDlg">快捷键</ElButton>
          </li>
          <li>
            <ElButton link type="primary" @click="gotoGithub">开源地址</ElButton>
          </li>
          <li>
            <ElButton link type="primary" @click="()=>platform.openDevTool()">开发者工具</ElButton>
          </li>
        </ul>
      </ElCol>
    </ElRow>
    <ElRow style="height: 30vh;">
      <ElCol :span="12">
        <h2>最近打开</h2>
        <ul>
          <li v-for="recent of recents" :key="recent.name">
            <ElButton link type="primary" :icon="getRecentItemIcon(recent.type)" @click="tabs.openRecentItem(recent)">
              {{ recent.name }}
            </ElButton>
          </li>
        </ul>
      </ElCol>
      <ElCol class="newVersion" :span="12">
        <h2>新版本: 
          <ElButton v-if="update.hasNewVersion" link type="primary" @click="gotoDownload">{{update.newVersion!.version}}</ElButton>
          <ElButton v-else link type="primary" @click="checkNewVersion">点击检查</ElButton>
        </h2>
        <ElScrollbar v-if="update.hasNewVersion">
          <ElText v-html="changeLog" style="width: 80%; display: inline-block;"></ElText>
        </ElScrollbar>
      </ElCol>
    </ElRow>
  </div>
</template>

<script lang="ts" setup>
import { ElButton, ElCol, ElMessage, ElRow, ElScrollbar, ElText } from 'element-plus';
import { storeToRefs } from 'pinia';
import { useDialogStore } from '../stores/dialogs';
import { useRecents, getRecentItemIcon } from '../stores/recents';
import { useTabsStore } from '../stores/tabsStore';
import {platform} from '../ipc/platform'
import {useUpdateStore} from '../stores/update'
import { onMounted, computed } from 'vue';
import {UpdateRotation} from '@icon-park/vue-next'

const {showSettingsDlg, showShortcutDlg} = useDialogStore()

const { recents } = storeToRefs(useRecents())

const update = useUpdateStore()
const changeLog = computed(()=>{
  if (update.hasNewVersion) {
    return update.newVersion!.change.replaceAll('\n', '<br>')
  }
  return ''
})

const tabs = useTabsStore()

function gotoDownload() {
  window.open('https://mbd.pub/o/bread/YZmYkpts')
}

function gotoGithub() {
  window.open('https://github.com/compilelife/loginsight')
}

async function checkNewVersion() {
  await update.checkNewVersion()
  if (!update.hasNewVersion) {
    ElMessage.warning('已经是最新版本')
  }
}

onMounted(()=>{update.checkNewVersion()})

</script>

<style scoped>
.welcome {
  padding: 40px;
  width: 100%;
  height: 100%;
}

li {
  margin: 0;  
  padding: 0;
  list-style: none;
}

ul {
  padding-inline-start: 0;
}

.newVersion {
  display: flex;
  flex-direction: column;
  height: 100%;
}

</style>
