
import { ElTooltip } from 'element-plus';

<template>

<ElTooltip
  effect="light"
  :show-after="1000"
  :content="tooltip">
  <ElButton text class='toolAreaBtn' @click="$emit('trigger')" :disabled="disabled">
    <el-icon size="18">
      <slot/>
    </el-icon>
  </ElButton>
</ElTooltip>

</template>

<script lang="ts" setup>

const {tooltip} = withDefaults(defineProps<{
  tooltip: string,
  disabled?: boolean,
}>(), {
  disabled: false,
});

defineEmits<{
  (e:'trigger'):void
}>()

</script>

<style>

.toolAreaBtn.el-button {
  padding: 3px 0; 
  height: auto;
}

</style>