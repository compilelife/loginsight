import { defineStore } from "pinia";
import { ref } from "vue";
import OpenProcDialog from "../views/OpenProcDialog.vue";
import OpenFolderDialog from "../views/OpenFolderDialog.vue";
import SettingsDlgVue from "../views/SettingsDlg.vue";
import SyntaxPreviewDialogVue from "../views/SyntaxPreviewDialog.vue";
import SyntaxManageDialog from "../views/SyntaxManageDialog.vue";
import { LogTabData } from "./LogTabData";
import ShortcutDialog from '../views/ShortcutDialog.vue'

export const useDialogStore = defineStore('dialogs', ()=>{
  const refToOpenProcDlg = ref<InstanceType<typeof OpenProcDialog>>()
  const refToOpenFolderDlg = ref<InstanceType<typeof OpenFolderDialog>>()
  const refToSettingsDlg = ref<InstanceType<typeof SettingsDlgVue>>()
  const refToSyntaxDlg = ref<InstanceType<typeof SyntaxPreviewDialogVue>>()
  const refToShortcutDlg = ref<InstanceType<typeof ShortcutDialog>>()
  const refToSyntaxManagerDlg = ref<InstanceType<typeof SyntaxManageDialog>>()

  function showOpenProcDlg() {
    return refToOpenProcDlg.value!.show()
  }

  function showOpenFolderDlg(folder: string, files: string[]) {
    return refToOpenFolderDlg.value!.show(folder, files)
  }

  function showSettingsDlg() {
    refToSettingsDlg.value!.show()
  }

  function showSyntaxDlg(tab: LogTabData, previewLines: Array<Line>) {
    refToSyntaxDlg.value!.show(tab, previewLines)
  }

  function showShortcutDlg() {
    refToShortcutDlg.value!.show()
  }

  function showSyntaxManagerDlg() {
    return refToSyntaxManagerDlg.value!.show()
  }

  return {
    refToOpenProcDlg,
    refToOpenFolderDlg,
    refToSettingsDlg,
    refToSyntaxDlg,
    refToShortcutDlg,
    refToSyntaxManagerDlg,

    showOpenProcDlg,
    showOpenFolderDlg,
    showSettingsDlg,
    showSyntaxDlg,
    showShortcutDlg,
    showSyntaxManagerDlg
  }
})