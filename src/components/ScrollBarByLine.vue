<template>

<ElScrollbar native always 
    ref="root" class="log-scrollbar" 
    @mouseup="userReleaseMouse" 
    @mousedown="userPressMouse"
    @wheel.prevent=""
    @scroll="handleScroll">
  <div class="content"></div>
</ElScrollbar>

</template>

<script setup lang="ts">

import { useDebounceFn, useElementSize, useScroll, useThrottleFn, useTimeoutFn } from '@vueuse/core';
import { ElScrollbar } from 'element-plus';
import { computed, ref, watch } from 'vue';

const props = defineProps<{
    count: number,
    modelValue: number,
    preload: (index:number) => Promise<void>
}>()
const emit = defineEmits<{
    (e: 'update:modelValue', index:number): void
    (e: 'focused'): void
}>()

const root = ref<InstanceType<typeof ElScrollbar>>()
const {height} = useElementSize(()=>root.value?.wrapRef)
const ratio = computed(()=>{
    const logValue = Math.floor(Math.log10(props.count)*2) // <10000 => 6...
    return Math.min(Math.max(2, logValue), 20) //限制范围
})
const contentHeight = computed(()=>`${ratio.value}00%`)
const maxScrollHeight = computed(()=>(ratio.value - 1) * height.value)

const currentIndex = computed({
    get() {
        return props.modelValue
    },
    set(value) {
        emit('update:modelValue', value)
    }
})

let userScroll = false
const mapIndexToOffset = (index: number)=>index / (props.count - 1) * maxScrollHeight.value
const mapOffsetToIndex = (offset: number)=>Math.floor(offset / maxScrollHeight.value * (props.count - 1))

function userPressMouse() {
    userScroll=true
}
function userReleaseMouse() {
    setTimeout(() => {
        userScroll=false
    }, 100);
}

const handleScroll = useThrottleFn(_handleScroll, 50)

async function _handleScroll(ev: { scrollLeft: number, scrollTop: number }) {
    console.log(ev.scrollTop, userScroll)
    if (userScroll) {
        const targetIndex = mapOffsetToIndex(ev.scrollTop)
        if (Number.isNaN(targetIndex))
            return
        console.log('preload', targetIndex, ev.scrollTop)
        await props.preload(targetIndex)
        console.log('update to', targetIndex)
        emit('update:modelValue', targetIndex)
        emit('focused')
    }
}

watch([props], ()=>{
    if (!userScroll) {
        root.value?.setScrollTop(mapIndexToOffset(currentIndex.value))
    }
})

</script>

<style scoped>

.content {
  width: 20px;
}

.content {
  height: v-bind(contentHeight);
}

</style>

<style>
.log-scrollbar .el-scrollbar__view {
    height: 100%;
}
</style>