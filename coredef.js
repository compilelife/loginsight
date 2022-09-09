.pragma library

const StateOk = 0;
const StateFail = 1;
const StateFuture = 2;
const StateCancel = 3;

const CmdReply = "reply"
const CmdQueryPromise = 'queryPromise'
const CmdCancelPromise = 'cancelPromise'
const CmdGetLines = 'getLines'
const CmdOpenFile = 'openFile'
const CmdOpenProcess = 'openProcess'
const CmdFilter = 'filter'
const CmdSyncLogs = 'syncLogs'
const CmdSetLineSegment = 'setLineSegment'
const CmdSearch = 'search'
const CmdMapLine = 'mapLine'
const CmdTestSyntax = 'testSyntax'
const CmdGetRange = 'getRange'
const CmdInitRegister = 'initRegister'
const CmdDoRegister = 'doRegister'
const CmdExportLog = 'exportLog'
const CmdCloseLog = 'closeLog'
const ServerCmdRangeChanged = 'rangeChanged'

const SegTypeDate = 0;
const SegTypeLogLevel = 1;
const SegTypeNum = 2;
const SegTypeStr = 3;

const SegTypeNames = ['date', 'loglevel', 'number', 'string']

const LogLevelDebug = 0;
const LogLevelInfo = 1;
const LogLevelWarn = 2;
const LogLevelError = 3;
const LogLevelFatal = 4;

const RSOpenSource = 0;
const RSTry = 1;
const RSTryEnd = 2;
const RSRegistered = 3;

