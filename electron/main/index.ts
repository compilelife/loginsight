import { app, BrowserWindow, shell, ipcMain, MessageChannelMain, WebContents, session, Menu } from 'electron'
import { release } from 'node:os'
import { join } from 'node:path'
import { dispatcher } from '../ipc/dispatcher'
import { platform } from '../ipc/platform'
import { machineIdSync } from 'node-machine-id'
import os from 'node:os'
import config from '../../package.json';

// The built directory structure
//
// ├─┬ dist-electron
// │ ├─┬ main
// │ │ └── index.js    > Electron-Main
// │ └─┬ preload
// │   └── index.js    > Preload-Scripts
// ├─┬ dist
// │ └── index.html    > Electron-Renderer
//
process.env.DIST_ELECTRON = join(__dirname, '..')
process.env.DIST = join(process.env.DIST_ELECTRON, '../dist')
process.env.PUBLIC = process.env.VITE_DEV_SERVER_URL
  ? join(process.env.DIST_ELECTRON, '../public')
  : process.env.DIST

// Disable GPU Acceleration for Windows 7
if (release().startsWith('6.1')) app.disableHardwareAcceleration()

// Set application name for Windows 10+ notifications
if (process.platform === 'win32') app.setAppUserModelId(app.getName())

if (!app.requestSingleInstanceLock()) {
  app.quit()
  process.exit(0)
}

Menu.setApplicationMenu(null)

let win: BrowserWindow | null = null

const preload = join(__dirname, '../preload/index.js')
const url = process.env.VITE_DEV_SERVER_URL
const indexHtml = join(process.env.DIST, 'index.html')

async function createWindow() {
  if (process.platform === 'darwin') {
    const template = [
      {
        label: "Application",
        submenu: [
          { label: "Quit", accelerator: "Command+Q", click: function() { app.quit(); }}
        ]
      }, 
      {
        label: "Edit",
        submenu: [
          { label: "Copy", accelerator: "CmdOrCtrl+C", selector: "copy:" },
          { label: "Paste", accelerator: "CmdOrCtrl+V", selector: "paste:" },
        ]
      }
    ];
    Menu.setApplicationMenu(Menu.buildFromTemplate(template))
  } else {
    Menu.setApplicationMenu(null)
  }
  
  win = new BrowserWindow({
    title: 'Loginsight',
    icon: join(process.env.PUBLIC, 'favicon.png'),
    width: 1150,
    height: 650,
    webPreferences: {
      preload,
      //FIXME: 去掉这行（目前加上是为了window.open）
      webSecurity: false,
      //FIXME: 去掉这行（目前加上是为了在在preload中import第三方包）
      nodeIntegration: true,
    },
  })

  platform.client = win
  platform.args = process.env.VITE_DEV_SERVER_URL ? [] : process.argv.slice(1)
  ipcMain.handle('ipc', (_, ...args) => dispatcher.dispatch(...args))

  if (process.env.VITE_DEV_SERVER_URL) { // electron-vite-vue#298
    win.loadURL(url)
    // Open devTool if the app is not packaged
    win.webContents.openDevTools()
    if (process.env.ELECTRON_PLUGIN) {//调试阶段加载插件
      session.defaultSession.loadExtension(process.env.ELECTRON_PLUGIN)
    }
  } else {
    win.loadFile(indexHtml)
  }

  win.webContents.setWindowOpenHandler(({ url }) => {
    if (url.startsWith('https:') || url.startsWith('http')) shell.openExternal(url)
    return { action: 'deny' }
  })
}

app.whenReady().then(createWindow)

app.on('window-all-closed', () => {
  win = null
  app.quit()
})

app.on('second-instance', () => {
  if (win) {
    // Focus on the main window if the user tried to open another
    if (win.isMinimized()) win.restore()
    win.focus()
  }
})

function collectPcStatus() {
  const machineId = machineIdSync()
  const time = new Date().getTime()
  const cpus = os.cpus()
  const payload = {
    type: 'startup',
    machineId,
    time,
    extra: {
      cpu: cpus[0]?.model || '',
      core: os.cpus().length,
      platform: os.platform(),
      arch: os.arch(),
      totalMem: os.totalmem(),
      freeMem: os.freemem(),
      version: config.version
    }
  }
  fetch('https://www.loginsight.top/api/collect', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify(payload)
  })
}

collectPcStatus()