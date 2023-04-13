import { computed, reactive, ref, toRaw, watch } from 'vue'
import { defineStore } from 'pinia'
import { LogTabData, newLogTabData } from './LogTabData'
import { platform, rangeCount } from '../ipc/platform'
import { ElMessage, ElMessageBox } from 'element-plus'
import { maybeLongOperation, nextTabId } from './util'
import { useDialogStore } from './dialogs'
import {openProject as openProjectImpl} from './project'
import { OpenFileRecentItem, OpenFolderRecentItem, OpenProcRecentItem, OpenProjectRecentItem, RecentItem, useRecents } from './recents'
import {useCollectStore} from './collect'

export type TabType = 'log' | 'welcome'
export interface Tab {
  name: string //唯一标识
  desc?: string 
  title: string
  type: TabType
}

export const useTabsStore = defineStore('tabs', () => {
  const pathSeparator = window.host.platform === 'win32' ? '\\' : '/'
  const {collect} = useCollectStore()

  const tabs = reactive<Array<Tab>>([{
    name: nextTabId(),
    title: '欢迎',
    type: 'welcome',
  }])
  const currentTab = ref(0)

  const { addRecent } = useRecents()

  function addTab(tab: Tab) {
    if (tabs[tabs.length - 1]?.type === 'welcome') {
      tabs.pop();
    }
    tabs.push(tab)
    currentTab.value = tabs.length - 1
  }

  function rmTab(name: string) {
    const index = tabs.findIndex(tab => tab.name === name)
    if (tabs[index].type === 'log') {
      (tabs[index] as LogTabData).backend.dispose()
    }
    tabs.splice(index, 1)
    if (tabs.length === 0) {
      addTab({ name: nextTabId(), title: '欢迎', type: 'welcome' })
    }
    currentTab.value = tabs.length - 1
  }

  async function openToNewTab(builder: (backend: IBackend) => Promise<OpenLogResult>, hint: string, tabName: string = "Untitled") {
    const backend = await platform.createBackend()
    await backend.start()
    let rootLog
    try {
      const p = builder(backend)
      maybeLongOperation(hint, p)
      rootLog = await p
    } catch (e: any) {
      ElMessageBox.alert(e, '', { type: 'error' })
      await backend.dispose()
      return
    }

    const tabData = newLogTabData(tabName, backend, rootLog)
    addTab(tabData)
    return tabData
  }

  async function doOpenProc(arg: OpenProcRecentItem) {
    addRecent(arg)
    const tabData = await openToNewTab(backend => backend.openProcess(arg.args), `正在打开程序：${arg.args.process}`, arg.name)
    if (tabData){
      collect('openProcess', {
        process: arg.args.process,
      })
      tabData.desc = arg.args.process
      tabData.openAction = arg
    }
    return tabData
  }

  async function openProc() {
    const { showOpenProcDlg } = useDialogStore()
    const ret = await showOpenProcDlg()
    if (ret) {
      const title = ret.process.split(' ').shift() ?? 'Untitled'
      return doOpenProc({
        name: title,
        type: 'proc',
        args: ret
      })
    }
  }

  function getFilename(path: string) {
    return path.split(pathSeparator).pop() || 'Untitled'
  }

  async function doOpenFile(path: string, saveToRecent: boolean = true) {
    const fileName =getFilename(path)
    const action: RecentItem = {
      type: 'file',
      name: fileName,
      args: { path }
    }
    saveToRecent && addRecent(action)
    const tabData = await openToNewTab(backend => backend.openFile({ path }), `打开文件中：${path}`, fileName)
    if (tabData) {
      tabData.openAction = action
      tabData.desc = path
      collect('openFile', {
        fileName,
        lineCount: rangeCount(tabData.rootLogView.range)
      })
    }
    return tabData
  }

  async function openClipboard() {
    const filePath = await platform.readClipboardToFile()
    const ret = await doOpenFile(filePath, false)
    if (ret)
      ret.desc = '粘贴板内容'
    return ret
  }

  async function openFile() {
    const { canceled, filePaths } = await platform.showOpenDialog({
      properties: ['openFile'],
      filters: [
        { name: 'Log Files', extensions: ['log'] },
        { name: 'All Files', extensions: ['*'] }
      ]
    })
    if (canceled) {
      return
    }

    const path = filePaths[0]
    await doOpenFile(path)
  }

  async function doOpenFolder(arg: OpenFolderRecentItem | string): Promise<LogTabData|undefined> {
    if (typeof arg === 'string') {
      const { showOpenFolderDlg } = useDialogStore()
      const folder = arg
      let files = await platform.getFilesUnderFolder(folder)
      files = await showOpenFolderDlg(folder, files)

      const name = folder.split(pathSeparator).pop() || 'Untitled'
      const args = { files: files.map(file => folder + pathSeparator + file) }
      return doOpenFolder({
        type: 'folder',
        name,
        args
      })
    }

    addRecent(arg)
    const tabData = await openToNewTab(backend => backend.openMultiFile(arg.args), `正在打开文件夹: ${arg.name}`, arg.name)
    if (tabData) {
      tabData.openAction = arg
      tabData.desc = `文件夹: ${arg.name}`
      collect('openFolder', {
        folder: arg.name,
        lineCount: rangeCount(tabData.rootLogView.range),
        files: arg.args.files,//一般有几个文件，什么后缀
      })
    }
    return tabData
  }

  async function openFolder() {
    const { canceled, filePaths } = await platform.showOpenDialog({
      properties: ['openDirectory'],
    })
    if (canceled) {
      return
    }

    const folder = filePaths[0]
    return doOpenFolder(folder)
  }

  async function openRecentItem(recent: RecentItem) {
    recent = toRaw(recent)
    switch (recent.type) {
      case 'file': {
        const args = (recent as OpenFileRecentItem).args
        return doOpenFile(args.path)
      }
      case 'folder': {
        return doOpenFolder((recent as OpenFolderRecentItem))
      }
      case 'proc': {
        return doOpenProc((recent as OpenProcRecentItem))
      }
      case 'project': {
        return doOpenProject((recent as OpenProjectRecentItem).args.path)
      }
      default: break
    }
  }

  async function doOpenProject(path: string){
    const content = await platform.readFile(path)
    if (content.length === 0) {
      ElMessage.error('文件打开失败，或文件为空')
      return
    }

    const name = getFilename(path)
  
    addRecent({
      type: 'project',
      name,
      args: {
        path
      }
    })
    collect('openProject', {
      name
    })
    openProjectImpl(JSON.parse(content))
  }

  async function openProject() {
    const { canceled, filePaths } = await platform.showOpenDialog({
      properties: ['openFile'],
      filters: [
        { name: 'project files', extensions: ['liprj'] },
      ]
    })
    if (canceled) {
      return
    }
  
    let path = filePaths[0]
    if (!path.endsWith('.liprj')) {
      path = `${path}.liprj`
    }
    return doOpenProject(path)
  }

  async function smartOpen(path: string) {
    let isFile = false
    try {
      isFile = await platform.isFile(path)
    }catch (e) {
      console.warn(`${path} is not a file, ignore it`)
      return
    }

    if (isFile) {
      if (path.endsWith('.liprj')) {
        return doOpenProject(path)
      } else {
        doOpenFile(path, true)
      }
    } else {
      doOpenFolder(path)
    }
  }

  async function exportCurLogView(){
    const tab = tabs[currentTab.value] as LogTabData
    const view = tab.activeLogView
    const count = rangeCount(view.range)
    if (count > 100000) {
      try{
        await ElMessageBox.confirm(
          `当前日志较大，有${count}行，是否确定导出？`,
          '导出日志',
          {
            confirmButtonText: '导出',
            cancelButtonText: '取消'
          }
        )
      }catch (e){return}
    }

    const { canceled, filePath } = await platform.showSaveDialog({
      properties: ['openFile'],
    })
    if (canceled)
      return
    
    await tab.backend.exportLog({
      path: filePath!,
      from: view.range.begin,
      to: view.range.end,
      logId: view.logId,
      all: true,
    })

    ElMessage.success(`成功导出到: ${filePath}`)
  }

  return {
    tabs,
    currentTab,

    addTab,
    rmTab,
    openFile,
    openClipboard,
    openProc,
    openFolder,
    openRecentItem,
    openProject,
    smartOpen,
    exportCurLogView
  }
})

const storeInstanceId = new Map<string, number>()
export function createStoreInstance<T>(name: string, builder: () => T) {
  const id = (storeInstanceId.get(name) || 0) + 1
  storeInstanceId.set(name, id)
  const key = `${name}-${id}`
  return defineStore(key, builder)()
}