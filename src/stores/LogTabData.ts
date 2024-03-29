import { computed, reactive, ref, unref, watch, watchEffect } from 'vue';
import { LineSegType, inRange } from '../ipc/platform';
import { ElMessage, ElMessageBox } from 'element-plus';
import { createStoreInstance, TabType } from './tabsStore';
import { Highlight } from "./Highlight";
import { newTimeLineData, newTimeLineNodeData, TimeLineData } from "./TimeLineData";
import { newLogViewData, LogViewData } from "./LogViewData";
import { newSearchData } from './SearchData';
import { maybeLongOperation, nextTabId } from './util';
import { RecentItem } from './recents';
import { useSettingsStore } from './Settings';
import { useCollectStore } from './collect';


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
    const canClear = ref(false)
    const openAction = ref<RecentItem>()
    const toolActions = {
      beginSearch: ()=>{},
      beginFilter: ()=>{},
      beginJump: ()=>{},
    }
    const {collect} = useCollectStore()

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
      if (searchData.pattern.length === 0) {
        ElMessage.warning('未指定搜索词')
        return
      }

      //全词匹配的处理：转换为\b正则匹配
      let {pattern, regex} = searchData
      if (searchData.wholeWord) {
        if (searchData.regex) {
          pattern = '\\b'+pattern+'\\b'
        } else {
          pattern = '\\b'+pattern.replace(/[\\^$.*+?()[\]{}|]/g, "\\$&")+'\\b'
          regex = true
        }
      }

      const arg: SearchArg = {
        fromChar: 0,
        fromLine: view.focusLineIndex,
        logId: view.logId,
        reverse: !forward,
        toLine: forward ? view.range.end : view.range.begin,
        caseSense: searchData.caseSense,
        regex: regex,
        pattern: pattern
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
      maybeLongOperation(`搜索${arg.pattern}...`, p)
      const ret = await p
      if (ret.found) {
        view.lastSearchResult = ret
        view.jumpTo(ret.index, true)
      } else {
        const noop = ()=>{}
        if (forward) {
          ElMessageBox.confirm(
            `从${arg.fromLine+1}行向下搜索到文件末尾，未找到${arg.pattern}，是否从文件头继续搜索？`,
            '搜索',
            {
              type: 'warning',
              confirmButtonText: '确定',
              cancelButtonText: '取消'
            }).then(async()=>{
              arg.toLine = arg.fromLine;
              arg.fromLine = view.range.begin;
              const p = backend.search(arg)
              maybeLongOperation('从头搜索中...', p)
              const ret = await p
              if (ret.found) {
                view.lastSearchResult = ret
                view.jumpTo(ret.index, true)
              } else {
                ElMessage.warning(`搜索完整个文件，未找到${arg.pattern}`)
              }
            }, noop)
        }
        else {
          ElMessageBox.confirm(
            `从${arg.fromLine+1}行向上搜索到文件开头，未找到${arg.pattern}，是否从文件末尾继续搜索？`,
            '搜索',
            {
              type: 'warning',
              confirmButtonText: '确定',
              cancelButtonText: '取消'
            }).then(async()=>{
              arg.toLine = arg.fromLine;
              arg.fromLine = view.range.end;
              const p = backend.search(arg)
              maybeLongOperation('从末尾搜索中...', p)
              const ret = await p
              if (ret.found) {
                view.lastSearchResult = ret
                view.jumpTo(ret.index, true)
              } else {
                ElMessage.warning(`搜索完整个文件，未找到${arg.pattern}`)
              }
            }, noop)
        }
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
      searchData.wholeWord = false
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

    async function setSyntax(syntax: Syntax) {
      await backend.setLineSegment({
        pattern: syntax.pattern,
        caseSense: true,
        segs: syntax.fields.map(field=>({
          type: LineSegType.Str,
          name: field.name,
          extra: {}
        }))
      })
    
      curSyntax.value = syntax
    
      await refresh()
    
      collect('setSyntax', curSyntax.value)
    }

    function clearLog() {
      rootLogView.dropCache()
      subLogViews.forEach(s => s.dropCache())
      return backend.clearLog({logId: rootLogId})
    }

    function save() {
      return {
        highlights: highlights.map(v=>v.save()),
        openAction: unref(openAction!),
        timeline: {
          nodes: timeline.nodes.map(v=>v.save())
        },
        followLog: followLog.value,
        logEncoding: logEncoding.value,
        curSyntax: curSyntax.value,
        rootLogView: {
          title: rootLogView.title,
          curLineIndex: rootLogView.curLineIndex,
          focusLineIndex: rootLogView.focusLineIndex
        },
        subLogViews: subLogViews.map(subLogView => ({
          title: subLogView.title,
          curLineIndex: subLogView.curLineIndex,
          focusLineIndex: subLogView.focusLineIndex,
          openAction: subLogView.openAction
        }))
      }
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
      canClear,

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
      filterSelectedWord,
      setSyntax,
      clearLog,
      save
    };
  });
}
