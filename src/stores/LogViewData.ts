import { nextTick, reactive, ref, watch } from 'vue';
import { computed } from 'vue';
import { inRange, isValidRange, rangeCount, updateRange } from '../ipc/platform';
import { ElMessage } from 'element-plus';
import { createStoreInstance } from './tabsStore';
import { storeToRefs } from 'pinia';
import { useSettingsStore } from './Settings';


export type LogViewData = ReturnType<typeof newLogViewData>;

export function newLogViewData(backend: IBackend, titleV: string, log: OpenLogResult) {
	return createStoreInstance('view', () => {
		const {logFontSize, lineSpacing} = storeToRefs(useSettingsStore())
		
		const logId = log.logId;
		const openAction = ref<FilterArg>()
		const range = reactive(log.range);
		const lineCount = computed(() => rangeCount(range));
		const curLineIndex = ref(0);
		const focusLineIndex = ref(0);
		const caching = ref(false);
		let cache: Line[] = [];
		const title = ref(titleV);
		const cacheRange = { begin: 0, end: -1 };
		const selectedWord = ref('');
		const visibleLastLineIndex = ref(0)
		const forceRefreshing = ref(false);
		const calculateLineHeight = ref<(s:string)=>number>()
		const lineHeight = computed(()=>logFontSize.value * lineSpacing.value)
		const viewPortHeight = ref(0)
		const displayCount = computed(()=>Math.round(viewPortHeight.value/lineHeight.value)+1)
		const cacheLen = computed(()=>Math.max(100, displayCount.value))
		//上次搜索结果位置(原始字节)
		const lastSearchResult = ref<SearchResult|null>(null)
		//当continueSearch为true时，从lastSearchResult的位置继续搜索
		//当用户点击到另一行，则连续搜索失效；
		//当有新的搜索结果，说明正在执行搜索动作，就设置为连续搜索
		const continueSearch = ref(false)
		watch(focusLineIndex, ()=>continueSearch.value = focusLineIndex.value === lastSearchResult.value?.index && lastSearchResult.value.found)
		watch(lastSearchResult, ()=>continueSearch.value = lastSearchResult.value?.found === true)
		
		const navHistory = reactive<Array<number>>([])
		const curNavPos = ref(-1)

		function addNavHistory(index: number) {
			navHistory.splice(curNavPos.value+1, navHistory.length, index)
			curNavPos.value = navHistory.length - 1
		}

		function goBack() {
			const backPos = navHistory[curNavPos.value - 1]
			if (backPos) {
				curNavPos.value -= 1
				jumpTo(backPos)
			}
		}
		
		function goForward() {
			const forwardPos = navHistory[curNavPos.value + 1]
			if (forwardPos) {
				curNavPos.value += 1
				jumpTo(forwardPos)
			}
		}

		const canGoBack = computed(()=>curNavPos.value - 1 >= 0)
		const canGoForward = computed(()=>curNavPos.value < navHistory.length - 1)

		function checkCacheEnough(index: number): boolean {
			const viewPortMaxIndex = Math.min(index + displayCount.value, range.end);
			return inRange(cacheRange, index) && inRange(cacheRange, viewPortMaxIndex);
		}

		async function preload(index: number, force = false) {
			if (checkCacheEnough(index) && !force) {
				return;
			}

			caching.value = true;

			const loadRange = {
				begin: Math.max(range.begin, index - cacheLen.value),
				end: Math.min(range.end, index + cacheLen.value)
			};
			if (!isValidRange(loadRange)) {
				ElMessage.warning('预加载的位置非法，可能日志内容为空');
				return;
			}

			try {
				console.log('loading', loadRange);
				cache = (await backend.getLines({ range: loadRange, logId })).lines;
			} catch (e) {
				ElMessage({
					message: `向日志引擎请求日志行失败：${loadRange.begin}-${loadRange.end}`,
					type: 'warning'
				});
				cache = [];
				for (let i = loadRange.begin; i < loadRange.end; i++) {
					cache.push({ index: i, line: -1, content: 'fake line', segs: [] });
				}
			}

			cacheRange.begin = loadRange.begin;
			cacheRange.end = loadRange.end;

			caching.value = false;
		}

		async function forceRefreshCache() {
			await preload(curLineIndex.value, true)
			forceRefreshing.value = true;
			nextTick(()=>forceRefreshing.value = false)
		}

		async function jumpTo(index: number, recordToHistory: boolean = false) {
			await preload(index);
			curLineIndex.value = index;
			focusLineIndex.value = index;
			if (recordToHistory) {
				addNavHistory(index)
			}
		}

		function getLine(index: number) {
			return cache[index - cacheRange.begin];
		}

		function getLines(from: number, to: number) {
			from -= cacheRange.begin
			to -= cacheRange.begin
			return cache.slice(from, to)
		}

		function updateLogRange(newRange: LogRange) {
			return updateRange(range, newRange);
		}

		function setCurrentLine(index: number) {
			focusLineIndex.value = index;
		}

		async function pageDown() {
			jumpTo(visibleLastLineIndex.value)
		}

		async function bottomTo(index: number) {
			const minLineIndex = Math.max(index - displayCount.value, range.begin)
			if (!inRange(cacheRange, minLineIndex) || !inRange(cacheRange, index)) {
				await preload(minLineIndex, true)
			}

			let height = 0
			let targetIndex = minLineIndex
			// console.log(range.begin, range.end, index, minLineIndex, cacheRange.begin, cacheRange.end)
			for (let i = index; i >= minLineIndex; i--) {
				if (calculateLineHeight.value) {
					height += calculateLineHeight.value(getLine(i).content)
					// console.log(viewPortHeight.value, height, i)
					if (height > viewPortHeight.value) {
						targetIndex = Math.min(index, i+1)
						break
					}
				}
			}

			await jumpTo(targetIndex)
			setCurrentLine(index)
			return targetIndex
		}

		async function goToBottom() {
			return bottomTo(range.end)
		}

		async function pageUp() {
			bottomTo(curLineIndex.value)
		}

		return {
			logId,
			range,
			curLineIndex,
			focusLineIndex,
			caching,
			displayCount,
			lineCount,
			title,
			continueSearch,
			lastSearchResult,
			canGoBack,
			canGoForward,
			openAction,
			selectedWord,
			visibleLastLineIndex,
			forceRefreshing,
			calculateLineHeight,
			lineHeight,
			viewPortHeight,

			addNavHistory,
			goBack,
			goForward,
			preload,
			jumpTo,
			getLine,
			getLines,
			updateLogRange,
			setCurrentLine,
			forceRefreshCache,
			pageDown,
			pageUp,
			goToBottom
		};
	});
}
