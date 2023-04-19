<template>
  <ElDialog
    v-model="visible"
    width="90%"
    title="语法管理">
    <!-- 表格 -->
    <el-table border :data="items" @current-change="handleCurrentChange" :highlight-current-row="true">
      <el-table-column prop="name" label="名字">
        <template #default="{row}">
          <el-input v-if="row === curItem && curEditing" 
            v-model="row.name" 
            @change="finishRename"></el-input>
          <span v-else>{{ row.name }}</span>
        </template>
      </el-table-column>
      <el-table-column prop="pattern" label="匹配规则"></el-table-column>
      <el-table-column label="操作">
        <template #default="{ row, $index }">
          <el-button type="success" link @click="select(row)">选择</el-button>
          <el-button  link @click="curEditing=true">重命名</el-button>
          <el-popconfirm title="确定要删除吗？" @confirm="removeItem($index)">
            <template #reference>
              <el-button type="danger" link>删除</el-button>
            </template>
          </el-popconfirm>
        </template>
      </el-table-column>
    </el-table>
    <!-- 高亮预览区 -->
    <div class="preview">
      <pre v-html="curItem?.previewLog" class="logText"></pre>
    </div>
  </ElDialog>
</template>

<script setup lang="ts">

import { computed, ref, watch, watchEffect } from 'vue'
import { ElTable, ElTableColumn, ElButton, ElInput, ElDialog, ElPopconfirm } from 'element-plus'
import {SyntaxStoreItem, useSyntaxStore} from '../stores/syntax'
import { storeToRefs } from 'pinia'

const syntaxs = useSyntaxStore()
const visible = ref(false)

const {items} = storeToRefs(syntaxs)
const {removeItem, sync} = syntaxs

const curItem = ref<SyntaxStoreItem|null>(null)
const curEditing = ref(false)

let resolver: null | ((v:SyntaxStoreItem|null)=>void) = null

const handleCurrentChange = (row: SyntaxStoreItem|null) => {
  curItem.value = row
}

const select = (row: SyntaxStoreItem) => {
  resolver && resolver(row)
  visible.value = false
}

watch(visible, ()=>{
  if (!visible.value && resolver) {
    resolver(null)
  }
})

function show() {
  visible.value = true

  return new Promise<SyntaxStoreItem|null>(r=>resolver = r)
}

function finishRename() {
  curEditing.value = false
  sync()
}

defineExpose({
  show
})

</script>

<style scoped>
.preview {
  margin-top: 20px;
  padding: 0 10px;
  border: 1px solid #EBEEF5;
}

.logText{
  white-space: pre-wrap;
	word-break: break-all;
}
</style>
