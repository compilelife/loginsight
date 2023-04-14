<template>
  <ContextMenu v-model:show="show" :options="options">
    <ContextMenuItem label="追踪" @click="track"/>
    <ContextMenuGroup label="更改颜色">
      <ColorMenuItems :colors="colors" v-model="curColor"/>
    </ContextMenuGroup>
    <ContextMenuItem label="删除" @click="remove"/>
  </ContextMenu>
</template>
  
<script lang="ts" setup>

import { ContextMenu, ContextMenuGroup, ContextMenuItem } from '@imengyu/vue3-context-menu';
import { reactive, toRef, ref, inject } from 'vue';
import { TimeLineNodeData } from "../stores/TimeLineData"
import { LogTabData } from "../stores/LogTabData"
import ColorMenuItems from './ColorMenuItems.vue';

const prop = defineProps<{
  colors: string[]
}>()

const tab = inject<LogTabData>('tab')!

const show = ref(false)
let curColor = ref('red')
const options = reactive({
  theme: 'flat',
  x: 0,
  y: 0,
})
let item: TimeLineNodeData

function popup(x: number, y: number, target:TimeLineNodeData) {
  show.value = true
  options.x = x
  options.y = y
  item = target
  curColor = toRef(item, 'color')
}

function remove() {
  tab.timeline.rmNode(item)
}

function track() {
  tab.trackLine(item.line, tab.timeline)
}

defineExpose({ popup })

</script>
  
<style></style>