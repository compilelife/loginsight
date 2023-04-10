import { ElMessage, ElMessageBox } from 'element-plus'
import key from 'keymaster'
import { computed, watch } from 'vue'
import { platform } from '../ipc/platform'
import { newHighlight } from './Highlight'
import { LogTabData } from './LogTabData'
import { useTabsStore } from './tabsStore'
import { nextRandHighlightColor } from './util'

interface KeyBinding {
  key: string,
  desc: string,
  handler: () => void
}

export let logTabBindings: KeyBinding[] = []

export function registerShortcuts() {
  const tabs = useTabsStore()
  const currentTab = computed(() => {
    const tab = tabs.tabs[tabs.currentTab]
    if (tab && tab.type === 'log') {
      return tab as LogTabData
    }
    return null
  })

  watch(currentTab, () => {
    if (currentTab) {
      key.setScope('logTab')
    } else {
      key.setScope('all')
    }
  }, { immediate: true })

  logTabBindings = [
    {
      key: 'a',
      desc: '添加当前行到时间线',
      handler: () => currentTab.value?.addFocusedLineToTimeLine()
    },
    {
      key: 's, ctrl+f, ⌘+f',
      desc: '搜索当前日志视图',
      handler: () => currentTab.value?.toolActions.beginSearch()
    },
    {
      key: 'd, ctrl+g, ⌘+g',
      desc: '跳转到行',
      handler: () => currentTab.value?.toolActions.beginJump()
    },
    {
      key: 'f',
      desc: '过滤',
      handler: () => currentTab.value?.toolActions.beginFilter()
    },
    {
      key: 't',
      desc: '追踪当前行',
      handler: () => {
        const tab = currentTab.value
        if (!tab) return
        const view = tab.activeLogView
        tab.trackLine(view.focusLineIndex, view)
      }
    },
    {
      key: 'e',
      desc: '高亮被选词',
      handler: () => {
        const tab = currentTab.value
        if (!tab) return
        const view = tab.activeLogView
        if (view.selectedWord.length === 0) {
          ElMessage.warning('没有选中的单词，无法高亮')
          return
        }
        tab.addHighlight(newHighlight(
          view.selectedWord, nextRandHighlightColor()
        ))
      }
    },
    {
      key: 'pagedown',
      desc: '向下翻页',
      handler: () => {
        const tab = currentTab.value
        if (!tab) return
        const view = tab.activeLogView

        view.jumpTo(view.visibleLastLineIndex)
      }
    },
    {
      key: 'pageup',
      desc: '向上翻页',
      handler: () => {
        const tab = currentTab.value
        if (!tab) return
        const view = tab.activeLogView
        //FIXME: 因为没有记录每行高度，所以往前翻没办法估计准要显示多少行
        //如果当前视口所有行没折行，而上一个视口有折行，下面的计算方式就会导致一些行被跳过
        //也许多跳几次来缓存各行高度可以解决，就是视觉上很差劲
        const visibleLines = view.visibleLastLineIndex - view.curLineIndex
        view.jumpTo(Math.max(view.curLineIndex - visibleLines, view.range.begin))
      }
    },
    {
      key: 'ctrl+w, ⌘+w',
      desc: '关闭当前标签页',
      handler: () => {
        const tab = currentTab.value
        if (!tab) return

        ElMessageBox.confirm(
          '关闭当前整个标签页？',
          'Warning',
          {
            confirmButtonText: '确定',
            cancelButtonText: '取消',
            type: 'warning',
          }
        ) .then(() => useTabsStore().rmTab(tab.name))
          .catch(() => { })
      }
    },
    {
      key: 'f12',
      desc: '开发者工具',
      handler: () => {
        platform.openDevTool()
      }
    }
  ]

  for (const binding of logTabBindings) {
    key(binding.key, 'logTab', binding.handler)
  }
}

export function showShortcutsDialog() {

}