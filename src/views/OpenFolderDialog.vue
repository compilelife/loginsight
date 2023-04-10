<template>
  <el-dialog v-model="visible" title="打开目录" width="60%">
    <div>
      <div class="row row1">
        <div>正在打开目录: {{ folder }}</div>
        <el-input v-model="suffix" placeholder="日志后缀"></el-input>
      </div>
      <div class="row">
        <el-select v-model="sortType" placeholder="排序方式">
          <el-option label="智能" value="smart"></el-option>
          <el-option label="时间序" value="time"></el-option>
          <el-option label="数字序" value="number"></el-option>
          <el-option label="字母序" value="letter"></el-option>
        </el-select>
        <el-input 
          @keyup.enter='sortLogs'
          :disabled="sortType !== 'number' && sortType !== 'letter'" 
          v-model="sortPattern" 
          :placeholder="patternHints[sortType]"></el-input>
      </div>
      <ElTable class="row" :data="logs" max-height="200" stripe border>
        <ElTableColumn label="日志文件打开顺序（从上往下）" prop="logs">
          <template #default="{row}">
            {{row}}
          </template>
        </ElTableColumn>
      </ElTable>
  </div>
    <div slot="footer" class="dialog-footer row">
      <el-button @click="visible = false">取消</el-button>
      <el-button type="primary" @click="onOpen">打开</el-button>
    </div>
  </el-dialog>
</template>

<script lang="ts" setup>

import { ElTableColumn,ElTable,ElButton,ElSelect,ElOption,ElInput,ElDialog, ElRow, ElCol, ElScrollbar, ElMessage } from 'element-plus'
import { ref, reactive,watch,computed } from 'vue';
const visible = ref(false)
const suffix = ref('')
type SortType = 'smart' | 'time' | 'number' | 'letter'
const sortType = ref<SortType>('smart')
const sortPattern = ref('')
const fileList = reactive<string[]>([])
const folder = ref('')

const patternHints: {
[K in SortType]: string;
} = {
  'smart': '无需填写正则表达式捕获',
  'time': '无需填写正则表达式捕获',
  'number': '填写至少有一个捕获的正则表达式，来捕获文件名中的数字部分；未填写则整个文件名当作数字进行比较',
  'letter': '填写至少有一个捕获的正则表达式，来捕获文件名中的字母部分；未填写则整个文件名当作字母进行比较'
}
const logs = ref<string[]>([])
function sortLogs(){
  //1. 构建一个临时数组tempLogs，如果suffix为空，则拷贝自fileList，否者作为文件名后缀过滤fileList
  const tempLogs = suffix.value !== '' ? fileList.filter(file => file.endsWith(suffix.value)) : [...fileList]
  //2.按sortType排序
  if(sortType.value === 'smart'){
    //2.1 如果是智能序：先尝试用[^0-9]*([0-9]+)[^0-9]*提取文件名中的数字部分，如果提取成功，就按这个数字部分排序；否则按整个文件名的字母序排列
    const reg = /[^0-9]*([0-9]+)[^0-9]*/
    try{
      tempLogs.sort((a,b)=>{
        const aMatch = a.match(reg)
        const bMatch = b.match(reg)
        if(aMatch && bMatch){
          return parseInt(aMatch[1]) - parseInt(bMatch[1])
        }else{
          return a.localeCompare(b)
        }
      })
    }catch(e){
      ElMessage.error('非法正则表达式')
    }
  }else if(sortType.value === 'time'){
    //2.2 如果是时间序：直接返回tempLogs
    return tempLogs
  }else if(sortType.value === 'number'){
    //2.3 如果是数字序：获取sortPattern捕获的第一项当作数字部分进行排序；如果sortPattern为空，整个文件名（除了后缀）当作数字进行排序
    try{
      tempLogs.sort((a,b)=>{
        const aMatch = sortPattern.value ? a.match(sortPattern.value) : a.match(/([0-9]+)/)
        const bMatch = sortPattern.value ? b.match(sortPattern.value) : b.match(/([0-9]+)/)
        if(aMatch && bMatch){
          return parseInt(aMatch[1]) - parseInt(bMatch[1])
        }else{
          return a.localeCompare(b)
        }
      })
    }catch(e){
      ElMessage.error('非法正则表达式')
    }
  }else if(sortType.value === 'letter'){
    //2.4 如果是字母序：获取sortPattern捕获的第一项当作字母部分进行排序；如果sortPattern为空，整个文件名（除了后缀）当作字母进行排序
    try{
      tempLogs.sort((a,b)=>{
        const aMatch = sortPattern.value ? a.match(sortPattern.value) : a.match(/([^0-9]+)/)
        const bMatch = sortPattern.value ? b.match(sortPattern.value) : b.match(/([^0-9]+)/)
        if(aMatch && bMatch){
          return aMatch[1].localeCompare(bMatch[1])
        }else{
          return a.localeCompare(b)
        }
      })
    }catch(e){
      ElMessage.error('非法正则表达式')
    }
  }
  logs.value = tempLogs
}

watch([sortType,fileList,suffix], sortLogs, {immediate: true})

let resolver : null | ((logs: string[])=>void)

watch(visible, () => {
  if (!visible && resolver)
    resolver([])
})

function onOpen() {
  visible.value = false
  resolver && resolver([...logs.value])
  resolver = null
}

async function show(dir: string, files: string[]) {
  folder.value = dir
  fileList.splice(0, fileList.length, ...files)
  visible.value = true
  return await new Promise<string[]>(resolve=>resolver = resolve)
}

defineExpose({show})

</script>

<style scoped>
.row {
  display: flex;
  margin-top: 10px;
  width: 100%;
}

.row1 > div:first-child {
  flex-grow: 1;
}
.row1 >div:last-child {
  flex-grow: 0;
  flex-shrink: 0;
  flex-basis: 80px;
}

.dialog-footer {
  justify-content: flex-end;
}
</style>