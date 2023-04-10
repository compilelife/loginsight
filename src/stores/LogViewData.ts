import { nextTick, reactive, ref, watch } from 'vue';
import { computed } from 'vue';
import { inRange, isValidRange, rangeCount, updateRange } from '../ipc/platform';
import { ElMessage } from 'element-plus';
import { createStoreInstance } from './tabsStore';


export type LogViewData = ReturnType<typeof newLogViewData>;

export function newLogViewData(backend: IBackend, titleV: string, log: OpenLogResult) {
	return createStoreInstance('view', () => {
		const logId = log.logId;
		const openAction = ref<FilterArg>()
		const range = reactive(log.range);
		const lineCount = computed(() => rangeCount(range));
		const curLineIndex = ref(0);
		const focusLineIndex = ref(0);
		const displayCount = ref(20);
		const caching = ref(false);
		let cache: Line[] = [];
		const title = ref(titleV);
		const cacheRange = { begin: 0, end: -1 };
		const selectedWord = ref('');
		const visibleLastLineIndex = ref(0)
		const forceRefreshing = ref(false);
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
				begin: Math.max(range.begin, index - 100),
				end: Math.min(range.end, index + 100)
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

			addNavHistory,
			goBack,
			goForward,
			preload,
			jumpTo,
			getLine,
			getLines,
			updateLogRange,
			setCurrentLine,
			forceRefreshCache
		};
	});
}
