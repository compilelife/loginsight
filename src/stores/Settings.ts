//用defineStore定义设置项，包括名字，类型，描述，取值范围
import { defineStore } from 'pinia';
import { computed, ref } from 'vue';
import { getDefaultFont } from './util';

export const useSettingsStore = defineStore('settings', () => {
  const logFontSize = ref(parseInt(localStorage.getItem('logFontSize')!) || 14);
  const logFontFamily = ref(localStorage.getItem('logFontFamily') || getDefaultFont());
  const defaultEncoding = ref(localStorage.getItem('defaultEncoding') || 'UTF-8');
  const lineSpacing = ref(parseFloat(localStorage.getItem('lineSpacing')!) || 1.5);
  const enableCollect = ref(true)

  const cssLogFont = computed(()=>`${logFontSize.value}px/${lineSpacing.value} ${logFontFamily.value}`)

  const save = () => {
    localStorage.setItem('logFontSize', logFontSize.value.toString());
    localStorage.setItem('logFontFamily', logFontFamily.value);
    localStorage.setItem('defaultEncoding', defaultEncoding.value);
    localStorage.setItem('lineSpacing', lineSpacing.value.toString());
  }

  return {
    logFontSize,
    logFontFamily,
    cssLogFont,
    defaultEncoding,
    lineSpacing,
    enableCollect,
    save
  }
});