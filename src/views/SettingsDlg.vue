<template>
  <el-dialog v-model="visible" width="50%" title="设置" 
    draggable 
    :close-on-click-modal="false"
    :close-on-press-escape="false"
    :show-close="false">
    <el-row style="margin-bottom: 10px;">
      <el-col :span="8">日志字体:</el-col>
      <el-col :span="8">
        <el-input-number v-model="logFontSize" :min="10" :max="30" :step="1"></el-input-number>
      </el-col>
      <el-col :span="8">
        <el-select v-model="logFontFamily">
          <el-option v-for="font in fontList" :key="font" :label="font" :value="font"></el-option>
        </el-select>
      </el-col>
    </el-row>
    <el-row style="margin-bottom: 10px;">
      <el-col :span="8">默认编码:</el-col>
      <el-col :span="16">
        <el-select v-model="defaultEncoding">
          <el-option label="UTF-8" value="utf-8"></el-option>
          <el-option label="GBK" value="gbk"></el-option>
        </el-select>
      </el-col>
    </el-row>
    <el-row style="margin-bottom: 10px;">
      <el-col :span="8">行间距:</el-col>
      <el-col :span="16">
        <el-input-number v-model="lineSpacing" :min="1" :max="2" :step="0.1"></el-input-number>
      </el-col>
    </el-row>
    <el-row style="margin-bottom: 10px;">
      <el-col :span="8">版本号:</el-col>
      <el-col :span="16">{{currentVersion}}</el-col>
    </el-row>
    <div slot="footer" class="dialog-footer" style="text-align: right;">
      <el-button type="primary" @click="onConfirm">确 定</el-button>
      <el-button @click="onCancel">取 消</el-button>
    </div>
  </el-dialog>
</template>

<script lang="ts" setup>
import { useSettingsStore } from '../stores/Settings'
import { toRef, ref, watch } from 'vue'
import { commonFonts } from '../stores/util';
import {useUpdateStore} from '../stores/update'

const setting = useSettingsStore()
const logFontSize = toRef(setting, 'logFontSize')
const defaultEncoding = toRef(setting, 'defaultEncoding')
const lineSpacing = toRef(setting, 'lineSpacing')
const logFontFamily = toRef(setting, 'logFontFamily')
const {currentVersion} = useUpdateStore()

const visible = ref(false)

const fontList = commonFonts[window.host.platform] || ['monospace']

interface SettingsBackup {
  logFontSize: number;
  defaultEncoding: string;
  lineSpacing: number;
  logFontFamily: string;
}

let settingsBackup: SettingsBackup

watch(visible, () => {
  if (visible.value) {
    settingsBackup = {
      logFontSize: setting.logFontSize,
      defaultEncoding: setting.defaultEncoding,
      lineSpacing: setting.lineSpacing,
      logFontFamily: setting.logFontFamily
    }
  }
})

const onConfirm = () => {
  visible.value = false
  setting.save()
}

const onCancel = () => {
  setting.logFontSize = settingsBackup.logFontSize
  setting.defaultEncoding = settingsBackup.defaultEncoding
  setting.lineSpacing = settingsBackup.lineSpacing
  setting.logFontFamily = settingsBackup.logFontFamily
  visible.value = false
}

const show = () => {
  visible.value = true
}

defineExpose({
  show
})
</script>
