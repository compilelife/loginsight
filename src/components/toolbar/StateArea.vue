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
import {inject,computed, toRef} from 'vue'
import { LogTabData } from "../../stores/LogTabData"
import {ElSelect, ElOption, ElButton} from 'element-plus'
import ToggleToolAreaButton from './ToggleToolAreaButton.vue';
import {Refresh} from "@icon-park/vue-next"

const tab = inject<LogTabData>('tab')!
const disableSub = computed(()=>tab.activeSubIndex < 0)
const followLog = toRef(tab, 'followLog')

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