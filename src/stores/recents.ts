import { Clipboard as OpenClipboardIcon, FileCabinet as OpenProjectIcon, FileText as OpenFileIcon, FolderClose as OpenFolderIcon, Terminal as OpenProcIcon } from '@icon-park/vue-next';
import { defineStore } from "pinia";
import { reactive } from "vue";

export interface RecentItem {
  type: 'file' | 'folder' | 'project' | 'proc'
  name: string
  args: any
}

export interface OpenProcRecentItem extends RecentItem {
  args: OpenProcessArg
}

export interface OpenFileRecentItem extends RecentItem {
  args: OpenFileArg
}

export interface OpenFolderRecentItem extends RecentItem {
  args: OpenFolderArg
}

export interface OpenProjectRecentItem extends RecentItem {
  args: {path:string}
}

export const getRecentItemIcon = (type: string) => {
  switch (type) {
    case 'file':
      return OpenFileIcon;
    case 'project':
      return OpenProjectIcon;
    case 'folder':
      return OpenFolderIcon;
    case 'proc':
      return OpenProcIcon;
    case 'clipboard':
      return OpenClipboardIcon;
    default:
      return OpenFileIcon;
  }
}

//定义一个名为recents的store
export const useRecents = defineStore('recents', () => {
  //使用local storage存取、初始化recents
  const recents = reactive<RecentItem[]>(JSON.parse(localStorage.getItem('recents') || '[]'))

  //保留最新的10项
  function addRecent(item: RecentItem) {
    const index = recents.findIndex(r => r.name === item.name && r.type === item.type)
    if (index !== -1) {
      recents.splice(index, 1)
    }
    recents.unshift(item)
    if (recents.length > 10) {
      recents.pop()
    }
    localStorage.setItem('recents', JSON.stringify(recents))
  }

  function clearRecents() {
    recents.splice(0, recents.length)
    localStorage.removeItem('recents')
  }

  return {
    recents,

    addRecent,
    clearRecents
  }
})
