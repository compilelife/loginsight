<template>
  <el-dialog title="注册" v-model="visible" width="60%">
    <el-alert v-if="alert.visible" :title="alert.title" :type="alert.type" closable center @close="alert.visible=false"/>
    <el-form :model="form">
      <el-form-item label="机器码：">
        <el-input v-model="form.machineCode" :disabled="true"></el-input>
      </el-form-item>
      <el-form-item label="输入订单号：">
        <el-input v-model="form.orderNumber"></el-input>
      </el-form-item>
      <el-form-item label="购买获取订单号：">
        <el-button type="text" @click="goTo('Windows')">Windows</el-button>
        <el-button type="text" @click="goTo('MacOS')">MacOS</el-button>
        <el-button type="text" @click="goTo('Linux')">Linux</el-button>
        <el-button type="text" @click="goTo('All')">全平台</el-button>
      </el-form-item>
    </el-form>
    <div slot="footer" class="dialog-footer" style="text-align: right;">
      <el-button type="primary" @click="onConfirm">确 定</el-button>
      <el-button @click="onCancel">取 消</el-button>
    </div>
  </el-dialog>
</template>

<script lang="ts" setup>

import {ref, onMounted, onUnmounted} from 'vue'
import { useRegister } from '../stores/register';

const visible = ref(false)
const register = useRegister()

function show() {
  visible.value = true
}

const alert = ref({
  type: 'success',
  title: '',
  visible: false,
})

const form = ref({
  machineCode: window.host.machineId,
  orderNumber: ''
})

async function onConfirm() {
  const ret = await register.doRegister({orderId: form.value.orderNumber})
  if (ret.ok) {
    alert.value = {
      title: '注册成功。请重启！',
      type: 'success',
      visible: true
    }
  } else {
    alert.value = {
      title: '注册失败：'+ret.info,
      type: 'error',
      visible: true
    }
  }
}

function onCancel() {
  visible.value = false
}

function goTo(plat:string) {
  const url = {
    'Windows': 'https://mbd.pub/o/bread/YZibl55q',
    'MacOS': 'https:/mbd.pub/o/bread/YZibmZpx',
    'Linux': 'https:/mbd.pub/o/bread/YZibmZtp',
    'All': 'https://mbd.pub/o/bread/YZibmZtw'
  }[plat]
  if (url)
    window.open(url)
}

defineExpose({show})
</script>
