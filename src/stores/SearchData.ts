import { reactive, ref } from "vue";
import { createStoreInstance } from "./tabsStore";

export interface SearchHistory {
  caseSense: boolean
  regex: boolean
  pattern: string
  wholeWord: boolean
}

export function newSearchData() {
  return createStoreInstance('search', ()=>{
    const caseSense = ref(false)
    const regex = ref(false)
    const history = Array<SearchHistory>()
    const pattern = ref('')
    const wholeWord = ref(false)

    function getSuggestion(word: string) {
      return history.filter(item=>item.pattern.indexOf(word) !== -1)
    }

    function saveToHistory() {
      if (history.find(item => item.pattern === pattern.value))
        return
      history.push({
        caseSense: caseSense.value, 
        regex: regex.value, 
        pattern: pattern.value,
        wholeWord: wholeWord.value
      })
    }

    return {
      caseSense,
      regex,
      pattern,
      wholeWord,

      getSuggestion,
      saveToHistory
    }
  })
}