import { nativeImage,clipboard, dialog, BrowserWindow} from "electron";
import { dispatcher } from "./dispatcher"
import {ChildProcessWithoutNullStreams, spawn} from 'child_process'
import iconv from 'iconv-lite'
import fs from 'fs';
import path from 'path'
import os from 'os'

const nativeEncode = process.platform === "win32" ? 'gbk' : 'utf-8';

class Platform implements IPlatform {
  id = "platform";
  client: BrowserWindow
  args: string[] = []

  constructor() {
    dispatcher.registerEx(this.id, 'createBackend', ()=>this.createBackend())
    dispatcher.registerEx(this.id, 'saveImageToClipboard', this.saveImageToClipboard.bind(this))
    dispatcher.registerEx(this.id, 'showOpenDialog', this.showOpenDialog.bind(this))
    dispatcher.registerEx(this.id, 'showSaveDialog', this.showSaveDialog.bind(this))
    dispatcher.registerEx(this.id, 'readClipboardToFile', this.readClipboardToFile.bind(this))
    dispatcher.registerEx(this.id, 'getFilesUnderFolder', this.getFilesUnderFolder.bind(this))
    dispatcher.registerEx(this.id, 'writeFile', this.writeFile.bind(this))
    dispatcher.registerEx(this.id, 'readFile', this.readFile.bind(this))
    dispatcher.registerEx(this.id, 'saveTextToClipboard', this.saveTextToClipboard.bind(this))
    dispatcher.registerEx(this.id, 'isFile', this.isFile.bind(this))
    dispatcher.registerEx(this.id, 'getCmdlineArgs', this.getCmdlineArgs.bind(this))
    dispatcher.registerEx(this.id, 'openDevTool', this.openDevTool.bind(this))
  }
  openDevTool() {
    this.client.webContents.openDevTools()
  }
  async getCmdlineArgs(): Promise<string[]> {
    return this.args
  }
  async isFile(path: string): Promise<boolean> {
    const stat = await fs.promises.stat(path)
    return stat.isFile()
  }
  saveTextToClipboard(text: string) {
    clipboard.writeText(text)
  }
  async readFile(path: string): Promise<string> {
    return fs.promises.readFile(path, {encoding: 'utf8'})
  }
  async writeFile(path: string, data: string): Promise<string> {
    try {
      await fs.promises.writeFile(path, data, {encoding: 'utf-8'});
      return "";
    } catch (error) {
      console.error('writeFile', path, error)
      return error.message;
    }
  }
  async getFilesUnderFolder(folder: string): Promise<string[]> {
    const files = fs.readdirSync(folder);
    const fileNames = files.filter(file => fs.statSync(path.join(folder, file)).isFile()).map(file => path.basename(file));
    const sortedFileNames = fileNames.sort((a, b) => {
        const aStat = fs.statSync(path.join(folder, a));
        const bStat = fs.statSync(path.join(folder, b));
        return aStat.mtimeMs - bStat.mtimeMs;
    });
    return sortedFileNames;
  }
  async readClipboardToFile(): Promise<string> {
    const clipboardText = clipboard.readText();
    const filePath = path.join(os.tmpdir(), `clipboard_${Date.now()}.txt`);
    fs.writeFileSync(filePath, clipboardText);
    return filePath;
  }
  showOpenDialog(options: any): Promise<OpenFileDialogResult> {
    return dialog.showOpenDialog(this.client, options)
  }
  showSaveDialog(options: any): Promise<SaveFileDialogResult> {
    return dialog.showSaveDialog(this.client, options)
  }
  saveImageToClipboard(imgData: string) {
    const image = nativeImage.createFromDataURL(imgData);
    clipboard.writeImage(image);
  }
  createBackend(): IBackend {
    return new Backend()
  }

  sendToClient(channel: string, ...args: any[]): void {
    this.client.webContents.send('ipc', channel, ...args)
  }
}

enum CoreCmdState {Ok, Fail, Future, Cancel}

class Backend implements IBackend , IBackendParcel{
  static instanceCount: number = 0;

  id: string

  logEncoding: string = 'utf-8';

  private core: ChildProcessWithoutNullStreams
  private callback?: IBackendCallback
  private tail = ''
  private cmdId = 0
  private pendingCmds = new Map<string, {resolve: Function, reject: Function}>()

  constructor() {
    this.id = `Backend${Backend.instanceCount++}`;

    dispatcher.registerEx(this.id, 'setListener', (callbackName: string)=>{
      this.setListener((arg)=>{
        platform.sendToClient(callbackName, arg)
      })
    })
    dispatcher.registerEx(this.id, 'openFile', this.openFile.bind(this))
    dispatcher.registerEx(this.id, 'start', this.start.bind(this))
    dispatcher.registerEx(this.id, 'getLines', this.getLines.bind(this))
    dispatcher.registerEx(this.id, 'setLogEncoding', this.setLogEncoding.bind(this))
    dispatcher.registerEx(this.id, 'syncLogs', this.syncLogs.bind(this))
    dispatcher.registerEx(this.id, 'filter', this.filter.bind(this))
    dispatcher.registerEx(this.id, 'dispose', this.dispose.bind(this))
    dispatcher.registerEx(this.id, 'mapLine', this.mapLine.bind(this))
    dispatcher.registerEx(this.id, 'search', this.search.bind(this))
    dispatcher.registerEx(this.id, 'mapOffsetFromLog', this.mapOffsetFromLog.bind(this))
    dispatcher.registerEx(this.id, 'testSyntax', this.testSyntax.bind(this))
    dispatcher.registerEx(this.id, 'setLineSegment', this.setLineSegment.bind(this))
    dispatcher.registerEx(this.id, 'openProcess', this.openProcess.bind(this))
    dispatcher.registerEx(this.id, 'openMultiFile', this.openMultiFile.bind(this))
    dispatcher.registerEx(this.id, 'exportLog', this.exportLog.bind(this))
  }
  exportLog(arg: ExportLogArg): Promise<any> {
    arg.path = this.toNativeEncoded(arg.path)
    return this.sendToCore('exportLog', arg)
  }
  openMultiFile(arg: OpenFolderArg): Promise<OpenLogResult> {
    arg.files = arg.files.map(file => this.toNativeEncoded(file))
    return this.sendToCore('openMultiFile', arg)
  }
  openProcess(arg: OpenProcessArg): Promise<OpenLogResult> {
    arg.process = this.toNativeEncoded(arg.process)
    return this.sendToCore('openProcess', arg)
  }
  setLineSegment(arg: SetLineSegmentArg): Promise<void> {
    arg.pattern = this.toLogEncoding(arg.pattern)
    return this.sendToCore('setLineSegment', arg)
  }
  async testSyntax(arg: TestSyntaxArg): Promise<TestSyntaxResult> {
    arg.pattern = this.toLogEncoding(arg.pattern)
    for (let i = 0; i < arg.lines.length; i++) {
      const line = arg.lines[i];
      arg.lines[i] = this.toLogEncoding(line)
    }
    return await this.sendToCore<TestSyntaxResult>('testSyntax', arg)
  }

