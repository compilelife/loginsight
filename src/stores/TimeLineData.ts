import { reactive, ref } from 'vue';
import { computed } from 'vue';
import { createStoreInstance } from './tabsStore';



export type TimeLineNodeData = ReturnType<typeof newTimeLineNodeData>;

export function newTimeLineNodeData(lineV: number, textV: string) {
	return createStoreInstance('timenode', () => {
		const text = ref(textV.trim());
		const comment = ref('');
		const line = ref(lineV);
		const color = ref('#337ecc');
		const key = computed(() => `${lineV + 1}`);

		return { line, color, comment, text, key };
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
			nodes.splice(0, nodes.length);
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
