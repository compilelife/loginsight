import { reactive, ref } from 'vue';
import { computed } from 'vue';
import { createStoreInstance } from './tabsStore';
import { ElMessageBox } from 'element-plus';



export type TimeLineNodeData = ReturnType<typeof newTimeLineNodeData>;

export function newTimeLineNodeData(lineV: number, textV: string) {
	return createStoreInstance('timenode', () => {
		const text = ref(textV.trim());
		const comment = ref('');
		const line = ref(lineV);
		const color = ref('#337ecc');
		const key = computed(() => `${lineV + 1}`);

		return { line, color, comment, text, key ,
		save: ()=>({
			line: line.value,
			color:color.value,
			comment: comment.value,
			text: text.value,
			key: key.value
		})};
	});
}

export type TimeLineData = ReturnType<typeof newTimeLineData>;

export function newTimeLineData() {
	return createStoreInstance('timeline', () => {
		const nodes = reactive<Array<TimeLineNodeData>>([]);
		const requiredNode = ref<TimeLineNodeData | null>(null);
		const htmlRef = ref<{$el: HTMLElement}>()
		const curColor = ref('#337ecc');

		function addNode(node: TimeLineNodeData) {
			node.color = curColor.value
			nodes.push(node);
			nodes.sort((a, b) => a.line - b.line);
		}

		function rmNode(node: TimeLineNodeData) {
			const index = nodes.findIndex(n => n.line === node.line);
			nodes.splice(index, 1);
		}

		function trackLine(line: number) {
			const target = nodes.find(n => n.line === line);
			if (target)
				requiredNode.value = target;
		}

		function clear() {
			ElMessageBox.confirm(
				`是否要清空日志线，该操作无法撤销`,
				'Loginsight',
				{
					confirmButtonText: '清空',
					cancelButtonText: '取消',
					type: 'warning',
				}
			)
				.then(() => {
					nodes.splice(0, nodes.length);
				})
				.catch(() => { })
		}

		return {
			nodes,
			requiredNode,
			htmlRef,
			curColor,

			addNode, 
			rmNode, 
			trackLine,
			clear
		};
	});
}
