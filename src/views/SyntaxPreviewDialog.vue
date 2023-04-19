<template>
  <ElDialog
    v-model="visible"
    width="90%"
    title="设置语法">
    <!-- 输入区 -->
    <div class="row1">
      <ElInput 
        v-model="pattern"
        placeholder="输入正则表达式，使用捕获创建字段"/>
      <ElButton type="primary" @click="onPreview">预览</ElButton>
      <ElPopover :visible="visbilityOfSyntaxName">
        <ElInput size="small" placeholder="输入规则名称" v-model="syntaxName" style="margin-bottom: 10px;"/>
        <ElButton type="primary" size="small" @click="onSave">确定</ElButton>
        <ElButton size="small" @click="visbilityOfSyntaxName = false">取消</ElButton>
        <template #reference>
          <ElButton @click="visbilityOfSyntaxName = true">保存</ElButton>
        </template>
      </ElPopover>
      <ElButton @click="onManage">管理</ElButton>
    </div>
    <div class="row2">
      <!-- 字段管理 -->
      <div class="fieldArea">
        <ElTable
          :data="fields"
          stripe border
          empty-text="添加字段: 指定字段颜色"
          style="flex-grow: 1;"
          >
          <ElTableColumn label="字段名" width="120">
            <template #default="scope">
              <ElInput v-model="scope.row.name"/>
            </template>
          </ElTableColumn>
          <ElTableColumn label="颜色" width="100">
            <template #default="scope">
              <div 
                :style="{width:'12px',height:'12px','backgroundColor':scope.row.color}"
                @click="ev=>showPickColorMenu(ev, scope.$index)"/>
            </template>
          </ElTableColumn>
          <ElTableColumn label="操作">
            <template #default="scope">
              <el-button
                link
                type="primary"
                size="small"
                @click.prevent="delField(scope.$index)"
              >
                删除
              </el-button>
            </template>
          </ElTableColumn>
        </ElTable>
        <ElButton style="width: 100%" @click="addField" :icon="AddOne" type="primary">添加字段</ElButton>
      </div>


      <!-- 日志预览 -->
      <ElScrollbar class="previewLogView" ref="refLogPreview">
        <LogLine v-for="(line,index) in lines"
          :line="line"
          :key="tab?.name+'-'+line.line"
          :active="true"
          :syntax-fields="fields"
          :focused="index === focusedIndex"
          @request-focus="focusedIndex = index"
          :search-result="null"
          :num-width="numWidth">
        </LogLine>
      </ElScrollbar>
    </div>
    <!-- 对话框按钮 -->
    <template #footer>
      <span class="dialog-footer">
        <el-button type="primary" @click="onConfirm">确定</el-button>
        <el-button @click="onCancel">取消</el-button>
      </span>
    </template>
  </ElDialog>
<PickColorMenu 
  ref="pickColorMenu"
  :colors="safeColorForWhiteBg" />
</template>

<script setup lang="ts">
import { ElButton, ElDialog, ElInput, ElTableColumn, ElTable, ElConfigProvider, ElMessage, ElScrollbar, ElPopover } from 'element-plus';
import { ref,provide, toRef, reactive, computed, toRaw } from 'vue';
import {LogTabData, Syntax, SyntaxField} from '../stores/LogTabData'
import { measureTextWidth, nextRandTextColor, safeColorForWhiteBg } from '../stores/util';
import LogLine from '../components/LogLine.vue';
import {LineSegType} from '../ipc/platform'
import {AddOne} from '@icon-park/vue-next'
import PickColorMenu from '../components/PickColorMenu.vue';
import { storeToRefs } from 'pinia';
import { useSettingsStore } from '../stores/Settings';
import { useCollectStore } from '../stores/collect';
import { useDialogStore } from '../stores/dialogs';
import { useSyntaxStore } from '../stores/syntax';
import { table } from 'console';

const lines = reactive<Array<Line>>([])

const visible = ref(false)
const tab = ref<LogTabData>()
provide('tab', tab)

const {collect} = useCollectStore()

const pattern = ref('')
const fields = reactive<SyntaxField[]>([])

const visbilityOfSyntaxName = ref(false)
const syntaxName = ref('')

const refLogPreview = ref<InstanceType<typeof ElScrollbar>|null>(null)

const pickColorMenu = ref<InstanceType<typeof PickColorMenu>>()

const focusedIndex = ref(0)
const {logFontFamily, logFontSize} = storeToRefs(useSettingsStore())
const numWidth = computed(()=>measureTextWidth(
    `${lines[lines.length - 1].line}`, 
    `${logFontSize.value}px ${logFontFamily.value}`)+16)

function show(curTab: LogTabData, previewLines: Array<Line>) {
  tab.value = curTab

  lines.splice(0, lines.length)
  lines.push(...previewLines)
  visible.value = true
  fillData(tab.value!.curSyntax)
}

function fillData(syntax: Syntax) {
  pattern.value = syntax.pattern
  fields.splice(0, fields.length, ...syntax.fields)
}

function delField(index: number) {
  fields.splice(index, 1)
}

function addField() {
  fields.push({
    name: `field-${fields.length+1}`,
    color: nextRandTextColor()
  })
}

async function onConfirm() {
  if (pattern.value.length === 0) {
    ElMessage.error('正则表达式不能为空')
    return
  }
  
  visible.value = false

  tab.value?.setSyntax({pattern: pattern.value, fields})
}

function onCancel() {
  visible.value = false
}

async function onPreview() {
  const {segs} = await tab.value!.backend.testSyntax({
    pattern: pattern.value,
    caseSense: true,
    lines: lines.map(l=>l.content),
  })

  for (let i = 0; i < segs.length; i++) {
    const seg = segs[i];
    lines[i].segs = seg
  }
}

function onSave() {
  if (syntaxName.value.length === 0) {
    ElMessage.warning('名称不能为空')
    return
  }

  useSyntaxStore().addItem({
    name: syntaxName.value,
    pattern: pattern.value, 
    fields,
    previewLog: refLogPreview.value?.$el.getElementsByTagName('pre')[0].innerHTML ?? ''
  })
  visbilityOfSyntaxName.value = false
  ElMessage.success('保存成功')
}

async function onManage() {
  const selectedSyntax = await useDialogStore().showSyntaxManagerDlg()
  if (selectedSyntax) {
    fillData(selectedSyntax)
    onPreview()
  } 
}

function showPickColorMenu(ev: MouseEvent, fieldIndex: number) {
  pickColorMenu.value?.popup(ev.pageX, ev.pageY, toRef(fields[fieldIndex], 'color'))
}

defineExpose({
  show
})

</script>

<style scoped>

.fieldArea {
  display: flex;
  flex-direction: column;
  height: 100%;
}
.row1 {
  display: flex;
  width: 100%;
}
.row1 button {
  flex-grow: 0;
  margin-left: 10px;
}
.row2 {
  display: flex;
  width: 100%;
  height: 40vh;
  margin-top: 20px;
}
.previewLogView {
  height: 100%;
  flex-grow: 1;
  margin-left: 20px;
  border: 1px solid #dcdfe6;
}
</style>