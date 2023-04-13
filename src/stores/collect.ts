import { defineStore, storeToRefs } from "pinia";
import { useSettingsStore } from "./Settings";


export const useCollectStore = defineStore('collect', () => {
  const { enableCollect } = storeToRefs(useSettingsStore())

  function collect(type: string, extra: any) {
    if (!enableCollect.value)
      return
    const machineId = window.host.machineId
    const time = new Date().getTime();
    const payload = {
      type, machineId, time, extra
    }
    fetch('https://www.loginsight.top/api/collect', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify(payload)
    })
  }

  return {
    collect
  }
})