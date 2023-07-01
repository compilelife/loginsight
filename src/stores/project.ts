import { unref } from "vue";
import { LineSegType } from "../ipc/platform";
import { LogTabData } from "./LogTabData";
import { TabType, useTabsStore } from "./tabsStore";
import { newHighlight } from "./Highlight";
import { newTimeLineNodeData } from "./TimeLineData";

interface Project {
  version: string;
  tabs: ProjectTab[]
}

type ProjectTab = ReturnType<typeof saveLogTab>

function saveLogTab(tab: LogTabData) {
  return tab.save()
}

export function saveProject(): Project {
  const project: Project = {
    version: '1.0',
    tabs: [],
  }

  const { tabs } = useTabsStore()

  project.tabs = tabs.filter(tab => tab.type === 'log')
    .map(tab => saveLogTab(tab as LogTabData))

  return project
}

export async function openProject(project: Project) {
  const {openRecentItem} = useTabsStore()
  for (const tab of project.tabs) {
    const tabData = await openRecentItem(tab.openAction!)
    if (tabData) {
      tabData.highlights.push(...tab.highlights.map(v=>newHighlight(v.keyword, v.color)))
      tabData.timeline.nodes.push(...tab.timeline.nodes.map(v=>{
        const n = newTimeLineNodeData(v.line, v.text)
        n.color = v.color
        n.comment = v.comment
        return n
      }))
      tabData.followLog = tab.followLog
      tabData.logEncoding = tab.logEncoding
      tabData.curSyntax = tab.curSyntax
      if (tab.curSyntax.pattern.length > 0) {
        await tabData.backend.setLineSegment({
          pattern: tab.curSyntax.pattern,
          caseSense: true,
          segs: tab.curSyntax.fields.map(field=>({
            type: LineSegType.Str,
            name: field.name,
            extra: {}
          }))
        })
        await tabData.refresh()
      }
      tabData.rootLogView.title = tab.rootLogView.title
      tabData.rootLogView.curLineIndex
      tabData.rootLogView.focusLineIndex = tab.rootLogView.focusLineIndex
      for (const sub of tab.subLogViews) {
        const subData = await tabData.addSubViewByFilter(sub.openAction!)
        if (subData) {
          subData.title = sub.title
          subData.curLineIndex = sub.curLineIndex
          subData.focusLineIndex = sub.focusLineIndex
        }
      }
    }
  }
}