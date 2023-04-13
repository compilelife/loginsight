
<template>
  <span v-if="!isEditing" @dblclick="startEditing">{{ text }}</span>
  <el-input v-else v-model="text" @blur="stopEditing" @keydown.enter="stopEditing" ref="input" style="width: 100px"></el-input>
</template>

<script lang="ts" setup>
import { ref, computed, nextTick } from 'vue';
import { ElInput } from 'element-plus';

const props = defineProps<{
  modelValue: string,
}>()

const emit = defineEmits<{
  (e: 'update:modelValue', value: string): void
}>()

const text = computed({
  get() {
    return props.modelValue
  },
  set(value) {
    emit('update:modelValue', value)
  }
})

const isEditing = ref(false);
const input = ref<InstanceType<typeof ElInput>>()

const startEditing = () => {
  isEditing.value = true;
  nextTick(() => {
    input.value?.focus();
    input.value?.select();
  });
};

const stopEditing = () => {
  isEditing.value = false;
};
</script>