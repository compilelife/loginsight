import { defineStore } from "pinia";
import { computed, ref } from "vue";

export interface UpdateInfo {
    version: string;
    change: string;
}

export const useUpdateStore = defineStore('update', ()=>{
    const currentVersion = 'v3.2.0'
    const newVersion = ref<UpdateInfo|null>(null)
    const hasNewVersion = computed(()=>{
        return newVersion.value && newVersion.value?.version != currentVersion
    })

    async function checkNewVersion() {
        let  newVersionResponse: UpdateInfo
        try {
            const response = await fetch('http://www.loginsight.top/api/web/version');
            newVersionResponse = await response.json();
        }catch(e) {
            console.error('fetch new version failed', e)
            return
        }
        
        newVersion.value = newVersionResponse
    }

    return {
        currentVersion,
        newVersion,
        hasNewVersion,

        checkNewVersion
    }
})

