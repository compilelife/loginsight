import { defineStore } from "pinia";
import {platform, RegisterState} from '../ipc/platform'
import {computed, Ref, ref, watch} from 'vue'
import { useDialogStore } from "./dialogs";
import { computedAsync } from "@vueuse/core";
import { ElMessage } from 'element-plus';

interface RegisterBackend {
  getRegisterState(): Promise<InitRegisterResult>
  doRegister(arg: DoRegisterArg): Promise<DoRegisterResult>
}

class CyRegisterBackend implements RegisterBackend {
  core!: IBackend
  async init() {
    const core = await platform.createBackend()
    await core.start()
    return core
  }

  getRegisterState(): Promise<InitRegisterResult> {
    return this.core.getRegisterState()
  }
  doRegister(arg: DoRegisterArg): Promise<DoRegisterResult> {
    return this.core.doRegister(arg)
  }
}

class AppleRegisterBackend implements RegisterBackend {
  constructor(rstate: Ref<InitRegisterResult>) {
    platform.setIAPListener((msg: IAPMessage)=>{
      switch(msg.type) {
        case 'purchased':
          ElMessage.success('购买成功')
          rstate.value = { rstate: RegisterState.eRegister, left: 0 }
          break
        case 'failed':
          ElMessage.error('购买失败')
          rstate.value = { rstate: RegisterState.eTry, left: 0 }
          break
        default:
          break
      }
    })
  }
  async getRegisterState(): Promise<InitRegisterResult> {
    const purchased = await platform.isIAPPurchased()
    if (purchased) {
      return { rstate: RegisterState.eRegister, left: 0 }
    }
    return { rstate: RegisterState.eTry, left: 0 }
  }
  async doRegister(arg: DoRegisterArg): Promise<DoRegisterResult> {
    const requestSuccess = await platform.IAPPurchase(arg.orderId)
    if (requestSuccess) {
      return { ok: true, info: '请求成功' }
    } else {
      return { ok: false, info: '请求失败' }
    }
  }
}


export const useRegister = defineStore('register', ()=>{

  const rstate = ref({ rstate: RegisterState.eRegister, left: 0 })

  const backend = computedAsync(async()=>{
    let ret
    if (window.host.platform === 'darwin') {
      ret = new AppleRegisterBackend(rstate)
    } else {
      ret = new CyRegisterBackend()
      await ret.init()
    }
    rstate.value = await ret.getRegisterState()
    return ret
  }) 

  const tryEndWord = computed(()=>{
    return window.host.platform === 'darwin' ? '免费版' : '试用版'
  })

  const registerWord = computed(()=>{
    return window.host.platform === 'darwin'? '专业版' : '已注册'
  })

  const hint = computed(()=>{
    const s = rstate.value
    if (!s) return ''
    switch (s.rstate) {
      case RegisterState.eRegister: return registerWord.value
      case RegisterState.eTry: {
        if (window.host.platform === 'darwin')
          return tryEndWord.value
        else
          return `${tryEndWord.value}：剩余${Math.ceil(s.left/3600)}小时`
      }
      case RegisterState.eTryEnd: return '试用结束'
    }
  })


  function registerAction() {
    if (rstate.value.rstate === RegisterState.eRegister) {
      ElMessage.info('您已是专业版用户')
      return
    }

    if (window.host.platform === 'darwin')
      useDialogStore().showBuyDlg()
    else
      useDialogStore().showRegisterDlg()
  }

  function doRegister(arg: DoRegisterArg) {
    return backend.value?.doRegister(arg)
  }


  return {
    tryEndWord,
    registerWord,
    rstate,
    hint,
    doRegister,
    registerAction,
  }
})