<template>
  <ToolArea>
    <template #up>
      <ElInputNumber type="number" style="width: 120px;" 
        ref="lineInput"
        controls
        v-model="line"
        @keyup.enter="jump"
        :min="minLine"
        :max="maxLine"/>
    </template>
    <template #down>
      <span>
        <ToolAreaButton tooltip="第一行" @trigger="gotoTop">
          <ArrowUp />
        </ToolAreaButton>
        <ToolAreaButton tooltip="最后一行" @trigger="gotoBottom">
          <ArrowDown />
        </ToolAreaButton>
        <ToolAreaButton tooltip="回退" @trigger="goBack" :disabled="!canGoBack">
          <ArrowLeft />
        </ToolAreaButton>
        <ToolAreaButton tooltip="前进" @trigger="goForward" :disabled="!canGoForward">
          <ArrowRight />
        </ToolAreaButton>
        <ToolAreaButton tooltip="追踪当前行" @trigger="trackCurLine">
          <ConnectionArrow />
        </ToolAreaButton>
      </span>
    </template>
  </ToolArea>
</template>

<script setup lang="ts">
import ToolArea from './ToolArea.vue'
import ToolAreaButton from './ToolAreaButton.vue'
import {ArrowLeft, ArrowRight, ArrowUp, ArrowDown} from '@element-plus/icons-vue'
import { ConnectionArrow} from '@icon-park/vue-next'
import { computed, inject, ref } from 'vue';
import { LogTabData } from '../../stores/LogTabData';
import { ElInput, ElInputNumber } from 'element-plus';

const tab = inject<LogTabData>("tab")!
const minLine = computed(()=>tab.activeLogView.range.begin+1)
const maxLine = computed(()=>tab.activeLogView.range.end+1)
const canGoBack = computed(()=>tab.activeLogView.canGoBack)
const canGoForward = computed(()=>tab.activeLogView.canGoForward)
const line = ref(0)

const lineInput = ref<InstanceType<typeof ElInput>>()

function gotoTop(){
  const view = tab.activeLogView
  const topIndex = view.range.begin
  view.jumpTo(topIndex)
}

function gotoBottom() {
  tab.activeLogView.goToBottom()
}

function goBack() {
  tab.activeLogView.goBack()
}
function goForward() {
  tab.activeLogView.goForward()
}

function trackCurLine() {
  const view = tab.activeLogView
  tab.trackLine(view.focusLineIndex, view)
}

function jump() {
  tab.activeLogView.jumpTo(line.value-1)
}

tab.toolActions.beginJump = ()=>lineInput.value?.focus()

</script>