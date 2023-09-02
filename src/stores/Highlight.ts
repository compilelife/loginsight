import { ref } from 'vue';
import { createStoreInstance } from './tabsStore';


export type Highlight = ReturnType<typeof newHighlight>;

export function newHighlight(keywordV: string, colorV: string) {
	return createStoreInstance('highlight', () => {
		const keyword = ref(keywordV);
		const color = ref(colorV);
		return {
			keyword,
			color,
			save: ()=>({keyword: keyword.value, color:color.value})
		};
	});
}
