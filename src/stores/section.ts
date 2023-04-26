import { defineStore } from "pinia";
import { inRange } from "../ipc/platform";
import { reactive } from "vue";

interface Section extends LogRange{
  color: string;
}

export const useSectionStore = defineStore('section', ()=>{
  const sections = reactive<Array<Section>>([])

  function addSection(section: Section): void {
    let insects = sections.filter(s=>s.begin <= section.end && s.end >= section.begin)
    const neibours = sections.filter(s=>s.begin === section.end || s.end === section.begin)

    function join(s1: Section) {
      section.begin = Math.min(section.begin, s1.begin)
      section.end = Math.max(section.end, s1.end)
      sections.splice(sections.indexOf(s1), 1)
    }

    neibours.forEach(join)
    
    for (const insect of insects) {
      if (insect.color === section.color) {
        join(insect)
      } else {
        const left = section.begin - insect.begin
        const right = insect.end - section.begin
        if (left > 0) {
          sections.push({begin: insect.begin, end: left-1, color: insect.color})
        }
        if (right > 0) {
          sections.push({begin: section.end+1, end: insect.end, color: insect.color})
        }
        sections.splice(sections.indexOf(insect), 1)
      }
    }

    sections.push(section)
  }

  function getLineColor(line: number): string {
    const s = sections.find(s=>inRange(s, line))
    return s?.color ?? 'transparent'
  }

  function rmSection(section: Section) {
    let index = sections.indexOf(section)
    if (index < 0) {
      index = sections.findIndex(s=>s.begin === section.begin && s.end === section.end)
    }
    sections.splice(index, 1)
  }
})

