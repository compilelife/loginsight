import { computed, reactive, ref, watch, watchEffect } from 'vue';
import { inRange } from '../ipc/platform';
import { ElMessage } from 'element-plus';
import { createStoreInstance, TabType } from './tabsStore';
import { Highlight } from "./Highlight";
import { newTimeLineData, newTimeLineNodeData, TimeLineData } from "./TimeLineData";
import { newLogViewData, LogViewData } from "./LogViewData";
import { newSearchData } from './SearchData';
import { maybeLongOperation, nextTabId } from './util';
import { RecentItem } from './recents';
import { useSettingsStore } from './Settings';


export type LogTabData = ReturnType<typeof newLogTabData>;
let tabId = 0;

export interface SyntaxField {
  name: string;
  color: string;
}

export interface Syntax {
  pattern: string;
  fields: Array<SyntaxField>;
}


export function newLogTabData(nameV: string, backendV: IBackend, rootLog: OpenLogResult) {
  return createStoreInstance('tab', () => {
    const {defaultEncoding} = useSettingsStore()
    const name = nextTabId()
    const desc = ref('')
    const highlights = reactive<Array<Highlight>>([]);
    const timeline = reactive(newTimeLineData());
    const backend = backendV;
    const rootLogId = rootLog.logId;
    const rootLogView = newLogViewData(backend, nameV, rootLog);
    const activeSubIndex = ref(-1);
    const subLogViews = reactive<Array<LogViewData>>([]);
    const activeLogView = ref<LogViewData>(rootLogView);
    const followLog = ref(true);
    const logEncoding = ref(defaultEncoding);
    const searchData = newSearchData()
    const curSyntax = ref<Syntax>({ pattern: '', fields: [] })
    const title = computed(() => rootLogView.title)
    const type: TabType = 'log'
    const openAction = ref<RecentItem>()
    const toolActions = {
      beginSearch: ()=>{},
      beginFilter: ()=>{},
      beginJump: ()=>{},
    }

    backend.setLogEncoding(logEncoding.value)

    function addHighlight(highlight: Highlight) {
      if (highlights.find(h => h.keyword === highlight.keyword))
        return;
      highlights.push(highlight);
    }

    function rmHighlight(highlight: Highlight) {
      const index = highlights.findIndex(h => h.keyword === highlight.keyword);
      if (index >= 0) {
        highlights.splice(index, 1);
      }
    }

    function isActive(view: LogViewData) {
      return activeLogView.value === view || activeLogView.value.logId === view.logId;
    }

    function getViewByLogId(logId: number): LogViewData | undefined {
      if (rootLogId === logId) {
        return rootLogView;
      }

      return subLogViews.find(view => view.logId === logId);
    }

    function rmSubView(id: number) {
      const index = subLogViews.findIndex(view => view.logId === id);
      subLogViews.splice(index, 1);
      activeSubIndex.value = subLogViews.length - 1;
    }

    function addFocusedLineToTimeLine() {
      const curView = activeLogView.value;
      if (curView.focusLineIndex < 0) {
        ElMessage.warning('没有选择当前行');
        return;
      }

      const { line, content } = curView.getLine(curView.focusLineIndex);
      const node = newTimeLineNodeData(line, content);
      timeline.addNode(node);
    }

    async function handleRangeChanged() {
      const { ranges } = await backend.syncLogs();
      ranges.forEach(async item => {
        const view = getViewByLogId(item.logId);
        if (!view)
          return;
        const rangeChanged = view.updateLogRange(item.range);
        if (rangeChanged && followLog.value) {
          await view.goToBottom()
        }
      });
    }

    function handleDisconnect() {
      ElMessage({
        showClose: true,
        message: `${nameV}意外关闭，该标签页的后续操作将会无效`,
        type: 'error',
      });
    }

    function handleDie() {
      ElMessage({
        showClose: true,
        message: '日志引擎已崩溃，该标签页的后续操作将会无效',
        type: 'error',
      });
    }

    backend.setListener(({ cmd }) => {
      switch (cmd) {
        case 'rangeChanged': return handleRangeChanged();
        case 'disconnected': return handleDisconnect();
        case 'die': return handleDie();
        default: break;
      }
    });

    async function addSubViewByFilter(arg: FilterArg) {
      arg.logId = activeLogView.value.logId;
      let openRet
      try{
        openRet = await backend.filter(arg);
      }catch (e:any) {
        ElMessage.error(e)
        return
      }
      const subView = newLogViewData(backend, arg.pattern, openRet);
      subView.openAction = arg
      subLogViews.push(subView);
      activeSubIndex.value = subLogViews.length - 1;

      return subView
    }

    async function trackLine(index: number, target: TimeLineData | LogViewData | null = null) {
      const fromTimeline = target !== null && (target as any)['nodes'] instanceof Array;
      const view = fromTimeline ? rootLogView : (target === null ? rootLogView : target as LogViewData);

      if (!inRange(view.range, index)) {
        ElMessage.warning('要追踪的行不在有效范围内');
        return;
      }

      const { lines } = await backend.mapLine({ index, logId: view.logId });
      let lineIndexInRoot = index;
      lines.forEach(({ logId, index }) => {
        if (logId === rootLogView.logId) {
          lineIndexInRoot = index;
        }
        if (!fromTimeline && view.logId === logId) {
          return;
        }

        getViewByLogId(logId)?.jumpTo(index, true);
      });

      if (!fromTimeline)
        timeline.trackLine(lineIndexInRoot);
    }

    async function toggleLogEncoding() {
      if (logEncoding.value === "gbk")
        logEncoding.value = "utf-8";
      else
        logEncoding.value = "gbk";
      await backend.setLogEncoding(logEncoding.value)
      refresh()
    }

    async function search(forward: boolean) {
      const view = activeLogView.value
      const arg: SearchArg = {
        fromChar: 0,
        fromLine: view.focusLineIndex,
        logId: view.logId,
        reverse: !forward,
        caseSense: searchData.caseSense,
        regex: searchData.regex,
        pattern: searchData.pattern
      }
      //从光标处连续搜索
      if (view.continueSearch && view.lastSearchResult) {
        if (forward)
          arg.fromChar = view.lastSearchResult.offset + view.lastSearchResult.len
        else
          arg.fromChar = Math.max(view.lastSearchResult.offset - 1, 0)
      }

      searchData.saveToHistory()
      const p = backend.search(arg)
      maybeLongOperation('搜索中...', p)
      const ret = await p
      if (ret.found) {
        view.lastSearchResult = ret
        view.jumpTo(ret.index, true)
      } else {
        if (forward)
          ElMessage.warning(`向下搜索到文件结束未找到${arg.pattern}`)
        else
          ElMessage.warning(`向上搜索到文件开始未找到${arg.pattern}`)
      }
    }

    async function refresh() {
      return Promise.all([
        rootLogView.forceRefreshCache(),
        ...subLogViews.map(sub => sub.forceRefreshCache())
      ])
    }

    function searchSelectedWord() {
      const view = activeLogView.value
      if (view.selectedWord.length === 0) {
        return toolActions.beginSearch()
      }

      searchData.pattern = view.selectedWord
      searchData.caseSense = true
      searchData.regex = false
      search(true)
    }

    function filterSelectedWord() {
      const view = activeLogView.value
      if (view.selectedWord.length === 0) {
        return toolActions.beginFilter()
      }

      filter({
        pattern: view.selectedWord,
        regex: false,
        caseSense: true,
        reverse: false,
        logId: view.logId,
      })

    }

    function filter(arg: FilterArg) {
      maybeLongOperation(`正在过滤${arg.pattern}`, addSubViewByFilter(arg))
    }

    return {
      highlights,
      timeline,
      backend,
      rootLogId,
      rootLogView,
      subLogViews,
      activeLogView,
      followLog,
      activeSubIndex,
      logEncoding,
      searchData,
      curSyntax,
      type,
      title,
      name,
      openAction,
      desc,

      toolActions,
      addHighlight,
      rmHighlight,
      isActive,
      rmSubView,
      addSubViewByFilter,
      trackLine,
      addFocusedLineToTimeLine,
      toggleLogEncoding,
      search,
      refresh,
      searchSelectedWord,
      filter,
      filterSelectedWord
    };
  });
}
