interface IPlatform {
  createBackend(): IBackend | Promise<IBackend>;
  saveImageToClipboard(imgData: string)
  saveTextToClipboard(text: string)
  readClipboardToFile(): Promise<string>
  showOpenDialog(options: any): Promise<OpenFileDialogResult>;
  showSaveDialog(options: any): Promise<SaveFileDialogResult>;
  getFilesUnderFolder(path: string): Promise<string[]>;
  writeFile(path: string, data: string): Promise<string>;
  readFile(path: string): Promise<string>;
  //如果文件不存在，则会抛出异常
  isFile(path: string): Promise<boolean>; 
  getCmdlineArgs(): Promise<string[]>;
  openDevTool(): any
}

interface OpenFileDialogResult {
  canceled: boolean;
  filePaths: string[];
}
interface SaveFileDialogResult {
  canceled: boolean;
  filePath?: string;
}

interface IBackendCallback{
  (arg: {cmd: 'rangeChanged'|'disconnected'|'die'}):void;
}

interface IBackendParcel {
  id: string
}

interface LogRange {
  begin: number
  end: number
}

interface OpenLogResult {
  logId: number
  range: LogRange
}

interface Line extends LineSyntax{
  //在自己log里的索引，一般用于显示logview里的行号
  index: number
  //在root log里的索引，可用于全局追踪，比如添加到timeline，追踪该行等
  line: number
  content: string
}

interface GetLinesResult {
  lines: Array<Line>
}

interface SyncLogsResult {
  ranges: Array<{
    logId: number,
    range: LogRange
  }>
}

interface FilterArg {
  logId: number
  regex:boolean
  pattern: string
  caseSense: boolean
  reverse: boolean
}

interface MapLineResult {
  lines: Array<{
    logId: number,
    index: number
  }>
}

interface SearchArg {
  logId: number
  fromLine: number
  fromChar: number
  toLine: number
  reverse: boolean
  regex: boolean
  pattern: string
  caseSense: boolean
}

interface SearchResult {
  found: boolean
  index: number
  offset: number
  len: number
}

interface LineRegion {
  offset: number
  length: number
}

interface TestSyntaxArg {
  pattern: string
  lines: Array<string>
  caseSense: boolean
}

interface LineSyntax {
  segs: Array<LineRegion>
}

interface TestSyntaxResult {
  segs: Array<Array<LineRegion>>
}

interface SetLineSeg {
  type: number
  name: string
  extra: {}
}
interface SetLineSegmentArg {
  pattern: string
  caseSense: boolean
  segs: Array<SetLineSeg>
}

interface OpenProcessArg {
  process: string
  cache: number
}

interface OpenFolderArg {
  files:string[]
}

interface OpenFileArg {
  path: string
}

interface ExportLogArg {
  logId: number
  from: number
  to: number
  all: boolean
  path: string
}

interface IBackend {
  setListener(callback: IBackendCallback):void
  start();
  dispose()
  openFile(arg: OpenFileArg): Promise<OpenLogResult>;
  openProcess(arg: OpenProcessArg): Promise<OpenLogResult>
  getLines(arg: {logId: number, range: LogRange}): Promise<GetLinesResult>;
  setLogEncoding(encoding: string)
  syncLogs(): Promise<SyncLogsResult>;
  mapLine(arg: {logId: number, index: number}): Promise<MapLineResult>
  filter(arg: FilterArg): Promise<OpenLogResult>;
  search(arg: SearchArg): Promise<SearchResult>
  testSyntax(arg: TestSyntaxArg): Promise<TestSyntaxResult>
  setLineSegment(arg: SetLineSegmentArg): Promise<void>
  openMultiFile(arg: OpenFolderArg): Promise<OpenLogResult>
  exportLog(arg: ExportLogArg): Promise<any>
  clearLog({logId: number}): Promise<void>

  mapOffsetFromLog(line: string, values: LineRegion[]): LineRegion[] | Promise<LineRegion[]>
}

interface Window {
  host: {
    ipcInvoke(who: string, func: string, ...args: any[]): Promise<any>
    ipcCallback(dispatcher: Function): void
    platform: string
    machineId: string
  }
}