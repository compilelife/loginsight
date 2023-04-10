<template>
  <ToolArea style="white-space: nowrap;">
    <template #up>
      <span>
        <ToolAreaButton tooltip="添加当前行到时间线" @trigger="tab.addFocusedLineToTimeLine">
          <ListAdd />
        </ToolAreaButton>
        <ToolAreaButton tooltip="清空时间线" @trigger="tab.timeline.clear">
          <Clear />
        </ToolAreaButton>
      </span>
    </template>
    <template #down>
      <div class="down">
        <ToolAreaButton tooltip="复制时间线到剪贴板" @trigger="saveToClipboard">
          <Clipboard />
        </ToolAreaButton>
        <ElTooltip
        effect="light"
        :show-after="1000"
        content="设置新建时间节点的颜色">
          <div class="pickColor"
            @click="ev=>onPickColor(ev)">
          </div>
        </ElTooltip>
      </div>
    </template>
  </ToolArea>
  <PickColorMenu 
  ref="pickColorMenu"
  :colors="safeColorForWhiteBg" />
</template>

<script lang="ts" setup>
import ToolArea from "./ToolArea.vue"
import ToolAreaButton from "./ToolAreaButton.vue"
import { ListAdd, Clear, Clipboard, Square } from '@icon-park/vue-next'
import { inject, ref } from "vue";
import { LogTabData } from "../../stores/LogTabData";
import html2canvas from 'html2canvas'
import { platform } from "../../ipc/platform";
import { safeColorForWhiteBg } from '../../stores/util';
import PickColorMenu from '../PickColorMenu.vue';
import { storeToRefs } from "pinia";
import { ElTooltip } from "element-plus";

const tab = inject<LogTabData>('tab')!
const pickColorMenu = ref<InstanceType<typeof PickColorMenu>>()
const {curColor} = storeToRefs(tab.timeline)

async function saveToClipboard() {
  if (tab.timeline.htmlRef) {
    const htmlEl = tab.timeline.htmlRef.$el
    const canvas = await html2canvas(htmlEl, {scale: 2, x: -10, y: -10, width: htmlEl.offsetWidth + 20, height: htmlEl.offsetHeight + 20})
    const imgData = canvas.toDataURL()
    platform.saveImageToClipboard(imgData)
  }
}

function onPickColor(ev:MouseEvent) {
  pickColorMenu.value?.popup(ev.pageX, ev.pageY, curColor)
}

</script>

<style scoped>
.pickColor {
  width: 18px;
  height: 18px;
  background-color: v-bind(curColor);
}
.down {
  display: flex;
  align-items: center;
  justify-content: space-between;
  width: 100%;
}
</style>