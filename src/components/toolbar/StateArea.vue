<template>
  <ToolArea>
    <template #up>
      <ElSelect v-model="view" style="width: 120px;">
        <ElOption value="main" label="主窗口" />
        <ElOption value="sub" label="过滤窗口" :disabled="disableSub"/>
      </ElSelect>
    </template>
    <template #down>
      <span>
        <ToggleToolAreaButton
         tooltip="跟随日志"
         v-model="followLog">
         <Refresh />
        </ToggleToolAreaButton>
        <ToolAreaButton tooltip="清除日志" @trigger="tab.clearLog()" v-if="tab.canClear">
          <VscodeIcon name="trash"/>
        </ToolAreaButton>
        <ElButton
          text
          size="small"
          @click ="()=>tab.toggleLogEncoding()"
        >
         {{ tab.logEncoding }}
        </ElButton>
      </span>
    </template>
  </ToolArea>
</template>

<script lang="ts" setup>

import ToolArea from './ToolArea.vue';
import {inject,computed, toRef, watch} from 'vue'
import { LogTabData } from "../../stores/LogTabData"
import {ElSelect, ElOption, ElButton} from 'element-plus'
import ToggleToolAreaButton from './ToggleToolAreaButton.vue';
import {Refresh} from "@icon-park/vue-next"
import ToolAreaButton from './ToolAreaButton.vue'
import VscodeIcon from '../VscodeIcon.vue';

const tab = inject<LogTabData>('tab')!
const disableSub = computed(()=>tab.activeSubIndex < 0)
const followLog = toRef(tab, 'followLog')
watch(followLog, ()=>{
  if (!followLog.value)
    return
  tab.rootLogView.goToBottom()
  for (const sub of tab.subLogViews) {
    sub.goToBottom()
  }
})

const view = computed({
  get: ()=>{
    return tab.activeLogView === tab.rootLogView ? 'main' : 'sub'
  },
  set: (v: 'main'|'sub')=>{
    if (v === 'main') {
      tab.activeLogView = tab.rootLogView
    } else {
      tab.activeLogView = tab.subLogViews[tab.activeSubIndex]
    }
  },
})

</script>

<style></style>