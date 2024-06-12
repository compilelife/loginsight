const { rm, copyFile} = require('node:fs/promises');
const {Arch} = require('electron-builder')
const path = require('path');

exports.default = async function(context) {
  if ((context.electronPlatformName !== 'darwin' && context.electronPlatformName != 'mas') || context.arch === Arch.universal)
    return

  const projectPath = path.resolve(__dirname, '..');
  console.log('mv core to MacOS', context.arch)

  const unusedPath = context.appOutDir+'/loginsight.app/Contents/core.darwin'
  const targetPath = context.appOutDir+'/loginsight.app/Contents/MacOS/core.darwin'
  let corePath = projectPath +"/core.darwin"
  if (context.arch === Arch.x64) {
    corePath += '.x64'
  }
  
  await rm(unusedPath)
  await copyFile(corePath, targetPath)
}
