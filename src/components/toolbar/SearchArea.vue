<template>
  <ToolArea>
    <template #up>
      <ElAutocomplete 
        ref="inputRef"
        v-model="pattern"
        :trigger-on-focus="true"
        :fetch-suggestions="fetchSuggestions"
        value-key="pattern"
        placeholder="输入要搜索的词"
        :clearable="true"
        @select="(item:any)=>selectSuggestion(item)"
        @keyup.enter="tab.search(true)"
        style="width: 150px;" >
        <template #suffix>
          <ElIcon @click="clearSearchText"><Close/></ElIcon>
        </template>
      </ElAutocomplete>
    </template>
    <template #down>
      <span>
        <ToggleToolAreaButton 
          v-model="caseSense"
          tooltip="区分大小写">
          <FontSize />
        </ToggleToolAreaButton>
        <ToggleToolAreaButton
          v-model="regex"
          tooltip="使用正则表达式">
          <Block />
        </ToggleToolAreaButton>
        <ToolAreaButton tooltip="下一个匹配项" @trigger="tab.search(true)">
          <ArrowDown />
        </ToolAreaButton>
        <ToolAreaButton tooltip="上一个匹配项" @trigger="tab.search(false)">
          <ArrowUp />
        </ToolAreaButton>
        <ToolAreaButton tooltip="清除搜索高亮" @trigger="clearSearchHighlight">
          <ClearFormat />
        </ToolAreaButton>
      </span>
    </template>
  </ToolArea>
</template>

<script setup lang="ts">
import ToolArea from './ToolArea.vue';
import {ElAutocomplete, ElIcon} from 'element-plus'
import ToolAreaButton from './ToolAreaButton.vue'
import {ArrowUp, ArrowDown, Block,FontSize, ClearFormat, Close} from '@icon-park/vue-next'
import { inject, toRef, ref } from 'vue';
import { LogTabData } from '../../stores/LogTabData';
import ToggleToolAreaButton from './ToggleToolAreaButton.vue';
import { SearchHistory } from '../../stores/SearchData';

const tab = inject<LogTabData>('tab')!
const searchData = tab.searchData
const caseSense = toRef(searchData, 'caseSense')
const regex = toRef(searchData, 'regex')
const pattern = toRef(searchData, 'pattern')
const inputRef = ref<InstanceType<typeof ElAutocomplete>>()

function fetchSuggestions(queryString: string, cb: Function) {
  cb(searchData.getSuggestion(queryString))
}

function clearSearchHighlight() {
  tab.activeLogView.lastSearchResult = null
}

function clearSearchText() {
  tab.searchData.pattern = ''
}

function selectSuggestion(item: SearchHistory) {
  searchData.caseSense = item.caseSense
  searchData.regex = item.regex
  searchData.pattern = item.pattern
}

tab.toolActions.beginSearch = ()=>{
  inputRef.value?.focus()
  setTimeout(() => {//FIXME: 这里是一个临时措施，用于防止聚焦后快捷键's'会被输入到输入框内
    pattern.value = ''
  }, 10);
}

</script>

