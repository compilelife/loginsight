<template>
  <div ref="logviewElem" class="wrapper">
    <ul 
      @wheel.prevent="onUserScroll($event)" 
      v-if="firstLoaded && !data.forceRefreshing" 
      @click.right.prevent="onContextMenu($event)"
      class="logview">
      <LogLine v-for="i in displayRange" 
        :key="i" 
        @requestFocus="onLineFocused" 
        :syntax-fields="tab.curSyntax.fields"
        :active="active"
        :focused="i == data.focusLineIndex" 
        :numWidth="numWidth" 
        :search-result="getSearchResult(i)" 
        :line="getLine(i)" />
    </ul>
    <ScrollBarByLine :count="data.lineCount" :preload="preload" :onFocused="setActivated" v-model="currentIndex" />
    <LogViewMenu :view="data" ref="contextMenu" />
  </div>
</template>

<script setup lang="ts">

import { ref, watch, computed, toRef, onMounted, inject , Ref} from 'vue'
import LogLine from './LogLine.vue';
import ScrollBarByLine from './ScrollBarByLine.vue';
import { useElementSize, useTextSelection } from '@vueuse/core'
import LogViewMenu from './LogViewMenu.vue';
import { LogViewData } from "../stores/LogViewData"
import { LogTabData } from "../stores/LogTabData"
import { isValidRange } from '../ipc/platform';
import { measureTextWidth } from '../stores/util';
import {useSettingsStore} from '../stores/Settings'
import { storeToRefs } from 'pinia';

const { data } = defineProps<{
  data: LogViewData
}>()

const { preload, getLine } = data

const tab = inject<LogTabData>('tab')!
const active = computed(() => tab.isActive(data))

const settings = useSettingsStore()
const {logFontSize, cssLogFont, lineSpacing} = storeToRefs(settings)

const currentIndex = toRef(data, 'curLineIndex')
const logviewElem = ref<HTMLElement>()
const numWidth = computed(()=>measureTextWidth(`${tab.rootLogView.lineCount}`, cssLogFont.value)+16)
const displayRange = computed(() => {
  if (!isValidRange(data.range))
    return []
  return Array.from({ length: data.displayCount }, (_, i) => currentIndex.value + i)
})


function setActivated() {
  tab.activeLogView = data
}

//搜索结果
function getSearchResult(index: number) {
  const found = data.lastSearchResult?.found
  if (found) {
    return index === data.lastSearchResult?.index ? data.lastSearchResult : null
  }
  return null
}

//第一次加载
const firstLoaded = ref(false)
onMounted(async () => {
  await preload(currentIndex.value)
  firstLoaded.value = true
})

//行焦点
function onLineFocused(index: number) {
  data.focusLineIndex = index
  setActivated()
}

//非滚动条区域的滚动
async function onUserScroll(ev: WheelEvent) {
  let targetIndex = currentIndex.value
  if (ev.deltaY < 0) {
    targetIndex = Math.max(data.range.begin, currentIndex.value - 1)
  } else {
    targetIndex = Math.min(data.range.end, currentIndex.value + 1)
  }

  if (data.caching)
    return

  await preload(targetIndex)
  currentIndex.value = targetIndex

  setActivated()
}

//可见区域高度变化
const { height: logViewHeight } = useElementSize(logviewElem)
watch(logViewHeight, () => {
  data.displayCount = Math.round(logViewHeight.value / (logFontSize.value * lineSpacing.value)) + 1 //15:font-size, 1.5行间距
})

//右键菜单
const { text: selectedText } = useTextSelection()
watch(selectedText, () => {data.selectedWord = selectedText.value})

const contextMenu = ref<InstanceType<typeof LogViewMenu>>()
function onContextMenu(ev: MouseEvent) {
  contextMenu.value?.popup(selectedText.value, ev.pageX, ev.pageY)
}

//这里delay10ms是为了等待日志行的这行排列完成
//FIXME:首次加载后的last line计算不准
watch([currentIndex, firstLoaded], ()=>setTimeout(() => {
  const lines = logviewElem.value?.getElementsByTagName('li')
  if (!lines)
    return
  
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    if (line.offsetTop + line.offsetHeight >= logViewHeight.value) {
      data.visibleLastLineIndex = currentIndex.value + i
      return
    }
  }

  if (lines.length > 0) {
    data.visibleLastLineIndex = currentIndex.value + lines.length - 1
  }
}, 10),{flush: 'post'})
</script>

<style scoped>
.wrapper {
  position: relative;
  width: 100%;
  height: 100%;
  display: flex;
  flex-direction: row;
  justify-content: space-between;
}

.logview {
  overflow: hidden;
  width: 100%;
  height: 100%;
  padding: 0;
  margin: 0;
}
</style>

<style>
mark {
  color: initial;
  background-color: initial;
}
</style>