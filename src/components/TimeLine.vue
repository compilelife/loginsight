<template>
	<ElScrollbar always class="timeline">
		<el-timeline ref="htmlRef">
			<el-timeline-item 
				v-for="item in items" 
				:timestamp="item.key" 
				:color="item.color" 
				:key="item.key"
				:ref="(el:any)=>setItemRef(el, item.key)"
				@click.right="(ev: MouseEvent)=>showNodeMenu(ev, item)"
				placement="top"
				style="margin-left: 2px;">
				<el-card shadow="hover" class="timenode" :style="{'border-color': item.color}">
					<ElScrollbar max-height="5em" class="logtextWrapper">
						<pre class="logtext">{{ item.text }}</pre>
					</ElScrollbar>
					<el-input v-model="item.comment" />
				</el-card>
			</el-timeline-item>
		</el-timeline>
	</ElScrollbar>

	<TimeLineMenu ref="menu" :colors="safeBgColorForWhiteText"/>
</template>

<script setup lang="ts">

import { inject, ref, watch, toRef, Ref } from 'vue';
import { TimeLineNodeData } from "../stores/TimeLineData"
import { LogTabData } from "../stores/LogTabData"
import TimeLineMenu from './TimeLineMenu.vue';
import {safeBgColorForWhiteText} from '../stores/util'
import { ElScrollbar, ElTimelineItem } from 'element-plus';
import { storeToRefs } from 'pinia';
import { useSettingsStore } from '../stores/Settings';

const data = inject<LogTabData>('tab')!.timeline
const htmlRef = toRef(data, 'htmlRef')
const items = data.nodes
const menu = ref<InstanceType<typeof TimeLineMenu>>()
const {cssLogFont} = storeToRefs(useSettingsStore())

type TimeLineItem = InstanceType<typeof ElTimelineItem>
const itemRefs = new Map<string, TimeLineItem>()

function showNodeMenu(ev: MouseEvent, item: TimeLineNodeData) {
	menu.value?.popup(ev.pageX, ev.pageY, item)
}

function setItemRef(el: TimeLineItem | null, key: string) {
	if (el)
		itemRefs.set(key, el)
	else
		itemRefs.delete(key)
}

watch(toRef(data, 'requiredNode'), ()=>{
	if (data.requiredNode) {
		const item = itemRefs.get(data.requiredNode.key)
		item?.$el.scrollIntoView()
	}
})

</script>

<style scoped>
.timeline {
	height: 100%;
	padding: 50px 20px 10px 20px;
	box-sizing: border-box;
}

.timeline ul {
	padding-inline-start: 0;
}

.timeline .el-card{
	--el-card-padding: 0 10px 5px 10px;
}

.footer {
	height: 50px;
}

.logtextWrapper {
	margin: 0 0 10px 0;
}

.logtext {
	white-space: pre-wrap;
	word-break: break-all;
	font: v-bind(cssLogFont);
}
</style>