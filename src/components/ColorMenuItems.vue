<template>
  <ContextMenuItem 
    v-for="color in props.colors"
    :style="{'background-color': color}"
    class="colorItem"
    @click="curColor=color"
    :label="color">
  </ContextMenuItem>
  <ContextMenuItem 
    class="moreColor" 
    :click-close="false"
    @click="colorInputRef?.click()">
    更多颜色 <input ref="colorInputRef" v-model="curColor" type="color" >
  </ContextMenuItem>
</template>


<script lang="ts" setup>
import { ContextMenuItem } from '@imengyu/vue3-context-menu';
import { computed, ref } from 'vue';

const props = defineProps<{
  colors: string[]
  modelValue: string,
}>()

const emit = defineEmits<{
  (e: 'update:modelValue', value: string): void
}>()

const curColor = computed({
  get() {
    return props.modelValue
  },
  set(value) {
    emit('update:modelValue', value)
  }
})

const colorInputRef = ref<HTMLElement>()
</script>