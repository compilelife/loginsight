<template>
<template v-if="showFilterForm">
  <ToolArea>
    <template #up>
      <ElInput 
        ref="inputRef" 
        @keyup.enter="doFilter"
        v-model="pattern"
        placeholder="输入要过滤的单词">
        <template #prefix>
          <Filter :theme="revertFilter?'filled':'outline'"/>
        </template>
      </ElInput>
    </template>
    <template #down>
      <span>
        <ToggleToolAreaButton 
          v-model="caseSense"
          tooltip="区分大小写">
          <VscodeIcon name="case-sensitive"/>
        </ToggleToolAreaButton>
        <ToggleToolAreaButton
          v-model="regex"
          tooltip="使用正则表达式">
          <VscodeIcon name="regex"/>
        </ToggleToolAreaButton>
        <ElButton size="small" type="primary" @click="doFilter" text>
          确定
        </ElButton>
        <ElButton size="small" @click="cancelFilter" text>
          取消
        </ElButton>
      </span>
    </template>
  </ToolArea>
</template>
<template v-else>
  <ToolButton label="过滤" @click="beginFilter(false)">
    <Filter />
  </ToolButton>
  <ToolButton label="反过滤" @click="beginFilter(true)">
    <Filter theme="filled" />
  </ToolButton>
  <ElDropdown trigger="click">
    <ToolButton label="语法"><Code /></ToolButton>
    <template #dropdown>
      <ElDropdownItem v-for="item of syntaxs.items"
        @click="tab.setSyntax(item)">{{ item.name }}</ElDropdownItem>
      <ElDropdownItem @click="showSyntaxDlg">新增</ElDropdownItem>
    </template>
  </ElDropdown>
</template>
</template>

<script lang="ts" setup>

import ToolButton from './ToolButton.vue';
import { inject, ref, toRaw, watch,nextTick } from 'vue'
import { LogTabData } from "../../stores/LogTabData";
import {Filter, Code} from '@icon-park/vue-next'
import ToolArea from './ToolArea.vue';
import { ElButton, ElDropdown, ElDropdownItem, ElInput, Instance } from 'element-plus';
import ToggleToolAreaButton from './ToggleToolAreaButton.vue';
import {FontSize, Block} from '@icon-park/vue-next'
import {maybeLongOperation} from '../../stores/util'
import { useDialogStore } from '../../stores/dialogs';
import { useSyntaxStore } from '../../stores/syntax';
import VscodeIcon from '../VscodeIcon.vue';

const tab = inject<LogTabData>('tab')!

const showFilterForm = ref(false)
const revertFilter = ref(false)

const pattern = ref('')
const caseSense = ref(true)
const regex = ref(false)

const inputRef = ref<InstanceType<typeof ElInput>>()
watch(inputRef, ()=>{
  if (inputRef.value) {
    inputRef.value?.focus()
    setTimeout(() => {//FIXME: 这里是一个临时措施，用于防止聚焦后快捷键's'会被输入到输入框内
      pattern.value = ''
    }, 10);
  }
})

const syntaxs = useSyntaxStore()

function beginFilter(revert: boolean) {
  revertFilter.value = revert
  showFilterForm.value = true
}

function doFilter() {
  tab.filter({
    caseSense: caseSense.value,
    regex: regex.value,
    pattern: pattern.value,
    logId: 0,
    reverse: revertFilter.value
  })
  
  showFilterForm.value = false
}

function cancelFilter() {
  showFilterForm.value = false
}

const {showSyntaxDlg: showSyntaxPreview} = useDialogStore()

function showSyntaxDlg() {
  const curIndex = tab.activeLogView.curLineIndex
  const lines = tab.activeLogView
    .getLines(curIndex, curIndex + 20)
    .map(l=>({...toRaw(l)}))
  showSyntaxPreview(tab, lines)
}

tab.toolActions.beginFilter = ()=>beginFilter(false)

</script>

<style></style>