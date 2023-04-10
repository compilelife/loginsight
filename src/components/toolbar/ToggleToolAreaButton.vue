<template>
  <ToolAreaButton :tooltip="prop.tooltip"
    @trigger="$emit('update:modelValue', !prop.modelValue)">
    <span :class='{toggled: prop.modelValue}'><slot /></span>
  </ToolAreaButton>
</template>

<script setup lang="ts">
import ToolAreaButton from './ToolAreaButton.vue'
const prop = defineProps<{
  tooltip: string,
  modelValue: boolean,
}>()
defineEmits<{
  (e: 'update:modelValue', id: boolean): void
}>()
</script>

<style>
.toggled {
  color: #409EFF;
}
</style>