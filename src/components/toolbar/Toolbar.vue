<template>
  <div class="toolbar">
    <FileArea />
    <Divider />
    <StateArea />
    <Divider />
    <SearchArea />
    <Divider />
    <FilterArea />
    <Divider />
    <NavigateArea />
    <Divider />
    <HighlightManageArea />
    <Divider />
    <TimeLineArea />
    <Divider />
    <ToolButton label="设置" @click="showSettingsDlg">
      <Setting />
    </ToolButton>
  </div>
</template>


<script lang="ts" setup>
import { Setting } from '@icon-park/vue-next';
import HighlightManageArea from './HighlightManageArea.vue';
import ToolButton from './ToolButton.vue';
import Divider from './Divider.vue';
import FileArea from './FileArea.vue';
import FilterArea from './FilterArea.vue';
import SearchArea from './SearchArea.vue';
import StateArea from './StateArea.vue';
import NavigateArea from './NavigateArea.vue';
import TimeLineArea from './TimeLineArea.vue';
import { useDialogStore } from '../../stores/dialogs';

const { showSettingsDlg } = useDialogStore()

</script>

<style >
.toolbar {
  width: 100%;
  z-index: 1;
  box-shadow: 0 2px 4px -2px grey;
  /* 可用高度48px */
  padding: 6px 10px 4px 10px;
  /* height: 58px; */
  box-sizing: border-box;

  display: flex;
}

.toolbar .el-button+.el-button {
  margin-left: 6px;
}
</style>