//@ts-ignore
import { dispatcher } from "./dispatcher";

export enum RegisterState {eTry, eTryEnd, eRegister}

export function inRange(range: LogRange, v: number): boolean {
  return v >= range.begin && v <= range.end;
}
export function rangeCount(range: LogRange): number {
  return range.end - range.begin
}
export function updateRange(dest: LogRange, src: LogRange) {
  if (dest.begin === src.begin && dest.end === src.end)
    return false

  dest.begin = src.begin
  dest.end = src.end

  return true
}
export function isValidRange(range: LogRange): boolean {
  return range.begin <= range.end
}

class Platform implements IPlatform {
  openDevTool() {
    return this.invoke("openDevTool")
  }
  getCmdlineArgs(): Promise<string[]> {
    return this.invoke('getCmdlineArgs')
  }
  isFile(path: string): Promise<boolean> {
    return this.invoke('isFile', path)
  }
  saveTextToClipboard(text: string) {
    return this.invoke("saveTextToClipboard", text);
  }
  readFile(path: string): Promise<string> {
    return this.invoke("readFile", path)
  }
  writeFile(path: string, data: string): Promise<string> {
    return this.invoke('writeFile', path, data)
  }
  getFilesUnderFolder(path: string): Promise<string[]> {
    return this.invoke('getFilesUnderFolder', path)
  }
  readClipboardToFile(): Promise<string> {
    return this.invoke('readClipboardToFile')
  }
  showOpenDialog(options: any): Promise<OpenFileDialogResult> {
    return this.invoke('showOpenDialog', options)
  }
  showSaveDialog(options: any): Promise<SaveFileDialogResult> {
    return this.invoke('showSaveDialog', options)
  }
  saveImageToClipboard(imgData: string) {
    return this.invoke("saveImageToClipboard", imgData)
  }
  private invoke = (func:string, ...args: any[]) =>window.host.ipcInvoke('platform', func, ...args);

  async createBackend(): Promise<IBackend> {
    const parcel = await this.invoke('createBackend')
    return new Backend(parcel)
  }
}

export enum LineSegType {
  Date, LogLevel, Num, Str
}

class Backend implements IBackend {
  private id: string
  private callbackId: string
  private invoke = (func:string, ...args: any[]) =>window.host.ipcInvoke(this.id, func, ...args);
  private disposed = false

  constructor(parcel: IBackendParcel) {
    this.id = parcel.id
    this.callbackId = `${this.id}-callback`
  }

  doRegister(arg: DoRegisterArg): Promise<DoRegisterResult> {
      return this.invoke('doRegister', arg)
  }

  getRegisterState(): Promise<InitRegisterResult> {
      return this.invoke('getRegisterState')
  }

  clearLog(arg:{ logId: number }): Promise<void> {
      return this.invoke('clearLog', arg)
  }
  exportLog(arg: ExportLogArg): Promise<any> {
    return this.invoke('exportLog', arg)
  }
  openMultiFile(arg: OpenFolderArg): Promise<OpenLogResult> {
    return this.invoke('openMultiFile', arg)
  }
  openProcess(arg: OpenProcessArg): Promise<OpenLogResult> {
    return this.invoke('openProcess', arg)
  }
  setLineSegment(arg: SetLineSegmentArg): Promise<void> {
    return this.invoke('setLineSegment', arg)
  }
  testSyntax(arg: TestSyntaxArg): Promise<TestSyntaxResult> {
    return this.invoke('testSyntax', arg)
  }
  search(arg: SearchArg): Promise<SearchResult> {
    return this.invoke('search', arg)
  }
  mapOffsetFromLog(line: string, values: LineRegion[]): Promise<LineRegion[]> {
    return this.invoke('mapOffsetFromLog', line, values)
  }
  mapLine(arg: { logId: number; index: number; }): Promise<MapLineResult> {
    return this.invoke('mapLine', arg)
  }
  filter(arg: FilterArg): Promise<OpenLogResult> {
    return this.invoke('filter', arg)
  }
  syncLogs(): Promise<SyncLogsResult> {
    return this.invoke('syncLogs',{})
  }

  setLogEncoding(encoding: string) {
    this.invoke('setLogEncoding', encoding)
  }

  start() {
    return this.invoke('start')
  }

  getLines(arg: { logId: number; range: LogRange; }): Promise<GetLinesResult> {
    return this.invoke('getLines', arg)
  }
  

  openFile(arg: { path: string; }): Promise<OpenLogResult> {
    return this.invoke('openFile', arg)
  }

  setListener(callback: IBackendCallback) {
    dispatcher.register(this.callbackId, callback)
    this.invoke('setListener', this.callbackId)
  }

  dispose() {
    if (this.disposed)
      return

    dispatcher.unregister(this.callbackId)
    this.invoke('dispose')
    this.disposed = true
  }
}

export const platform = new Platform()