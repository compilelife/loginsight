<template>
  <ContextMenu v-model:show="show" :options="options">
    <ContextMenuGroup label="更改颜色">
      <ContextMenuItem v-for="color in prop.colors" :style="{ 'background-color': color }" class="colorItem"
        @click="curColor = color" :label="color">
      </ContextMenuItem>
      <ContextMenuItem class="moreColor" :click-close="false" @click="colorInputRef?.click()">
        更多颜色 <input ref="colorInputRef" v-model="curColor" type="color">
      </ContextMenuItem>
    </ContextMenuGroup>
    <ContextMenuItem label="删除" @click="remove"/>
    <ContextMenuItem label="追踪" @click="track"/>
  </ContextMenu>
</template>
  
<script lang="ts" setup>

import { ContextMenu, ContextMenuGroup, ContextMenuItem } from '@imengyu/vue3-context-menu';
import { reactive, toRef, ref, inject } from 'vue';
import { TimeLineNodeData } from "../stores/TimeLineData"
import { LogTabData } from "../stores/LogTabData"

const prop = defineProps<{
  colors: string[]
}>()

const tab = inject<LogTabData>('tab')!

const show = ref(false)
const colorInputRef = ref<HTMLElement>()
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