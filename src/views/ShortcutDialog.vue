<template>
  <el-dialog v-model="visible">
    (仅在打开日志后有效：)
    <el-table :data="shortcuts">
      <el-table-column prop="key" label="按键">
        <template #default="{row}">
          <kbd>{{row.key}}</kbd>
        </template>
      </el-table-column>
      <el-table-column prop="desc" label="描述"></el-table-column>
    </el-table>
  </el-dialog>
</template>
<script setup lang="ts">
import {logTabBindings } from "../stores/shortcuts";
import {ref} from 'vue'
const visible = ref(false)

function show() {
  visible.value = true
}

const shortcuts = logTabBindings.map(b=>({
  key: b.key.toUpperCase(),
  desc: b.desc
}))

defineExpose({show})
</script> 

