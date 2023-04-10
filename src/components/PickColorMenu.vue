<template>
<ContextMenu v-model:show="show" :options="options">
  <ContextMenuItem 
    v-for="color in prop.colors"
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
</ContextMenu>
</template>

<script lang="ts" setup>

import { ContextMenu, ContextMenuItem } from '@imengyu/vue3-context-menu';
import { reactive, Ref, ref } from 'vue';

const prop = defineProps<{
  colors: string[]
}>()

const show = ref(false)
const colorInputRef = ref<HTMLElement>()
let curColor = ref('red')
const options = reactive({
	theme: 'flat',
	x: 0,
	y: 0,
})

function popup(x: number, y: number, colorRef: Ref<string>) {
	show.value = true
	options.x = x
	options.y = y
  curColor = colorRef
}

defineExpose({ popup })

</script>

<style>

.colorItem .label{
  color: white;
}
.colorItem:hover .label{
  color: black;
}
.moreColor{
  font-size: 12px;
}
.moreColor input {
  border: 0;
}
.moreColor .mx-context-menu-item{
  padding: 0 4px !important;
}
</style>