
<template>
  <div class="highlightManage">
    <ElButton v-if="highlights.length === 0" text class="helpButton" :icon="Add" @click="showHelp">
      添加高亮单词
    </ElButton>
    <ElTag v-else 
      v-for="item in highlights" 
      :key="item.keyword" 
      :color="item.color" 
      closable 
      @click="ev=>showMenu(ev, item)"
      @close="rmHighlight(item)">
      {{ item.keyword }}
    </ElTag>
    <ContextMenu ref="itemMenu" v-model:show="itemMenuVisible" :options="itemMenuOptions">
      <ContextMenuItem label="过滤" @click="onFilter" />
      <ContextMenuGroup label="更改颜色">
        <ColorMenuItems :colors="safeBgColorForWhiteText" v-model="curColor"/>
      </ContextMenuGroup>
    </ContextMenu>
  </div>
</template>

<script lang="ts" setup>

import { ElButton, ElColorPicker, ElMessage, ElTag } from 'element-plus';
import { inject, ref, toRef, reactive } from 'vue';
import { Highlight } from "../../stores/Highlight"
import { LogTabData } from "../../stores/LogTabData"
import { Add } from '@icon-park/vue-next'
import {safeBgColorForWhiteText, maybeLongOperation} from '../../stores/util'
import { ContextMenu, ContextMenuGroup, ContextMenuItem } from '@imengyu/vue3-context-menu';
import ColorMenuItems from '../ColorMenuItems.vue';

const tab = inject<LogTabData>('tab')!
const { highlights, rmHighlight } = tab
const itemMenu = ref<InstanceType<typeof ContextMenu>>()
const itemMenuVisible = ref(false)
const itemMenuOptions = reactive({
	theme: 'flat',
	x: 0,
	y: 0,
})
let curColor = ref('red')
let curItem: Highlight|null = null

function showHelp() {
  ElMessage.success("选中一个单词，然后右键高亮\n或选中单词后，按下E键添加高亮")
}

function onFilter() {
  if (!curItem)
    return

  const p = tab.addSubViewByFilter({
    caseSense:true,
    regex: false,
    pattern: curItem!.keyword,
    reverse: false,
    logId: tab.activeLogView.logId,
  })
  maybeLongOperation(`正在过滤关键字：${curItem!.keyword}`, p)
}

function showMenu(ev: MouseEvent, item: Highlight) {
  itemMenuOptions.x = ev.pageX
  itemMenuOptions.y = ev.pageY
  curColor = toRef(item, 'color')
  itemMenuVisible.value = true
  curItem = item
}

</script>

<style scoped>
.highlightManage {
  height: 100%;
  width: 180px;
  overflow-y: auto;
  flex-shrink: 0;
  display: flex;
  flex-wrap: wrap;
  gap: 4px;
}

.highlightManage .el-tag {
  --el-tag-text-color: white;
}

.helpButton {
  width: 100%;
  height: 100%;
}
</style>