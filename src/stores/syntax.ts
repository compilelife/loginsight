import { defineStore } from "pinia";
import { Syntax } from "./LogTabData";
import { reactive } from "vue";
import { ElMessage } from "element-plus";

export interface SyntaxStoreItem extends Syntax{
  name: string;
  previewLog: string;
}

const STORAGE_KEY = 'syntax_items';

export const useSyntaxStore = defineStore('syntax', () => {
  const items = reactive<Array<SyntaxStoreItem>>(JSON.parse(localStorage.getItem(STORAGE_KEY) || '[]'));

  const addItem = (item: SyntaxStoreItem) => {
    items.unshift(item)
    sync()
  }

  const removeItem = (index: number) => {
    items.splice(index, 1);
    sync()
  }

  const sync = () => {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(items));
  }


  return {
    items,
    addItem,
    removeItem,
    sync
  }
})

