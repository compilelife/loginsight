<template>
  <el-dialog v-model="visible" width="70%" title="订阅">
    <el-row style="margin: 10px;" :gutter="20">
      <el-col :span="12">
        <el-card shadow="hover" >
          <div slot="header" class="header">
            <h3>免费版</h3>
          </div>
          <el-row v-for="(item, index) of freeList" :key="item" class="ability">
            <CheckOne theme="outline" size="24" fill="#333" class="icon"/>
            {{ item }}
          </el-row>
        </el-card>
      </el-col>
      <el-col :span="12">
        <el-card shadow="hover" style="background-color: #EBEEF5;">
          <el-row slot="header" class="header" >
            <el-col :span="6"><h3>专业版</h3></el-col>
            <el-col :span="18" class="endbuttons">
              <el-button type="primary" @click="onBuy">订阅</el-button>
              <el-button @click="onRestore">恢复已购</el-button>
            </el-col>
          </el-row>
          <el-row v-for="(item, index) of proList" :key="item" class="ability">
            <CheckOne theme="two-tone" size="24" :fill="['#ffffff' ,'#7ed321']"  class="icon"/>
            {{ item }}
          </el-row>
        </el-card>
      </el-col>
    </el-row>
    <el-divider></el-divider>
    <span style="color: #909399;">[1] 能打开的文件大小的上限与设备的内存大小有关 </span>
  </el-dialog>
</template>

<script lang="ts" setup>
import {CheckOne} from '@icon-park/vue-next'
import {ref} from 'vue'
import { useRegister } from '../stores/register';
import { platform } from '../ipc/platform';

const freeList = ref([
  "打开100M以下的文件",
  "最多2个过滤窗口",
  "最多5个时间线节点"
])

const proList = ref([
  "打开文件大小不受限[1]",
  "过滤窗口个数不受限",
  "时间线节点个数不受限",
  "可以打开多文件日志（打开文件夹）",
  "可以保存和打开工程"
])

const visible = ref(false)
const register = useRegister()

const onBuy = () => {
  register.doRegister({orderId: '1234567890'  })
}

const onRestore = () => {
  platform.restorePurchase()
}

const show = () => {
  visible.value = true
}

defineExpose({
  show
})

</script>

<style scoped>
.header {
  border-bottom: 1px solid #ccc;
  align-items:center;
}
.ability {
  margin-top: 10px;
}
.icon{
  margin-right: 5px;
}
.endbuttons {
  display: flex;
  justify-content: end;
}
</style>