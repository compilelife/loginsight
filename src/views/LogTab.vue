<template>
<div class="root">
  <Toolbar></Toolbar>
  <Splitpanes class="default-theme" style="overflow: hidden;">
    <Pane :size="100 - timeLineWidth">
      <Splitpanes horizontal class="default-theme">
        <Pane :size="100 - subHeight">
          <LogView :data="rootLogView"/>
        </Pane>
        <Pane max-size="80" :size="subHeight">
          <el-tabs type="border-card" style="height: 100%;" v-model="data.activeSubIndex">
            <el-tab-pane
              class="tabContent"
              v-for="(sub, index) of subLogViews"
              :name="index"
              :key="sub.logId">
              <template #label>
                <span>
                  <span>{{sub.title}}</span>
                  <el-icon
                    class="tabCloseBtn"
                    @click.stop="rmSubView(sub.logId)">
                    <Close />
                  </el-icon>
                </span>
              </template>
              <LogView :data="sub" />
            </el-tab-pane>
          </el-tabs>
        </Pane>
      </Splitpanes>
    </Pane>
    <Pane max-size="50" :size="timeLineWidth">
      <TimeLine />
    </Pane>
  </Splitpanes>
</div>
</template>
  
<script setup lang="ts">

import {Close} from '@icon-park/vue-next'
import { Pane, Splitpanes } from 'splitpanes';
import 'splitpanes/dist/splitpanes.css';
import { provide, computed } from 'vue';
import { LogTabData } from "../stores/LogTabData"
import LogView from '../components/LogView.vue';
import TimeLine from '../components/TimeLine.vue';
import Toolbar from '../components/toolbar/Toolbar.vue';

const {data} = defineProps<{
  data: LogTabData
}>()

const {rootLogView, subLogViews, rmSubView} = data
const subHeight = computed(()=>subLogViews.length === 0 ? 0 : 30)
const timeLineWidth = computed(()=>data.timeline.nodes.length === 0 ? 0 : 30)

provide('tab', data)

</script>
  
<style scoped>
.root {
  height: 100%;
  display: flex;
  flex-direction: column;
}
</style>
  