  mapOffsetFromLog(line: string, values:LineRegion[]):LineRegion[] {
    const buf = iconv.encode(line, this.logEncoding)
    return values.map(({offset, length})=>{
      return {
        offset: iconv.decode(buf.slice(0, offset), this.logEncoding).length,
        length: iconv.decode(buf.slice(offset, offset+length), this.logEncoding).length
      }
    })
  }

  search(arg: SearchArg): Promise<SearchResult> {
    arg.pattern = this.toLogEncoding(arg.pattern)
    return this.sendToCore<SearchResult>('search', arg)
  }
  mapLine(arg: { logId: number; index: number; }): Promise<MapLineResult> {
    return this.sendToCore<MapLineResult>('mapLine', arg)
  }

  filter(arg: FilterArg): Promise<OpenLogResult> {
    arg.pattern = this.toLogEncoding(arg.pattern)
    return this.sendToCore<OpenLogResult>('filter', arg)
  }

  dispose() {
    this.core.removeAllListeners('exit')
    this.stop()
    dispatcher.unregisterAll(this.id)
  }

  start() {
    let appDir = path.dirname(process.execPath)
    if (process.env.VITE_DEV_SERVER_URL) {
      appDir = process.cwd()
    }
    
    this.core = spawn(`${appDir}/core.${process.platform}`)
    this.core.stdout.on('data', this.handleData.bind(this))

    this.core.on('exit', ()=>{
      this.callback && this.callback({cmd: 'die'})
    })
  }

  private handleData(data: Buffer) {
    const txt = this.tail + data.toString()
    const lines = txt.split('\n')

    this.tail = ''
    if (lines[lines.length - 1] !== '') {//最后一行还没结束，留下次拼接
      this.tail = lines.pop()
    }

    lines.forEach(line=>{
      let data
      try{
        data = JSON.parse(line)
      }catch(e) {//其他日志输出
        console.log(line)
        return
      }
      if (data.origin !== 'getLines') {
        console.log(line)
      }

      const {state, cmd, id} = data
      if (cmd === 'reply') {
        const promise = this.pendingCmds.get(id)
        if (promise) {
          //TODO: 处理其他情况
          if (state === CoreCmdState.Fail) {
            promise.reject(data.why)
          } else if (state === CoreCmdState.Ok) {
            promise.resolve(data)
          }
        }
      } else {
        //TODO: 支持callback的返回值回包给backend
        if (cmd === 'rangeChanged' || cmd === 'disconnected') {
          this.callback && this.callback({cmd})
        }
      }
    })
  }

  private nextCmdId() {
    return `${this.cmdId++}`
  }

  private sendToCore<T>(proc:string, args: any) {
    const cmdId = this.nextCmdId()

    const ret = new Promise<T>((resolve, reject) => {
      this.pendingCmds.set(cmdId, {resolve, reject})
    })

    const cmd = {
      cmd: proc,
      id: cmdId,
      ...args
    }
    console.log(JSON.stringify(cmd))
    this.core.stdin.write(JSON.stringify(cmd)+'\n')

    return ret
  }

  private toNativeEncoded(s: string): string {
    return iconv.encode(s, nativeEncode).toString('base64')
  }

  private fromLogEncoding(s: string): string {
    return iconv.decode(Buffer.from(s, 'base64'), this.logEncoding)
  }

  private toLogEncoding(s: string): string {
    return iconv.encode(s, this.logEncoding).toString('base64')
  }

  setLogEncoding(encoding: string) {
    this.logEncoding = encoding
  }

  openFile(arg: {path: string}) {
    arg.path = this.toNativeEncoded(arg.path)
    return this.sendToCore<OpenLogResult>('openFile', arg)
  }

  getLines(arg: {logId: number, range: LogRange}) {
    return this.sendToCore<GetLinesResult>('getLines', arg)
      .then((result)=>{
        for (let i = 0; i < result.lines.length; i++) {
          result.lines[i].content = this.fromLogEncoding(result.lines[i].content)
        }
        return result
      })
  }
  
  syncLogs(): Promise<SyncLogsResult> {
    return this.sendToCore<SyncLogsResult>('syncLogs', {})
  }

  stop() {
    console.log('send kill')
    this.core.kill('SIGKILL')
  }

  setListener(callback: IBackendCallback): void {
    this.callback = callback
  }
}

export const platform = new Platform()