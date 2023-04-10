<template>
  <el-dialog v-model="visible" title="打开程序" width="60%">
    <el-form :model="form" >
      <el-form-item label="程序路径">
        <el-input v-model="form.process"></el-input>
      </el-form-item>
      <el-form-item label="缓存上限" >
        <el-input-number v-model="form.cache" :min="2" :max="10000"></el-input-number>
        <span style="margin-left: 10px;">块（一个块200K）</span>
      </el-form-item>
    </el-form>
    <el-table 
      height="200"
      empty-text="无历史记录"
      stripe border
      :data="tableData" 
      style="width: 100%" 
      @row-click="handleRowClick">
      <el-table-column prop="process" label="程序" min-width="200"></el-table-column>
      <el-table-column prop="cache" label="缓存"></el-table-column>
      <el-table-column label="操作">
        <template #default="{row}">
          <el-link type="danger" @click="handleDelete(row)">删除</el-link>
        </template>
      </el-table-column>
    </el-table>
    <div slot="footer" class="dialog-footer" style="text-align: right;">
      <el-button type="primary" @click="handleConfirm">确 定</el-button>
      <el-button @click="visible = false">取 消</el-button>
    </div>
  </el-dialog>
</template>

<script lang="ts" setup>
import { watch, reactive, ref, onMounted, toRaw } from 'vue';

const visible = ref(false);

const form = reactive<OpenProcessArg>({
  process: '',
  cache: 100
});
const tableData = reactive<Array<OpenProcessArg>>([]);

let resultResolver: null | ((arg: OpenProcessArg|null) => void) = null;
watch(visible, () => {
  if (!visible && resultResolver)
    resultResolver(null)
})

const handleConfirm = () => {
  const isExist = tableData.some(item => item.process === form.process)
  if (!isExist) {
    tableData.push({
      process: form.process,
      cache: form.cache
    });
    localStorage.setItem('procHistories', JSON.stringify(tableData));
  }
  resultResolver && resultResolver(toRaw(form))
  resultResolver = null
  visible.value = false
};

const handleDelete = (row: OpenProcessArg) => {
  const index = tableData.indexOf(row);
  if (index !== -1) {
    tableData.splice(index, 1);
    localStorage.setItem('procHistories', JSON.stringify(tableData));
  }
};

const handleRowClick = (row: OpenProcessArg) => {
  form.process = row.process;
  form.cache = row.cache;
};

function show() {
  visible.value = true;
  return new Promise<OpenProcessArg|null>(resolve => {
    resultResolver = resolve
  })
}

onMounted(() => {
  const data = localStorage.getItem('procHistories');
  if (data) {
    tableData.push(...JSON.parse(data));
  }
});

defineExpose({show})

</script>
