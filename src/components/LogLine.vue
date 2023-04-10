<template>

<li v-if="line && line.index >= 0"
	:class="{curline: $props.focused}"
	@click="$emit('requestFocus', line.index)"
	@click.right="$emit('requestFocus', line.index)">
	<div class="num" :class="numStyle">{{ lineNum }}</div>
	<pre ref="textNode" class="logText"> {{ line.content }}</pre>
</li>

</template>

<script setup lang="ts">

import {computed, inject, ref, toRef, watch, onMounted, toRaw} from 'vue'
import Mark from "mark.js"
import { LogTabData, SyntaxField } from "../stores/LogTabData"
import { useDebounceFn } from '@vueuse/shared';
import {useSettingsStore} from '../stores/Settings'
import { storeToRefs } from 'pinia';

const props = defineProps<{
	line: Line,
	active: boolean,
	focused: boolean,
	numWidth: number,
	searchResult: SearchResult|null,
	syntaxFields: SyntaxField[]
}>()

defineEmits<{
	(e: 'requestFocus', index: number): void
}>()

const line = computed(()=>props.line || {index: -1, content: ''})
const lineNum = computed(()=>line.value.index + 1)
const numWidth = computed(()=>`${props.numWidth}px`)
const settings = useSettingsStore()
const {lineSpacing, cssLogFont} = storeToRefs(settings)

const numStyle = computed(()=>({
	numActive: props.active,
	numFocused: props.focused
}))

const textNode = ref<HTMLElement>()

let tab = inject<LogTabData>('tab')!
if ((tab as any).value) {
	tab = (tab as any).value
}
async function _makeHighlight() {
	if (!textNode.value)
		return

	const highlighter = new Mark(textNode.value!)
	highlighter.unmark()
	
	//单词高亮
	const colors = new Map()
	tab.highlights.forEach(highlight => {
		colors.set(highlight.keyword, highlight.color)
	});
	highlighter.mark(Array.from(colors.keys()), {
		each: (node: HTMLElement)=>{
			node.style.backgroundColor = colors.get(node.innerText)
			node.style.color = 'white'
			node.style.padding = '2px'
		}
	})

	//搜索高亮
	if (props.searchResult) {
		const region = await tab.backend.mapOffsetFromLog(line.value.content, 
				[{offset: props.searchResult.offset, length: props.searchResult.len}])
		const ranges = [
			{
				start: region[0].offset,
				length: region[0].length
			}
		]
		highlighter.markRanges(ranges, {
			each: (node: HTMLElement)=>{
				node.style.backgroundColor = '#f00'
				node.style.padding = '2px'
			}
		})
	}

	//语法高亮
	if (line.value.segs.length === 0)
		return

	const syntaxRegions = await tab.backend.mapOffsetFromLog(
		toRaw(line.value.content),
		toRaw(line.value.segs)
	)
	interface SyntaxRange extends Mark.Range, SyntaxField{
	}
	const syntaxRanges = syntaxRegions.map((r,i)=>{
		const field = props.syntaxFields[i] ?? {color: '#f00', name: 'undef'}
		return {
			start: r.offset, 
			length: r.length,
			color: field.color,
			name: field.name
		}	
	})
	highlighter.markRanges(syntaxRanges, {
		each: (node: HTMLElement, range: SyntaxRange)=>{
			node.style.backgroundColor = ''
			node.style.color = range.color
		}
	})
}

watch([
	tab.highlights,
	()=>props.searchResult,
	()=>props.syntaxFields,
	()=>props.line
 ], 
 useDebounceFn(_makeHighlight, 100),
 {deep: true, flush: 'post'})

onMounted(_makeHighlight)

</script>

<style scoped>

li {
	display: flex;
	flex-direction: row;
	line-height: v-bind(lineSpacing);
}

.curline{
	background-color:   #ecf5ff;
}

.num {
	background-color:  #C0C4CC;
	color: white;
	width: v-bind(numWidth);
	user-select: none;
	text-align: center;
	flex-shrink: 0;
	font: v-bind(cssLogFont);
}
.numActive {
	background-color: #337ecc;
}

.numFocused {
	background-color:  #79bbff;
}

.logText{
	padding-left: 0.4em;
	white-space: pre-wrap;
	word-break: break-all;
	margin: 0;
	font: v-bind(cssLogFont);
}

</style>