
<template>
	<ContextMenu v-model:show="show" :options="options">
		<ContextMenuItem label="追踪该行" @click="trackLine" />
		<ContextMenuItem label="添加到时间线" @click="tab.addFocusedLineToTimeLine"/>
		<template v-if="hasSelectedWord">
			<ContextMenuItem label="高亮" @click="addHighlight" />
			<ContextMenuItem label="复制" @click="copy"/>
			<ContextMenuItem label="过滤" @click="filter(false)" />
			<ContextMenuItem label="反过滤" @click="filter(true)" />
		</template>
	</ContextMenu>
</template>

<script setup lang="ts">

import { ContextMenuItem } from '@imengyu/vue3-context-menu';
import { ref, reactive, computed, inject } from 'vue';
import { newHighlight } from "../stores/Highlight"
import { LogViewData } from "../stores/LogViewData"
import { LogTabData } from "../stores/LogTabData"
import { maybeLongOperation, nextRandHighlightColor } from '../stores/util';
import {platform} from '../ipc/platform'

const { view } = defineProps<{
	view: LogViewData
}>()

const tab = inject<LogTabData>('tab')!

const show = ref(false)
const options = reactive({
	theme: 'flat',
	x: 0,
	y: 0,
})
const selectedWord = ref('')
const hasSelectedWord = computed(() => selectedWord.value.length > 0)

function popup(word: string, x: number, y: number) {
	show.value = true
	options.x = x
	options.y = y
	selectedWord.value = word
}

function trackLine() {
	tab.trackLine(view.focusLineIndex, view)
}

function addHighlight() {
	const color = nextRandHighlightColor()
	tab.addHighlight(newHighlight(selectedWord.value, color))
}

function copy() {
	platform.saveTextToClipboard(selectedWord.value)
}

function filter(reverse: boolean) {
	tab.filter({
		caseSense: true,
		regex: false,
		pattern: selectedWord.value,
		reverse,
		logId: tab.activeLogView.logId,
	})
}

defineExpose({ popup })

</script>