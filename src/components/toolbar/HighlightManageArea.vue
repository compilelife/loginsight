
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
      @click="ev=>showPickColorMenu(ev, item)"
      @close="rmHighlight(item)">
      {{ item.keyword }}
    </ElTag>
    <PickColorMenu 
      ref="pickColorMenu"
      :colors="safeBgColorForWhiteText" />
  </div>
</template>

<script lang="ts" setup>

import { ElButton, ElColorPicker, ElMessage, ElTag } from 'element-plus';
import { inject, ref, toRef } from 'vue';
import { Highlight } from "../../stores/Highlight"
import { LogTabData } from "../../stores/LogTabData"
import { Add } from '@icon-park/vue-next'
import PickColorMenu from '../PickColorMenu.vue';
import {safeBgColorForWhiteText} from '../../stores/util'

const tab = inject<LogTabData>('tab')!
const { highlights, rmHighlight } = tab
const pickColorMenu = ref<InstanceType<typeof PickColorMenu>>()

function showHelp() {
  ElMessage.success("选中一个单词，然后右键高亮\n或选中单词后，按下E键添加高亮")
}

function showPickColorMenu(ev: MouseEvent, item: Highlight) {
  pickColorMenu.value?.popup(ev.pageX, ev.pageY, toRef(item, 'color'))
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