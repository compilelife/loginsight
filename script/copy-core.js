const {rename} = require('node:fs/promises');

exports.default = async function(context) {
  if (context.electronPlatformName !== 'darwin')
    return

  console.log('mv core to MacOS')
  await rename(context.appOutDir+'/loginsight.app/Contents/core.darwin', context.appOutDir+'/loginsight.app/Contents/MacOS/core.darwin')
}
