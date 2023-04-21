import { defineStore } from "pinia";
import { Syntax } from "./LogTabData";
import { reactive } from "vue";

export interface SyntaxStoreItem extends Syntax{
  name: string;
  previewLog: string;
  unmodify?: boolean;
}

const STORAGE_KEY = 'syntax_items';

export const useSyntaxStore = defineStore('syntax', () => {
  const items = reactive<Array<SyntaxStoreItem>>(JSON.parse(localStorage.getItem(STORAGE_KEY) || '[]'));
  addBuiltIn()

  const addItem = (item: SyntaxStoreItem) => {
    items.unshift(item)
    sync()
  }

  const removeItem = (index: number) => {
    if (items[index].unmodify)
      return
    items.splice(index, 1);
    sync()
  }

  const sync = () => {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(items.filter(item=>!item.unmodify)));
  }

  function addBuiltIn() {
    //内置语法规则
    items.push({
      name: 'nginx access log',
      pattern: '^(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})\\s-\\s-\\s\\[(\\d{1,2}/\\w+/\\d{4}:\\d{2}:\\d{2}:\\d{2}\\s\\+\\d{4})\\]',
      fields: [
        {name: 'ip', color: '#b88230'},
        {name: 'date', color: '#529b2e'}
      ],
      unmodify: true,
      previewLog: '<mark style="color: rgb(184, 130, 48);">113.214.205.214</mark> - - [<mark style="color: rgb(82, 155, 46);">21/Apr/2023:16:26:26 +0800</mark>] "GET /logo192.png HTTP/1.1" 200 645 "https://www.loginsight.top/" "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_3) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.5005.124 Safari/537.36 Edg/102.0.1245.44" "-"'
    }, {
      name: 'adb logcat',
      pattern: /(\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}\.\d{3})\s+(\d+)\s+(\d+)\s+([A-Z]+)\s+(\S+)/.source,
      fields: [
        {name: 'date', color: '#67C23A'},
        {name: 'pid', color: '#409EFF'},
        {name: 'tid', color: '#F56C6C'},
        {name: 'level', color: '#E6A23C'},
        {name: 'tag', color: '#00f'},
      ],
      unmodify: true,
      previewLog: '<mark style="color: rgb(103, 194, 58);">01-01 08:00:10.237</mark>  <mark style="color: rgb(64, 158, 255);">2585</mark>  <mark style="color: rgb(245, 108, 108);">2585</mark> <mark style="color: rgb(230, 162, 60);">I</mark> <mark style="color: rgb(0, 0, 255);">RTK_OMX_DEC_VIDEO:</mark> vpu-&gt;instIdx : 0'
    })
  }


  return {
    items,
    addItem,
    removeItem,
    sync
  }
})

