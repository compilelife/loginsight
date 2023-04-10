import { ElLoading, LoadingParentElement } from "element-plus"
import { ComponentPublicInstance, ComponentOptionsBase, Ref } from "vue"

//用作白色文字的背景
export const safeBgColorForWhiteText = [
  '#337ecc',
  ' #79bbff',
  ' #529b2e',
  ' #95d475',
  ' #b88230',
  ' #eebe77',
  ' #c45656',
  ' #f89898',
  ' #73767a'
]
function randomItemFrom<T>(arr: Array<T>):T {
  const index = Math.floor(Math.random() * arr.length)
  return arr[index]
}
export function nextRandHighlightColor() {
  return randomItemFrom(safeBgColorForWhiteText)
}

//用作白色背景的文字颜色
export const safeColorForWhiteBg = [
  '#67C23A',
  '#409EFF',
  '#F56C6C',
  '#E6A23C',
  '#f00',
  '#00f'
]
export function nextRandTextColor() {
  return randomItemFrom(safeColorForWhiteBg)
}


//长时操作提示
let longOpFinished = false
let loadingInstance: {
  close: () => void;
} | null = null

export function maybeLongOperation(text:string, p: Promise<any>) {
  longOpFinished = false

  p.finally(() => {
    longOpFinished = true
    loadingInstance?.close()
  })

  setTimeout(()=>{
    if (longOpFinished)
      return
    loadingInstance = ElLoading.service({
      text
    })
  }, 1000)
}

export function measureTextWidth(text:string, font: string) {
  const canvas = document.createElement('canvas')
  const context = canvas.getContext('2d')!
  context.font = font
  const ret = context.measureText(text)
  return ret.width
}

let tabId = 0;
export function nextTabId() {
  return `log-${++tabId}`
}

export const commonFonts: {[key:string]: string[]} = {
  'win32': ['Courier New', 'Arial', 'Microsoft YaHei', 'Cascadia Mono'],
  'linux': ["文泉驿等宽微米黑", "DejaVu Sans Mono", "Liberation Mono"],
  'darwin': ["Menlo", "Monaco", "华文等距"]
}

export function getDefaultFont(): string {
  return commonFonts[window.host.platform][0] ?? 'monospace'
}