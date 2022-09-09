.pragma library

const VERSION = 'v3.0.0'
function getNumVersion(v) {
  return v.substring(1).split('.').map(v=>parseInt(v))
}
function isVersionBigger(v1, v2) {
  const nv1 = getNumVersion(v1)
  const nv2 = getNumVersion(v2)
  const versionToInt = function(v) {
    return v[0]*1000000 + v[1]*1000 + v[2]
  }
  return versionToInt(nv1) > versionToInt(nv2)
}

const WEB_HOST = 'http://www.loginsight.top'
const WEB_PAGE = 'http://www.loginsight.top'
const WEB_GET_VERSION_URL = WEB_HOST+'/api/web/version'
const WEB_REGISTER_URL = WEB_HOST+'/api/register'
const WEB_DOWNLOAD_URL = "https://mianbaoduo.com/o/bread/YZmYkpts"
