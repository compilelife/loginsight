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
      key: 'q',
      desc: '切换日志跟随',
      handler: () => {
        const tab = currentTab.value
        if (!tab) return
        tab.followLog = !tab.followLog
      }
    },
    {
      key: 'pagedown',
      desc: '向下翻页',
      handler: () => currentTab.value?.activeLogView.pageDown()
    },
    {
      key: 'pageup',
      desc: '向上翻页',
      handler: () => currentTab.value?.activeLogView.pageUp()
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