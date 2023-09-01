import { defineStore } from "pinia";
import {platform, RegisterState} from '../ipc/platform'
import {computed, ref, watch} from 'vue'
import { useDialogStore } from "./dialogs";
import { computedAsync } from "@vueuse/core";

export const useRegister = defineStore('register', ()=>{
  const backend = computedAsync(async()=>{
    const core = await platform.createBackend()
    await core.start()
    return core
  })

  const rstate = computedAsync(()=>{
    return backend.value.getRegisterState()
  })

  const hint = computed(()=>{
    const s = rstate.value
    if (!s) return ''
    switch (s.rstate) {
      case RegisterState.eRegister: return '已注册！'
      case RegisterState.eTry: return `试用版：剩余${Math.ceil(s.left/3600)}小时`
      case RegisterState.eTryEnd: return '试用结束'
    }
  })


  function registerAction() {
    if (rstate.value.rstate === RegisterState.eRegister) {
      return
    }

    useDialogStore().showRegisterDlg()
  }

  function doRegister(arg: DoRegisterArg) {
    return backend.value?.doRegister(arg)
  }


  return {
    rstate,
    hint,
    doRegister,
    registerAction,
  }
})