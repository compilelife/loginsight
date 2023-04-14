<template>
<ContextMenu v-model:show="show" :options="options">
  <ColorMenuItems :colors="colors" v-model="curColor"/>
</ContextMenu>
</template>

<script lang="ts" setup>

import { ContextMenu, ContextMenuItem } from '@imengyu/vue3-context-menu';
import { reactive, Ref, ref } from 'vue';
import ColorMenuItems from './ColorMenuItems.vue';

const prop = defineProps<{
  colors: string[]
}>()

const show = ref(false)
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